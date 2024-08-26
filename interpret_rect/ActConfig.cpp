#include "ActConfig.h"

namespace act {

void ActConfig::loadValue(const phy::Tech &tech, pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &value, string name, map<string, string> &mtrlMap) {
	string kind = lexer.read(value.tokens[0].begin, value.tokens[0].end);
	string attr = lexer.read(value.tokens[1].begin, value.tokens[1].end);

	if (kind == "string") {
		if (attr == "mangle_chars") {
			mangleChars = lexer.read(value.tokens[2].begin, value.tokens[2].end);
			mangleChars = mangleChars.substr(1, mangleChars.size()-2);
		} else if (attr == "mangle_letter") {
			mangleLetter = lexer.read(value.tokens[2].begin, value.tokens[2].end);
			mangleLetter = mangleLetter.substr(1, mangleLetter.size()-2);
		} else if (name.rfind(".vias", 0) == 0 and attr.rfind("_name") != string::npos) {
			attr = attr.substr(0, attr.rfind("_"));
			string str = lexer.read(value.tokens[2].begin, value.tokens[2].end);
			str = str.substr(1, str.size()-2);
			mtrlMap[attr] = str;
		} else if (name.rfind(".materials.metal", 0) == 0 and attr[0] == 'm' and attr.find("_") == string::npos) {
			string str = lexer.read(value.tokens[2].begin, value.tokens[2].end);
			str = str.substr(1, str.size()-2);
			printf("found %s -> %s\n", str.c_str(), attr.c_str());
			mtrlMap[str] = attr;
		}
	} else if (kind == "string_table" and (name.rfind(".materials", 0) == 0 or name.rfind(".vias", 0) == 0) and attr.rfind("gds") != string::npos) {
		vector<int> layers;
		for (auto tok = value.tokens.begin()+2; tok != value.tokens.end(); tok++) {
			string layer = lexer.read(tok->begin, tok->end);
			layer = layer.substr(1, layer.size()-2);
			int idx = tech.findPaint(layer);
			if (idx >= 0) {
				layers.push_back(idx);
			}
		}
		
		if (not layers.empty()) {
			string mtrl;
			if (attr.rfind("_gds") != string::npos) {
				mtrl = attr.substr(0, attr.rfind("_"));
				if (name.rfind(".vias", 0) == 0) {
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
				mtrl = name.substr(name.rfind(".")+1);
			}

			mtrls.push_back(pair<string, vector<int> >(mtrl, layers));
		}
	} 
}

void ActConfig::loadBlock(const phy::Tech &tech, pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &block, string name) {
	map<string, string> mtrlMap;
	for (auto tok = block.tokens.begin(); tok != block.tokens.end(); tok++) {
		if (tok->type == lang.TABLE) {
			loadValue(tech, lang, lexer, *tok, name, mtrlMap);
		} else if (tok->type == lang.VALUE) {
			loadValue(tech, lang, lexer, *tok, name, mtrlMap);
		} else if (tok->type == lang.SECTION) {
			string sub = name + "." + lexer.read(tok->tokens[0].begin, tok->tokens[0].end);
			loadBlock(tech, lang, lexer, tok->tokens[1], sub);
		} else if (tok->type == lang.INCLUDE) {
			string path = lexer.read(tok->tokens[0].begin, tok->tokens[0].end);
			path = path.substr(1, path.size()-2);
			path = replaceEnvVariables(path);
			if (not load(tech, path)) {
				printf("error loading: '%s'\n", path.c_str());
			}
		}
	}
}

bool ActConfig::load(const phy::Tech &tech, string path) {
	// Initialize the grammar
	pgen::grammar_t gram;
	pgen::conf_t lang;
	lang.load(gram);

	// Load the file into the lexer
	pgen::lexer_t lexer;
	if (not lexer.open(path)) {
		printf("file not found: '%s'\n", path.c_str());
		return false;
	}

	// Parse the file with the grammar
	pgen::parsing ast = gram.parse(lexer);
	if (ast.msgs.size() != 0) {
		// there were parsing errors, print them out
		for (int i = 0; i < (int)ast.msgs.size(); i++) {
			cout << ast.msgs[i];
		}
		printf("error loading: '%s'\n", path.c_str());
		return false;
	}

	// no errors, print the parsed abstract syntax tree
	loadBlock(tech, lang, lexer, ast.tree.tokens[0], "");

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
