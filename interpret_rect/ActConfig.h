#pragma once

#include <string>
#include <map>
#include <vector>

#include <phy/Tech.h>
#include <parse_act/block.h>
#include <parse_act/section.h>

using namespace std;

namespace act {

struct ActConfig {
	string mangleChars;
	string mangleLetter;
	vector<pair<string, vector<int> > > mtrls;

	void loadBlock(const phy::Tech &tech, const parse_act::block &syntax, string parent, map<string, string> &mtrlMap);
	void loadSection(const phy::Tech &tech, const parse_act::section &syntax, string parent);
	bool load(const phy::Tech &tech, string path);

	string replaceEnvVariables(string input) const;
	string mangleName(string name) const;
	string demangleName(string name) const;
};

}
