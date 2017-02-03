#pragma once
#include "Resources.h"

struct TtextAppearance {
	static const TtextAppearance pureString, defaultNormal, defaultDark, defaultRightAlign, defaultBounce, defaultSpin, defaultPalShift;

	enum characterTreatment {ch_HIDE, ch_DISPLAY, ch_SPECIAL};
	enum textAlignment {align_DEFAULT, align_LEFT, align_CENTER, align_RIGHT};
	int xAmp, yAmp;
	int spacing;
	bool skipInitialHash;
	characterTreatment at, hash, pipe;
	textAlignment align;

	TtextAppearance(int xAmp = 0, int yAmp = 0, bool skipInitialHash = false,   characterTreatment at = ch_DISPLAY, characterTreatment hash = ch_DISPLAY, characterTreatment pipe = ch_DISPLAY, textAlignment align = align_DEFAULT, int spacing = 1)
		: xAmp(xAmp), yAmp(yAmp), skipInitialHash(skipInitialHash), at(at), hash(hash), pipe(pipe), align(align), spacing(spacing) { }
};

int WriteText(VertexCollectionQueue& sprites, int x, int y, const char* text, const std::vector<AnimFrame>& font, const TtextAppearance& textParams = TtextAppearance::defaultNormal);
