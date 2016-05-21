#pragma once
#include <set>
#include <map>
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"

#define MS_PER_UPDATE (1000.0 / 70.0) // 1000.0 / 8.0;
#define WINDOW_WIDTH_PIXELS 640
#define WINDOW_HEIGHT_PIXELS 480
#define WINDOW_WIDTH_TILES (WINDOW_WIDTH_PIXELS + TILEWIDTH - 1) / TILEWIDTH //rounded up
#define WINDOW_HEIGHT_TILES (WINDOW_HEIGHT_PIXELS + TILEHEIGHT - 1) / TILEHEIGHT //rounded up

#define EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY 5

typedef std::vector<sf::Vertex> VertexVector; //basically an sf::VertexArray but with all the methods exposed
typedef sf::Rect<unsigned int> SpriteCoordinateRectangle;

#define TILESETDIMENSION 1024
#define TILESETDIMENSIONINTILES (TILESETDIMENSION/TILEWIDTH)
#define TILESPERTEXTURE (TILESETDIMENSIONINTILES*TILESETDIMENSIONINTILES)
#define NUMBEROFTILESETTEXTURES (TILE_HFLIPPED / TILESPERTEXTURE)
#define COLORSPERPALETTE 256
#define BYTESPER32BITPIXEL sizeof(sf::Uint32)

//LUTs stored in the GPU in the texture "tables", corresponding to paletteTexture32 in the CPU. Each row is 256 colors wide and is a single LUT (or a space where one could be added later). They may be rearranged freely--the shaders in drawing32shaders.cpp are generated using these enum values to determine which row to use for any given lookup.
enum paletteLineNames {
	pallineNORMALPALETTE = 0,
	pallineXPOSTOINDEX,
	//pallineBRIGHTNESS, pallineBRIGHTNESSTOFREEZECOLORS, pallineMENUPLAYERSPRITEMAPPING, pallineTBGFADEINTENSITY, pallineHEATEFFECTOFFSETS, pallinePLAYERS, pallineGEMS = pallinePLAYERS + 32,
	pallineNUMBEROFPALLINES = 2//64 //lowest containing power of 2; even if the platform supports other sizes, this makes for cleaner division and therefore access to specific lines
};
enum shaderTypes { //or do I move this into the game-specific code?
	shader_NORMAL,
	shader_LAST
};
#define PALLINEHEIGHT (1.f/float(pallineNUMBEROFPALLINES)) //texture positions in GLSL are not 0-255 (or 0-63 or whatever) but 0.0 to 1.0. This is therefore the height of a single line in the "tables" texture, and e.g. (float(pallineBRIGHTNESS) * PALLINEHEIGHT) points to line pallineBRIGHTNESS in GLSL coordinates
#define TOPALLINE(A) (float(A) * PALLINEHEIGHT) //a macro for the above location purpose, to be used while writing shaders

#define BITS_PER_MASKBYTE 8
#define TILES_IN_WORD 4
#define TILE_HFLIPPED 0x1000
#define TILE_VFLIPPED 0x2000
#define TILE_ROTATED 0x4000
#define MAX_TILES TILE_HFLIPPED
#define TILE_ID_RANGE (MAX_TILES-1)
#define TILEWIDTH 32
#define TILEHEIGHT 32

typedef sf::Uint16 TileID;
typedef sf::Uint16 WordID;
typedef sf::Uint8 EventID;
typedef float Position;

#define FILE_HEADER_COPYRIGHTLENGTH 180
#define FILE_HEADER_MAGICLENGTH 4
#define FILE_NAMEFIELDLENGTH 32
#define FILE_DATASTREAMCOUNT 4

const char Jazz2FileCopyrightString[FILE_HEADER_COPYRIGHTLENGTH + sizeof('\00')] = "                      Jazz Jackrabbit 2 Data File\x0D\x0A\x0D\x0A         Retail distribution of this data is prohibited without\x0D\x0A             written permission from Epic MegaGames, Inc.\x0D\x0A\x0D\x0A\x1A";
typedef sf::Uint16 FileVersion;

#define LEVEL_HELPSTRINGCOUNT 16
#define LEVEL_HELPSTRINGLENGTH 512

#define LEVEL_LAYERCOUNT 8
#define SPRITELAYER 3

class Level;
class ObjectInitialization;
typedef std::set<int> PreloadedAnimationsList;
typedef void ObjectActivityFunction(Level&);
typedef std::map<EventID, ObjectInitialization> ObjectList;