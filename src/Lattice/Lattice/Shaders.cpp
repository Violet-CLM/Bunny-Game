#include "Shaders.h"

std::vector<std::string> ShaderSources = {
		//Shaders::Normal
	"uniform sampler2D texture;\
	\
	void main(void)\
	{\
		gl_FragColor = texture2D(texture, gl_TexCoord[0].xy);\
	}",
		//Shaders::Paletted
	"uniform sampler2D texture;\
	uniform sampler2D tables;\
	\
	void main(void)\
	{\
		vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
		vec4 pixel = texture2D(tables, vec2(index.r, 0)); \
		gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a);\
	}"
};
std::vector<sf::Shader*> Shaders;