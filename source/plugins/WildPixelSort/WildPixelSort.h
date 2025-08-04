#pragma once
#include <cstdint>
#include <string>
#include <FFGLSDK.h>

class WildPixelSort : public ffglqs::Effect
{
public:
	WildPixelSort();
	~WildPixelSort();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

private:
	ffglex::FFGLShader shader;  //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad;//!< Utility to help us render a full screen quad.

	std::shared_ptr< ffglqs::Param > th;
	std::shared_ptr< ffglqs::ParamRange > wildness;
	std::shared_ptr< ffglqs::ParamBool > vert;
	GLuint handle;
	FFGLTextureStruct* texture;
	bool frozen    = false;
    uint32_t m_frame = 0;
};
