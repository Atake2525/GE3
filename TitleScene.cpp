#include "TitleScene.h"
#include "WinApp.h"
#include "DirectXBase.h"

TitleScene::TitleScene(WinApp* winApp, DirectXBase* directxBase) {
	winApp_ = winApp;
	directxBase_ = directxBase;
}

void TitleScene::Initialize() {
	input_ = new Input();
	input_->Initialize(winApp_);

}

void TitleScene::Update() {
	if (input_->PushKey(DIK_SPACE)) {
		sceneNo = STAGE;
	}
	input_->Update();
}

void TitleScene::Draw() { 
	directxBase_->PreDraw();



	directxBase_->PostDraw();
}
