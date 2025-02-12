#pragma once

#include <memory>
#include "IScene.h"
#include "TitleScene.h"
#include "StageScene.h"
#include "WinApp.h"
#include "D3DResourceLeakChecker.h"
#include "DirectXBase.h"

class GameManager {
private:
	// シーンを保持するメンバ変数
	std::unique_ptr<IScene> sceneArr_[2];

	// どのシーンを呼び出すかを管理する変数
	int currentSceneNo_; // 現在のシーン
	int prevSceneNo_; // 前シーン

	// D3D12ResourceChecker
	D3DResourceLeakChecker d3dResourceLeakChecker;


	WinApp* winApp = nullptr;
	DirectXBase* directxBase = nullptr;

public:
	GameManager(); // コンストラクタ
	~GameManager(); // デストラクタ
	
	int Run(); // ゲームループを呼び出す
};
