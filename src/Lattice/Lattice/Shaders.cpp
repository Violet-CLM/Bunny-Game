#include "Shaders.h"

std::string shaderSources[shader_LAST] = {
		//shader_NORMAL
	"uniform sampler2D texture;\
	\
	void main(void)\
	{\
		gl_FragColor = texture2D(texture, gl_TexCoord[0].xy);\
	}",
		//shader_PALETTED
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
sf::Shader* shaders[shader_LAST];