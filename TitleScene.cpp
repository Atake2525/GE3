#include "TitleScene.h"
#include "WinApp.h"
#include "DirectXBase.h"


void TitleScene::Initialize(WinApp* winApp, DirectXBase* directxBase) {
	winApp_ = winApp;
	directxBase_ = directxBase;
	input_ = new Input();
	input_->Initialize(winApp_);

	spriteBase_ = new SpriteBase();
	spriteBase_->Initialize(directxBase_);

	TextureManager::GetInstance()->Initialize(directxBase_);
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");

	sprite_ = new Sprite();
	sprite_->Initialize(spriteBase_, "Resources/uvChecker.png");
	sprite_->SetPosition(Vector2{0.0f, 0.0f});

}

void TitleScene::Update() {
	if (input_->PushKey(DIK_SPACE)) {
		sceneNo = STAGE;
	}
	sprite_->Update();
	input_->Update();
}

void TitleScene::Draw() { 
	directxBase_->PreDraw();
	if (sceneNo == TITLE) {
		spriteBase_->sharedDrawSet();
		sprite_->Draw();
	}

	directxBase_->PostDraw();
}
