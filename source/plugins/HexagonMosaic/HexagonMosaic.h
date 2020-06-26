#pragma once
#include <string>
#include <FFGLSDK.h>

class HexagonMosaic : public ffglqs::Plugin
{
public:
	HexagonMosaic();
	~HexagonMosaic();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

private:
	ffglex::FFGLShader shader;   //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad; //!< Utility to help us render a full screen quad.
	std::shared_ptr< ffglqs::Param > width;
	std::shared_ptr< ffglqs::Param > blending;
	std::shared_ptr< ffglqs::Param > xoffset;
	std::shared_ptr< ffglqs::Param > yoffset;
};
