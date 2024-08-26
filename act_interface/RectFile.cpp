#include "RectFile.h"

void emitRect(FILE *fptr, const ActConfig &cfg, const Rect &rect, const Layout &layout, string mtrl) {
	fprintf(fptr, "rect %s %s %d %d %d %d\n", rect.net < 0 ? "#" : cfg.demangleName(layout.nets[rect.net]).c_str(), mtrl.c_str(), rect.ll[0], rect.ll[1], rect.ur[0], rect.ur[1]);
}

void emitRect(FILE *fptr, const ActConfig &cfg, const Layer &layer, const Layout &layout, string mtrl) {
	for (auto r = layer.geo.begin(); r != layer.geo.end(); r++) {
		emitRect(fptr, cfg, *r, layout, mtrl);
	}
}

void emitRect(FILE *fptr, const ActConfig &cfg, const Layout &layout) {
	if (fptr == nullptr) {
		return;
	}

	Rect bound = layout.bbox();
	fprintf(fptr, "bbox %d %d %d %d\n", bound.ll[0], bound.ll[1], bound.ur[0], bound.ur[1]);
	vector<vector<Layer>::const_iterator> operands;
	for (auto layer = cfg.mtrls.begin(); layer != cfg.mtrls.end(); layer++) {
		operands.clear();
		for (int i = 0; i < (int)layer->second.size(); i++) {
			operands.push_back(layout.find(layer->second[i]));
			if (operands.back() == layout.layers.end()) {
				operands.pop_back();
				break;
			}
		}
		if (operands.size() < layer->second.size()) {
			continue;
		}
		Layer result(true);
		for (int i = 0; i < (int)operands.size(); i++) {
			result = result & *(operands[i]);
		}

		emitRect(fptr, cfg, result, layout, layer->first);
	}
}

