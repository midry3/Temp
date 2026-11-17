#pragma once
#include <Backend/Backend.h>
#include <Asset/Shader/PixelShader.h>

struct alignas(16) BattleConcentrationLineShaderParam {
	float progress;
	float strength;
	float texelSize[2];
	float focusCenter[2];
	float focusRadius;
	float rayCount;
	float time;
	float padding;
};

struct BattleConcentrationLineShaderBuffer {
	int handle;
	BattleConcentrationLineShaderParam* param;
};

class ConcentrationLineShader : public PixelShader {
protected:
	int Load() override {
		file = "ConcentrationLineShader.cso";
		return PixelShader::Load();
	}

public:
	static BattleConcentrationLineShaderBuffer CreateConstantBuffer() {
		BattleConcentrationLineShaderBuffer buffer;
		buffer.handle = CoreBackend::Shader::CreateBuffer(
			sizeof(BattleConcentrationLineShaderParam));
		buffer.param = static_cast<BattleConcentrationLineShaderParam*>(
			CoreBackend::Shader::Buffer(buffer.handle));
		return buffer;
	}
};
