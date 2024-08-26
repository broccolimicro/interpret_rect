#pragma once

#include <pgen/grammar.h>

namespace pgen
{

struct conf_t
{
	int32_t CONF;
	int32_t BLOCK;
	int32_t _;
	int32_t __;
	int32_t TABLE;
	int32_t VALUE;
	int32_t SECTION;
	int32_t INCLUDE;
	int32_t TEXT;
	int32_t NAME;
	int32_t INTEGER;
	int32_t REAL;

	void load(grammar_t &grammar);
};

}

