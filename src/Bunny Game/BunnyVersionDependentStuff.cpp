#include "BunnyVersionDependentStuff.h"
#include "BunnyObjectList.h"
#include "Resources.h"
#include "Windows.h"
#include "Misc.h"

bool VersionTSF = true;

int GetVersionSpecificAnimationID_124(int originalAnimID) {
	return originalAnimID;
}
int GetVersionSpecificAnimationID_123(int originalAnimID) {
	if (originalAnimID == AnimSets::XBilsy)
		return GetVersionSpecificAnimationID_123(AnimSets::BilsBoss);
	if (originalAnimID == AnimSets::XLizard)
		return GetVersionSpecificAnimationID_123(AnimSets::Lizard);
	if (originalAnimID == AnimSets::XTurtle)
		return GetVersionSpecificAnimationID_123(AnimSets::Turtle);
	if (originalAnimID == AnimSets::ZDog)
		return GetVersionSpecificAnimationID_123(AnimSets::Dog);
	if (originalAnimID == AnimSets::ZSpark)
		return GetVersionSpecificAnimationID_123(AnimSets::Spark);

	int animID = originalAnimID;
	if (originalAnimID >= AnimSets::EndTuneLori)
		animID -= 1;
	if (originalAnimID >= AnimSets::Lori)
		animID -= 3;
	return animID;
}
GetAnimationIDFunc* GetVersionSpecificAnimationID = GetVersionSpecificAnimationID_124;

static int ConvertRabbitAnimNumToTSF(int base) { //from JJ2+, written by blur
	if (base <= 8) {
		//0-8		1-9
		base += 1;
	}
	else if (base >= 9 && base <= 17) {
		//9-17		11-19
		base += 2;
	}
	else if (base >= 18 && base <= 44) {
		//18-44		21-47
		base += 3;
	}
	else if (base >= 45 && base <= 51) {
		//45-51		55-61
		base += 10;
	}
	else if (base == 52) {
		//52		63
		base += 11;
	}
	else if (base == 53 || base == 54) {
		//53-54		67-68
		base += 14;
	}
	else if (base >= 55 && base <= 77) {
		//55-77		70-92
		base += 15;
	}
	else if (base >= 78 && base <= 81) {
		//78-81		104-107
		base += 26;
	}
	return base;
}

int RabbitAnimIDs[RabbitAnims::LAST];
void InitializeRabbitAnimIDs() {
	if (VersionTSF)
		for (int i = 0; i < RabbitAnims::LAST; ++i)
			RabbitAnimIDs[i] = i;
	else
		for (int i = 0; i < RabbitAnims::LAST; ++i)
			RabbitAnimIDs[i] = ConvertRabbitAnimNumToTSF(i);
}

bool IsTSF(bool& isTSF) {
	std::ifstream file("Anims.j2a", std::ios_base::binary);
	file.seekg(24, std::ios_base::beg);
	int animSetCount; file.read((char*)&animSetCount, sizeof(animSetCount));
	file.close();
	if (animSetCount == 109)
		isTSF = false;
	else if (animSetCount == 118)
		isTSF = true;
	else {
		ShowErrorMessage(L"Invalid Anims.j2a file");
		return false;
	}

	GetVersionSpecificAnimationID = isTSF ? GetVersionSpecificAnimationID_124 : GetVersionSpecificAnimationID_123;
	return true;
}