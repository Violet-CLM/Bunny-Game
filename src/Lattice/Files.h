#pragma once
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include "SFML/Config.hpp"
#include "Constants.h"

struct DataSizes_t {
	sf::Uint32 CData; // compressed size of
	sf::Uint32 UData; // uncompressed size of
};
class JazzFile {
protected:
	FileVersion Version;
	std::wstring Filepath;
	std::streamoff FileSize;
	bool FileLoadedSuccessfully;

	virtual unsigned int SpecificFileTypeHeaderSize() = 0;
	virtual bool ReadSpecificFileHeader(std::ifstream&) = 0;
	virtual bool ReadStream(std::ifstream&) = 0;
	JazzFile(std::wstring&);
public:

	bool Open();
	static bool Uncompress(std::ifstream&, const DataSizes_t* const, std::array<std::vector<sf::Uint8>, FILE_DATASTREAMCOUNT>&);
};
class LevelTileset : public JazzFile {
protected:
	struct GenericFileHeader_t {
	public:
		sf::Uint32 FileSize;
		sf::Uint32 CRC32;
		DataSizes_t DataSizes[FILE_DATASTREAMCOUNT];
	} GenericFileHeader;
	static_assert(sizeof(GenericFileHeader_t) == 40, "GenericFileHeader incorrect size!");

	std::array<std::vector<sf::Uint8>, FILE_DATASTREAMCOUNT> UncompressedData;

	bool ReadStream(std::ifstream&) override;
	LevelTileset(std::wstring&);
public:
	std::wstring Name;
};