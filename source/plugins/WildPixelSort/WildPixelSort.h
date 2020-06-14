#pragma once
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
	GLuint handle;
	FFGLTextureStruct* texture;
	bool frozen    = false;
	UINT32 m_frame = 0;
};
