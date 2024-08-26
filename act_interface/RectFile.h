#pragma once

#include "ActConfig.h"
#include <phy/Layout.h>

using namespace phy;
using namespace act;

void emitRect(FILE *fptr, const ActConfig &cfg, const Rect &rect, const Layout &layout, string mtrl);
void emitRect(FILE *fptr, const ActConfig &cfg, const Layer &layer, const Layout &layout, string mtrl);
void emitRect(FILE *fptr, const ActConfig &cfg, const Layout &layout);
