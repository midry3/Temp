#pragma once
#include <Backend/Backend.h>
#include <Asset/Shader/PixelShader.h>

struct alignas(16) BattleScreenFlashShaderParam {
	float color[4];
	float progress;
	float padding[3];
};

class ScreenFlashShader : public PixelShader {
protected:
	int Load() override {
		file = "ScreenFlashShader.cso";
		return PixelShader::Load();
	}
};
