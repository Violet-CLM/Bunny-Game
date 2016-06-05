#include "BunnyShaders.h"
#include "Misc.h"

void AppendBunnyShaders(std::vector<std::string>& shaderSources)
{
	shaderSources.push_back("uniform sampler2D texture;\
		uniform sampler2D tables;\
		uniform float param;\
		\
		void main(void)\
		{\
			vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
			vec4 pixel = texture2D(tables, vec2(mod(index.r, 0.5), param)); \
			gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a * 0.75);\
		}");
}
