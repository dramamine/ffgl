#pragma once
#include <FFGLSDK.h>
using namespace ffglqs;
using namespace ffglex;

class Bloom : public Effect
{
public:
	Bloom();
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* inputTextures ) override;
	FFResult DeInitGL() override;
	~Bloom();

private:
	std::shared_ptr< ParamRange > threshold;
	std::shared_ptr< ParamRange > radius;
	std::shared_ptr< ParamRange > intensity;
	std::shared_ptr< ParamBool > hq;
	std::shared_ptr< ParamBool > antiFlicker;
	std::shared_ptr< ParamRange > jitter;
	FFGLShader downSampleFilter;
	FFGLShader upSampleFilter;
	FFGLShader final;
};
