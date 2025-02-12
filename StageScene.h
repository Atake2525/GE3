#pragma once
#include "IScene.h"


// ステージシーン
class StageScene : public IScene {
public:
	
	void Initialize(WinApp* winApp, DirectXBase* directxBase) override;
	void Update() override;
	void Draw() override;

private:
	WinApp* winApp_ = nullptr;
	DirectXBase* directxBase_ = nullptr;
};
