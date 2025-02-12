#pragma once
#include "IScene.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteBase.h"
#include "TextureManager.h"

// タイトルシーン
class TitleScene : public IScene {
public:

	void Initialize(WinApp* winApp, DirectXBase* directxBase) override;
	void Update() override;
	void Draw() override;

private:
	Input* input_ = nullptr;
	DirectXBase* directxBase_ = nullptr;
	WinApp* winApp_ = nullptr;

	SpriteBase* spriteBase_ = nullptr;
	Sprite* sprite_ = nullptr;
};
