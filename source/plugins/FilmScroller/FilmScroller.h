#pragma once
#include <string>
#include <FFGLSDK.h>
using namespace ffglqs;
using namespace ffglex;
class FilmScroller : public ffglqs::Plugin
{
public:
	FilmScroller();
	~FilmScroller();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

private:
	ffglex::FFGLShader shader;  //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad;//!< Utility to help us render a full screen quad.
	std::shared_ptr< ParamRange > vtiles;
	std::shared_ptr< Param > tilex;
	std::shared_ptr< Param > offsetx;
	std::shared_ptr< Param > offsety;
	std::shared_ptr< Param > scroll;
	std::shared_ptr< Param > huescale;
	std::shared_ptr< Param > hueoffset;
	std::shared_ptr< Param > filmholesize;
	std::shared_ptr< Param > filmholepct;
	std::shared_ptr< Param > filmalpha;
	std::shared_ptr< Param > lanewidth;
	std::shared_ptr< ParamBool > tilesonly;

	int m_frames;
};
