#include <vector>
#include <array>
#include <fstream>
#include "SFML/Config.hpp"
#include "Windows.h"
#include "J2S.h"
#include "Misc.h"

std::vector<char> LanguageStrings;
std::array<const char*, StringID::LAST> LanguageStringPointers;

const char* GetTranslatedString(StringID::StringID id) {
	return LanguageStringPointers[id];
}
void InitLoadStrings() {
	const std::wstring filename = L"English.j2s";
	std::ifstream file(filename, std::ios_base::binary);
	if (!file)
		ShowErrorMessageF(L"Could not load Language file \"%s\"", filename.c_str());
	else {
		sf::Uint32 numberOfStrings;
		file.read((char*)&numberOfStrings, sizeof(numberOfStrings));
		if (numberOfStrings != StringID::LAST)
			ShowErrorMessageF(L"Invalid Language Version in file \"%s\", found %u strings, expecting %u strings", filename.c_str(), numberOfStrings, StringID::LAST);
		else {
			sf::Uint32 sizeOfStringsSection;
			file.read((char*)&sizeOfStringsSection, sizeof(sizeOfStringsSection));
			LanguageStrings.resize(sizeOfStringsSection);
			file.read(LanguageStrings.data(), sizeOfStringsSection);
			const char* stringPointer = LanguageStrings.data();
			for (auto& it : LanguageStringPointers) {
				it = stringPointer;
				while (*stringPointer++);
			}
		}
	}
	file.close();
}
