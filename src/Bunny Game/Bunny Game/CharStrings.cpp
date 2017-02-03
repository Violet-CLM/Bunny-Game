#include "CharStrings.h"
#include "BunnyMisc.h"

const TtextAppearance TtextAppearance::pureString;
const TtextAppearance TtextAppearance::defaultNormal(0, 0, true,		ch_DISPLAY, ch_DISPLAY, ch_SPECIAL);
const TtextAppearance TtextAppearance::defaultDark(0, 0, true,			ch_DISPLAY, ch_DISPLAY, ch_HIDE);
const TtextAppearance TtextAppearance::defaultRightAlign(0, 0, false,	ch_DISPLAY, ch_DISPLAY, ch_HIDE, align_RIGHT);
const TtextAppearance TtextAppearance::defaultBounce(0, 1, false,		ch_DISPLAY, ch_DISPLAY, ch_HIDE, align_DEFAULT, 0);
const TtextAppearance TtextAppearance::defaultSpin(1, 1, false,			ch_SPECIAL, ch_SPECIAL, ch_HIDE);
const TtextAppearance TtextAppearance::defaultPalShift(0, 0, false,		ch_DISPLAY, ch_DISPLAY, ch_HIDE);

int WriteText(VertexCollectionQueue& sprites, int x,int y, const char* text, const std::vector<AnimFrame>& font, const TtextAppearance& textParams) {
	int textLength = strlen(text);
	if (textLength <= 0)
		return x;
	const int spaceWidth = textParams.xAmp ? (font[28].Width - 2) : (textParams.yAmp? 16 : 7);
	const int newlineHeight = font[1].Height * 5 / 3;
	if (text[0] == '#' && textParams.skipInitialHash) {
		if (--textLength == 0)
			return x;
		text++;
	}
	int initialXPos = x;
	/*if (textParams.align != TtextAppearance::align_LEFT) {
		const int textWidth = getTextWidth(text, anim, textParams);
		if (textParams.align == TtextAppearance::align_CENTER) {
			x -= getTextLineWidth(text, anim, textParams) / 2;
		} else {
			if (textParams.align == TtextAppearance::align_RIGHT) {
				x -= getTextLineWidth(text, anim, textParams);
			} else if (x > 0x4000) {
				if (x < 0x10000)
					initialXPos = x = x + *screenWidth / 2 - textWidth / 2 - 0x8000;
				else
					initialXPos = x = *screenWidth - textWidth - x + 0x10000 - (textParams.xAmp ? 8 : 0);
			}
		}
	}*/
	y -= 10;
	int spacing = textParams.spacing;
	int xArg = 0;//textParams.xAmp ? animationTick + 256 : 0;
	int yArg = 0;//textParams.yAmp ? animationTick : 0;
	bool hashColored = false;
	bool pipeColored = false;
	int currentColor = 0;
	int frameID = 0;
	for (int i = 0; i < textLength; i++) {
		if (textParams.xAmp)
			xArg += 16 * frameID;
		if (textParams.yAmp)
			yArg += 8 * frameID;
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
					if (pipeColored) {
						currentColor += 8;
						if (currentColor > 2*8)
							currentColor = -6*8;
						else if (currentColor == -1*8)
							currentColor = 0*8;
					} else {
						pipeColored = true;
						currentColor = -6*8;
					}
					continue;
				}
				if (textParams.pipe == TtextAppearance::ch_HIDE)
					realFrameID = 0;
				break;
		}
		if (hashColored) {
			currentColor += 8;
			if (currentColor > 16)
				currentColor = -48;
			if (currentColor == -8)
				currentColor = 0;
		}
		if (realFrameID < 0)
			realFrameID = 0;
		const auto& frame = font[realFrameID];
		if (frame.Width) {
			/*if (inverseAmplitude) {
				if (textParams.xAmp)
					xUlt += sinTable(xArg) * textParams.xAmp / inverseAmplitude;
				if (textParams.yAmp)
					yUlt += sinTable(yArg) * textParams.yAmp / inverseAmplitude;
			}*/
			//sf::Uint8 paramUlt = (spriteParam + currentColor) & 255;
			sprites.AppendSprite(SpriteMode::Paletted, x,y, frame);
			x += spacing + frame.Width;
		}
	}
	return x;
}
