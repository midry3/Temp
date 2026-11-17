#pragma once
#include <Backend/Backend.h>
#include <Asset/Shader/PixelShader.h>

struct alignas(16) BattleBlurShaderParam {
	float progress;
	float strength;
	float texelSize[2];
	float focusCenter[2];
	float focusRadius;
	float padding;
};

struct BattleBlurShaderBuffer {
	int handle;
	BattleBlurShaderParam* param;
};

class BlurInShader : public PixelShader {
protected:
	int Load() override {
		file = "BlurInShader.cso";
		return PixelShader::Load();
	}

public:
	static BattleBlurShaderBuffer CreateConstantBuffer() {
		BattleBlurShaderBuffer buffer;
		buffer.handle = CoreBackend::Shader::CreateBuffer(sizeof(BattleBlurShaderParam));
		buffer.param = static_cast<BattleBlurShaderParam*>(
			CoreBackend::Shader::Buffer(buffer.handle));
		return buffer;
	}
};
