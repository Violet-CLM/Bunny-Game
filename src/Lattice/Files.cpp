#include "Windows.h"
#include "include/zlib.h"
#include "Files.h"
#include "Misc.h"

JazzFile::JazzFile(std::wstring& fp)
{
	Filepath = fp;
}
LevelTileset::LevelTileset(std::wstring& fp) : JazzFile(fp) {}
bool JazzFile::Open() {
	std::ifstream file(Filepath, std::ios_base::binary);
	file.seekg(0, std::ios_base::end);
	FileSize = file.tellg();
	file.seekg(0, std::ios_base::beg);

	const bool successful = ReadStream(file);

	file.close();

	return successful;
}
bool LevelTileset::ReadStream(std::ifstream& file) {
	if (FileSize <= SpecificFileTypeHeaderSize() + sizeof(GenericFileHeader))
		ShowErrorMessage(L"Invalid header (too small)");
	else if (!ReadSpecificFileHeader(file))
		;//method should include its own debug messages
	else if (file.read((char*)&GenericFileHeader, sizeof(GenericFileHeader)), GenericFileHeader.FileSize != FileSize)
		ShowErrorMessage(L"Invalid header (internal filesize doesn't match)");
	else if (!Uncompress(file, GenericFileHeader.DataSizes, UncompressedData))
		ShowErrorMessage(L"Error while uncompressing data streams");
	else return true;

	return false;
}
bool JazzFile::Uncompress(std::ifstream& File, const DataSizes_t* const dataSizes, std::array<std::vector<sf::Uint8>, FILE_DATASTREAMCOUNT>& uncompressedData)
{
	for (int i = 0; i < FILE_DATASTREAMCOUNT; ++i) {
		std::vector<sf::Uint8> compressedData(dataSizes[i].CData);
		File.read((char*)compressedData.data(), dataSizes[i].CData);
		uLongf destLen = dataSizes[i].UData;
		if (!destLen)
			continue;
		uncompressedData[i].resize(destLen);
		if (uncompress(uncompressedData[i].data(), &destLen, compressedData.data(), dataSizes[i].CData) != Z_OK) {
			ShowErrorMessage(L"Decompression failed");
			return false;
		}
		//else {
			//OutputDebugStringF(L"%d", dataSizes.UData);
		//}
	}
	return true;
}
