#include "BunnyVersionDependentStuff.h"
#include "BunnyObjectList.h"
#include "Windows.h"
#include "Misc.h"

int GetVersionSpecificAnimationID(int originalAnimID, bool isTSF) {
	if (isTSF)
		return originalAnimID;

	if (originalAnimID == AnimSets::XBilsy)
		return GetVersionSpecificAnimationID(AnimSets::BilsBoss, false);
	if (originalAnimID == AnimSets::XLizard)
		return GetVersionSpecificAnimationID(AnimSets::Lizard, false);
	if (originalAnimID == AnimSets::XTurtle)
		return GetVersionSpecificAnimationID(AnimSets::Turtle, false);
	if (originalAnimID == AnimSets::ZDog)
		return GetVersionSpecificAnimationID(AnimSets::Dog, false);
	if (originalAnimID == AnimSets::ZSpark)
		return GetVersionSpecificAnimationID(AnimSets::Spark, false);

	int animID = originalAnimID;
	if (originalAnimID >= AnimSets::EndTuneLori)
		animID -= 1;
	if (originalAnimID >= AnimSets::Lori)
		animID -= 3;
	return animID;
}


static int ConvertRabbitAnimNumToTSF(int base) { //from JJ2+, written by blur
	if (base >= 1 && base <= 9) {
		//1-9		0-8
		base -= 1;
	}
	else if (base >= 11 && base <= 19) {
		//11-19		9-17
		base -= 2;
	}
	else if (base >= 21 && base <= 47) {
		//21-47		18-44
		base -= 3;
	}
	else if (base >= 55 && base <= 61) {
		//55-61		45-51
		base -= 10;
	}
	else if (base == 63) {
		//63		52
		base -= 11;
	}
	else if (base == 67 || base == 68) {
		//67-68		53-54
		base -= 14;
	}
	else if (base >= 70 && base <= 92) {
		//70-92		55-77
		base -= 15;
	}
	else if (base >= 104 && base <= 107) {
		//104-107	78-81
		base -= 26;
	}
	return base;
}

int RabbitAnimIDs[RabbitAnims::LAST];
void InitializeRabbitAnimIDs(bool isTSF) {
	if (isTSF)
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
	return true;
}