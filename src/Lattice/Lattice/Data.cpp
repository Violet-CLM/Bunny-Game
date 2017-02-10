#include <fstream>
#include "include/zlib.h"
#include "Windows.h"
#include "Misc.h"
#include "Data.h"

struct DataFileHeader {
	char Magic[8];
	sf::Uint32 Version;
	sf::Uint32 FileSize;
	sf::Uint32 CRC;
	sf::Uint32 CSize;
	sf::Uint32 USize;
};
static_assert(sizeof(DataFileHeader) == 28, "DataFile's DataFileHeader incorrect size!");
struct DataSubfileHeader {
	char Name[36];
	sf::Uint32 Location;
	sf::Uint32 Unused; //well, unimportant at least
	sf::Uint32 CSize;
	sf::Uint32 USize;
};
static_assert(sizeof(DataSubfileHeader) == 52, "DataSubfileHeader incorrect size!");

void LoadDataFromFile(const std::wstring & filename, std::initializer_list<const char*> dataNames, std::initializer_list<DataTarget*> dataTargets) {
	if (!dataNames.size())
		ShowErrorMessage(L"At least one data chunk must be requested from LoadDataFromFile.");
	else if (dataNames.size() != dataTargets.size())
		ShowErrorMessage(L"The lists passed to LoadDataFromFile must be the same size.");
	else {
		std::ifstream file(filename, std::ios_base::binary);
		file.seekg(0, std::ios_base::end);
		std::streamoff FileSize = file.tellg();
		file.seekg(0, std::ios_base::beg);

		if (FileSize < 28)
			ShowErrorMessage(L"Invalid header (too small)");
		else {
			DataFileHeader FileHeader;
			file.read((char*)&FileHeader, sizeof(DataFileHeader));

			if (memcmp(&FileHeader, "PLIB\xDE\xAD\xBA\xBE\x00\x01\x00\x00", 12))
				ShowErrorMessage(L"Invalid header (magic string doesn't match)"); //technically I'm subsuming the version in there too but AFAIK there are no other .j2d formats out there so it doesn't really matter
			else if (FileHeader.FileSize != FileSize)
				ShowErrorMessage(L"Invalid header (internal filesize doesn't match)");
			else if (FileHeader.USize % sizeof(DataSubfileHeader) != 0)
				ShowErrorMessage(L"Invalid header (subfile header list is an invalid size)");
			//in theory there should be a CRC check here but maybe I don't really care
			else {
				std::vector<DataSubfileHeader> SubfileHeaders(FileHeader.USize / sizeof(DataSubfileHeader));
				std::vector<sf::Uint8> compressedData(FileHeader.CSize);
				file.read((char*)compressedData.data(), FileHeader.CSize);

				if (uncompress((Bytef*)SubfileHeaders.data(), (uLongf*)&FileHeader.USize, compressedData.data(), FileHeader.CSize) != Z_OK)
					ShowErrorMessageF(L"Decompression of %s failed", filename);
				else {
					auto name = dataNames.begin();
					auto target = dataTargets.begin();
					while (true) {
						for (const auto subfileHeader : SubfileHeaders) {
							if (!strcmp(*name, subfileHeader.Name)) { //found the right subfile
								compressedData.resize(subfileHeader.CSize);
								(*target)->resize(subfileHeader.USize);
								file.seekg(sizeof(DataFileHeader) + FileHeader.CSize + subfileHeader.Location, std::ios_base::beg);
								file.read((char*)compressedData.data(), subfileHeader.CSize);
								if (uncompress((*target)->data(), (uLongf*)&subfileHeader.USize, compressedData.data(), subfileHeader.CSize) != Z_OK)
									ShowErrorMessageF(L"Decompression of subfile %s in %s failed (%u into %u)", WStringFromCharArray(subfileHeader.Name).c_str(), filename.c_str(), subfileHeader.CSize, subfileHeader.USize);
								break; //don't need to keep looping through subfilenames
							}
						}

						if (++name == dataNames.end())
							break;
						++target;
					}
				}
			}
		}

		file.close();
	}
}
