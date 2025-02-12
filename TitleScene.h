#pragma once
#include "IScene.h"
#include "Input.h"

class WinApp;
class DirectXBase;

// タイトルシーン
class TitleScene : public IScene {
public:
	TitleScene(WinApp* winApp, DirectXBase* directxBase);

	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	Input* input_ = nullptr;
	DirectXBase* directxBase_ = nullptr;
	WinApp* winApp_ = nullptr;
};
