#pragma once
#include "Resources.h"

typedef const std::vector<sf::Uint8> TtextPalshiftList;
typedef const std::vector<AnimFrame> FontAnim;
extern const TtextPalshiftList IngamePalShifts, MenuPalShifts;

struct TtextAppearance {
	static const TtextAppearance pureString, defaultNormal, defaultDark, defaultRightAlign, defaultBounce, defaultSpin, defaultPalShift, defaultMenuSpinFast, defaultMenuSpinSlow;
	static const int DefaultCenterAlign = 0x8000;
	static const int DefaultRightAlign = 0x10000;

	enum characterTreatment {ch_HIDE, ch_DISPLAY, ch_SPECIAL};
	enum textAlignment {align_DEFAULT, align_LEFT, align_CENTER, align_RIGHT};
	float xAmp, yAmp;
	int animSpeed, inverseAmplitude;
	int spacing;
	bool skipInitialHash;
	characterTreatment at, hash, pipe;
	textAlignment align;
	unsigned char spriteParam;
	TtextPalshiftList& spriteParams;

	TtextAppearance(float xAmp = 0, float yAmp = 0, int animSpeed = 0, bool skipInitialHash = false,   characterTreatment at = ch_DISPLAY, characterTreatment hash = ch_DISPLAY, characterTreatment pipe = ch_DISPLAY, textAlignment align = align_DEFAULT, int spacing = 1, unsigned char spriteParam = 0, TtextPalshiftList& spriteParams = IngamePalShifts)
		: xAmp(xAmp), yAmp(yAmp), animSpeed(animSpeed), inverseAmplitude(0xC000 - (animSpeed << 11)), skipInitialHash(skipInitialHash), at(at), hash(hash), pipe(pipe), align(align), spacing(spacing), spriteParam(spriteParam), spriteParams(spriteParams) { }
};

int WriteText(VertexCollectionQueue& sprites, int x, int y, const char* text, FontAnim& font, const TtextAppearance& textParams = TtextAppearance::defaultNormal, unsigned int animationTick = 0);
