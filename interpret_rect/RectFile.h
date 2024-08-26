#pragma once

#include "ActConfig.h"
#include <phy/Library.h>

#include <set>

using namespace phy;
using namespace act;
using namespace std;

void emitRect(FILE *fptr, const ActConfig &cfg, const Rect &rect, const Layout &layout, string mtrl);
void emitRect(FILE *fptr, const ActConfig &cfg, const Layer &layer, const Layout &layout, string mtrl);
void emitRect(FILE *fptr, const ActConfig &cfg, const Layout &layout);
void emitRect(string path, const ActConfig &cfg, const Library &library, set<string> cellNames = set<string>());
