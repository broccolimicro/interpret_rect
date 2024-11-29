#include "RectFile.h"

#include <sys/stat.h>

void emitRect(FILE *fptr, const ActConfig &cfg, const Rect &rect, const Layout &layout, string mtrl) {
	fprintf(fptr, "rect %s %s %d %d %d %d\n", rect.net < 0 ? "#" : cfg.demangleName(layout.nets[rect.net].names[0]).c_str(), mtrl.c_str(), rect.ll[0], rect.ll[1], rect.ur[0], rect.ur[1]);
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

	Rect bound = layout.box;
	fprintf(fptr, "bbox %d %d %d %d\n", bound.ll[0], bound.ll[1], bound.ur[0], bound.ur[1]);
	vector<map<int, Layer>::const_iterator> operands;
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
		Layer result(*layout.tech, true);
		for (int i = 0; i < (int)operands.size(); i++) {
			result = result & operands[i]->second;
		}

		emitRect(fptr, cfg, result, layout, layer->first);
	}
}

void emitRect(string path, const ActConfig &cfg, const Library &library, set<string> cellNames) {
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	for (auto cell = library.macros.begin(); cell != library.macros.end(); cell++) {
		if (cellNames.empty() or cellNames.find(cell->name) != cellNames.end()) {
			string fpath = path;
			if (path.back() != '/') {
				fpath += "/";
			}
			fpath += cell->name + ".rect";
			printf("creating %s\n", fpath.c_str());
			FILE *fptr = fopen(fpath.c_str(), "w");
			emitRect(fptr, cfg, *cell);
			fclose(fptr);
		}
	}
}

