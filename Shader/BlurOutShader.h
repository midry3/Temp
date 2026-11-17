#pragma once
#include "BlurInShader.h"

class BlurOutShader : public PixelShader {
protected:
	int Load() override {
		file = "BlurOutShader.cso";
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
