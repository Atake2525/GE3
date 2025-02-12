#pragma once
#include "IScene.h"

class WinApp;
class DirectXBase;

// ステージシーン
class StageScene : public IScene {
public:
	StageScene(WinApp* winApp, DirectXBase* directxBase);
	
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	WinApp* winApp_ = nullptr;
	DirectXBase* directxBase_ = nullptr;
};
