#pragma once

#include "conf.h"
#include <string>
#include <map>
#include <vector>

#include <phy/Tech.h>

using namespace std;

namespace act {

struct ActConfig {
	string mangleChars;
	string mangleLetter;
	vector<pair<string, vector<int> > > mtrls;

	void loadValue(const phy::Tech &tech, pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &value, string name, map<string, string> &mtrlMap);
	void loadBlock(const phy::Tech &tech, pgen::conf_t lang, pgen::lexer_t &lexer, pgen::token_t &block, string name);
	bool load(const phy::Tech &tech, string path);

	string replaceEnvVariables(string input) const;
	string mangleName(string name) const;
	string demangleName(string name) const;
};

}
