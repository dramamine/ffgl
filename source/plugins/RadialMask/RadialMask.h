#pragma once
#include <string>
#include <FFGLSDK.h>

class RadialMask : public ffglqs::Plugin
{
public:
	RadialMask();
	~RadialMask();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

private:
	ffglex::FFGLShader shader;   //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad; //!< Utility to help us render a full screen quad.
	std::shared_ptr< ffglqs::ParamRange > angle;
	std::shared_ptr< ffglqs::Param > range;
};
