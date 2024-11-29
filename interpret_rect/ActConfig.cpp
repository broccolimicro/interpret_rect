#include "ActConfig.h"
#include <filesystem>

using namespace std;

namespace act {

void ActConfig::loadBlock(const phy::Tech &tech, const parse_act::block &syntax, string parent, map<string, string> &mtrlMap) {
	if (syntax.kind == "begin") {
		loadSection(tech, syntax.sub, parent + "." + syntax.name);
	} else if (syntax.kind == "string") {
		if (syntax.name == "mangle_chars") {
			mangleChars = syntax.values[0].substr(1, mangleChars.size()-2);
		} else if (syntax.name == "mangle_letter") {
			mangleLetter = syntax.values[0].substr(1, mangleLetter.size()-2);
		} else if (parent.rfind(".vias", 0) == 0 and syntax.name.rfind("_name") != string::npos) {
			string name = syntax.name.substr(0, syntax.name.rfind("_"));
			mtrlMap[name] = syntax.values[0].substr(1, syntax.values[0].size()-2);
		} else if (parent.rfind(".materials.metal", 0) == 0 and syntax.name[0] == 'm' and syntax.name.find("_") == string::npos) {
			string str = syntax.values[0].substr(1, syntax.values[0].size()-2);
			printf("found %s -> %s\n", str.c_str(), syntax.name.c_str());
			mtrlMap[str] = syntax.name;
		}
	} else if (syntax.kind == "string_table" and (parent.rfind(".materials", 0) == 0 or parent.rfind(".vias", 0) == 0) and syntax.name.rfind("gds") != string::npos) {
		vector<int> layers;
		for (auto value = syntax.values.begin(); value != syntax.values.end(); value++) {
			string layer = value->substr(1, value->size()-2);
			int idx = tech.findPaint(layer);
			if (idx >= 0) {
				layers.push_back(idx);
			}
		}
		
		if (not layers.empty()) {
			string mtrl;
			if (syntax.name.rfind("_gds") != string::npos) {
				mtrl = syntax.name.substr(0, syntax.name.rfind("_"));
				if (parent.rfind(".vias", 0) == 0) {
					for (auto mat = mtrls.begin(); mat != mtrls.end(); mat++) {
						if (mat->first == mtrl) {
							layers.reserve(layers.size()+mat->second.size());
							layers.insert(layers.end(), mat->second.begin(), mat->second.end());
							break;
						}
					}
				}
				auto pos = mtrlMap.find(mtrl);
				if (pos != mtrlMap.end()) {
					mtrl = pos->second;
				}
			} else {
				mtrl = parent.substr(parent.rfind(".")+1);
			}

			mtrls.push_back(pair<string, vector<int> >(mtrl, layers));
		}
	} 
}

void ActConfig::loadSection(const phy::Tech &tech, const parse_act::section &syntax, string parent) {
	map<string, string> mtrlMap;
	for (auto incl = syntax.includes.begin(); incl != syntax.includes.end(); incl++) {
		string path = *incl;
		path = path.substr(1, path.size()-2);
		path = replaceEnvVariables(path);
		if (not load(tech, path)) {
			printf("error loading: '%s'\n", path.c_str());
		}
	}

	for (auto blk = syntax.blocks.begin(); blk != syntax.blocks.end(); blk++) {
		loadBlock(tech, *blk, parent, mtrlMap);
	}
}

bool ActConfig::load(const phy::Tech &tech, string path) {
	filesystem::path current = filesystem::current_path();

	configuration config;
	config.set_working_directory(current.string());

	tokenizer tokens;

	parse_act::section::register_syntax(tokens);
	config.load(tokens, path, "");

	tokens.increment(true);
	tokens.expect<parse_act::section>();
	if (tokens.decrement(__FILE__, __LINE__))
	{
		parse_act::section syntax(tokens);
		loadSection(tech, syntax, "");
	}

	return true;
}

string ActConfig::replaceEnvVariables(string input) const {
	string result = input;
	size_t pos = 0;
	while ((pos = result.find("${", pos)) != string::npos) {
		size_t endPos = result.find("}", pos);
		if (endPos != string::npos) {
			string varName = result.substr(pos + 2, endPos - pos - 2);
			const char* envValue = getenv(varName.c_str());
			if (envValue) {
				result.replace(pos, endPos - pos + 1, envValue);
			} else {
				// Environment variable not found, leave placeholder unchanged
				pos = endPos + 1;
			}
		} else {
			// Closing '}' not found, stop searching
			break;
		}
	}
	return result;
}

string ActConfig::mangleName(string name) const {
	std::ostringstream result;
	for (int i = 0; i < (int)name.size(); i++) {
		if (name[i] == mangleLetter[0]) {
			result << mangleLetter << mangleLetter;
		} else {
			int loc = mangleChars.find(name[i]);
			if (loc == -1) {
				result << name[i];
			} else if (loc <= 9) {
				result << mangleLetter << loc;
			} else if (loc > 9) {
				result << mangleLetter << (char)('a' + loc-10);
			}
		}
	}
	return result.str();
}

string ActConfig::demangleName(string name) const {
	std::ostringstream result;
	for (int i = 0; i < (int)name.size(); i++) {
		if (name[i] == mangleLetter[0] && ++i < (int)name.size()) {
			if (name[i] == mangleLetter[0]) {
				result << mangleLetter;
			} else {
				int loc = name[i] - '0';
				if (loc > 9)
					loc = (name[i] - 'a') + 10;
				result << mangleChars[loc];
			}
		} else {
			result << name[i];
		}
	}
	return result.str();
}

}
