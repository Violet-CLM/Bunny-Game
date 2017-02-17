#include "CharStrings.h"
#include "Lattice.h"
#include "BunnyShaders.h"
#include "BunnyMisc.h"

const TtextAppearance TtextAppearance::pureString;
const TtextAppearance TtextAppearance::defaultNormal(0, 0, 0, true,			ch_DISPLAY, ch_DISPLAY, ch_SPECIAL);
const TtextAppearance TtextAppearance::defaultDark(0, 0, 0, true,			ch_DISPLAY, ch_DISPLAY, ch_HIDE);
const TtextAppearance TtextAppearance::defaultRightAlign(0, 0, 0, false,	ch_DISPLAY, ch_DISPLAY, ch_HIDE, align_RIGHT);
const TtextAppearance TtextAppearance::defaultBounce(0, 1, 16, false,		ch_DISPLAY, ch_DISPLAY, ch_HIDE, align_DEFAULT, 0);
const TtextAppearance TtextAppearance::defaultSpin(1, 1, 16, false,			ch_SPECIAL, ch_SPECIAL, ch_HIDE);
const TtextAppearance TtextAppearance::defaultPalShift(0, 0, 0, false,		ch_DISPLAY, ch_DISPLAY, ch_HIDE);
const TtextAppearance TtextAppearance::defaultMenuSpinFast(1, 1, 16, false,	ch_HIDE, ch_SPECIAL, ch_HIDE, align_DEFAULT, 1, 0, MenuPalShifts);
const TtextAppearance TtextAppearance::defaultMenuSpinSlow(1, 1, 8, false,	ch_HIDE, ch_SPECIAL, ch_HIDE, align_DEFAULT, 1, 0, MenuPalShifts);

const TtextPalshiftList IngamePalShifts = {
	sf::Uint8(2 * 8), sf::Uint8(-6 * 8), sf::Uint8(-5 * 8), sf::Uint8(-4 * 8), sf::Uint8(-3 * 8), sf::Uint8(-2 * 8), 0, sf::Uint8(1 * 8)
};
const TtextPalshiftList MenuPalShifts = {
	16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6
};

WriteCharacter GetWriteCharacterFunction(VertexCollectionQueue& sprites) {
	return [&sprites](const AnimFrame& frame, sf::Uint8 spriteParam, int x, int y) {
		sprites.AppendSprite(SpriteMode(Shaders[BunnyShaders::Palshift], spriteParam), x,y, frame);
	};
}

int getTextWidth(const char* text, FontAnim& font, const TtextAppearance& textParams) {
	const int spaceWidth = textParams.xAmp ? 15 : (textParams.yAmp? 16 : 7);
	int currentLineWidth = 0;
	int longestLineWidth = 0;
	int spacing = textParams.spacing;
	for (; *text; ++text) {
		int frameID = *text - 32;
		switch (*text) {
			case ' ':
				currentLineWidth += spacing + spaceWidth;
				continue;
			case '§':
				spacing = 48 - (unsigned char)*++text;
				continue;
			case '#':
				if (textParams.hash == TtextAppearance::ch_SPECIAL)
					continue;
				if (textParams.hash == TtextAppearance::ch_HIDE)
					frameID = 0;
				break;
			case '@':
				if (textParams.at == TtextAppearance::ch_SPECIAL) {
					if (currentLineWidth > longestLineWidth)
						longestLineWidth = currentLineWidth;
					currentLineWidth = 0;
					continue;
				}
				if (textParams.at == TtextAppearance::ch_HIDE)
					frameID = 0;
				break;
			case '|':
				if (textParams.pipe == TtextAppearance::ch_SPECIAL)
					continue;
				if (textParams.pipe == TtextAppearance::ch_HIDE)
					frameID = 0;
				break;
		}
		if (frameID < 0)
			frameID = 0;
		currentLineWidth += spacing + font[frameID].Width;
	}
	return std::max(currentLineWidth, longestLineWidth);
}
int WriteText(WriteCharacter& writeChar, int x,int y, const char* text, FontAnim& font, const TtextAppearance& textParams, unsigned int animationTick) {
	int textLength = strlen(text);
	if (textLength <= 0)
		return x;
	const int spaceWidth = textParams.xAmp ? 15 : (textParams.yAmp? 16 : 7);
	const int newlineHeight = font[1].Height * 5 / 3;
	if (text[0] == '#' && textParams.skipInitialHash) {
		if (--textLength == 0)
			return x;
		text++;
	}
	int initialXPos = x;
	if (x > 0x4000) {
		const int textWidth = getTextWidth(text, font, textParams);
		if (x < 0x10000)
			initialXPos = x += WINDOW_WIDTH_PIXELS / 2 - textWidth / 2 - TtextAppearance::DefaultCenterAlign;
		else
			initialXPos = x = WINDOW_WIDTH_PIXELS - textWidth - x + TtextAppearance::DefaultRightAlign - (textParams.xAmp ? 8 : 0);
	}
	y -= 10;
	int spacing = textParams.spacing;
	animationTick *= textParams.animSpeed;
	int xArg = textParams.xAmp ? animationTick + 256 : 0;
	int yArg = textParams.yAmp ? animationTick : 0;
	bool hashColored = false;
	unsigned int currentColor = 0;
	int frameID = 0;
	for (int i = 0; i < textLength; i++) {
		xArg += int(16 * textParams.xAmp * frameID);
		yArg += int(8 * textParams.yAmp * frameID);
		int realFrameID = frameID = (unsigned char)text[i] - 32;
		switch (text[i]) {
			case ' ':
				x += spacing + spaceWidth;
				continue;
			case '§':
				spacing = 48 - (unsigned char)text[++i];
				continue;
				break;
			case '#':
				if (textParams.hash == TtextAppearance::ch_SPECIAL) {
					hashColored = true;
					continue;
				}
				if (textParams.hash == TtextAppearance::ch_HIDE)
					realFrameID = 0;
				break;
			case '@':
				if (textParams.at == TtextAppearance::ch_SPECIAL) {
					x = initialXPos;
					y += 20;
					/*if (textParams.align == TtextAppearance::align_CENTER)
						x -= getTextLineWidth(&text[i+1], anim, textParams) / 2;
					else if (textParams.align == TtextAppearance::align_RIGHT)
						x -= getTextLineWidth(&text[i+1], anim, textParams);*/
					continue;
				}
				if (textParams.at == TtextAppearance::ch_HIDE)
					realFrameID = 0;
				break;
			case '|':
				if (textParams.pipe == TtextAppearance::ch_SPECIAL) {
					++currentColor;
					continue;
				}
				if (textParams.pipe == TtextAppearance::ch_HIDE)
					realFrameID = 0;
				break;
		}
		if (hashColored)
			++currentColor;
		if (realFrameID < 0)
			realFrameID = 0;
		const auto& frame = font[realFrameID];
		if (frame.Width) {
			auto xUlt = x, yUlt = y;
			if (textParams.animSpeed) {
				xUlt += int(textParams.xAmp * sinTable(xArg) * 65536 / textParams.inverseAmplitude);
				yUlt += int(textParams.yAmp * sinTable(yArg) * 65536 / textParams.inverseAmplitude);
			}
			writeChar(frame, textParams.spriteParam + (!currentColor ? 0 : textParams.spriteParams[currentColor % textParams.spriteParams.size()]), xUlt,yUlt);
			x += spacing + frame.Width;
		}
	}
	return x;
}
