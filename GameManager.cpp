#include "GameManager.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

// コンストラクタ
GameManager::GameManager() {

	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	directxBase = new DirectXBase();
	directxBase->Initialize(winApp);


	// 各シーンの配列
	sceneArr_[TITLE] = std::make_unique<TitleScene>(winApp, directxBase);
	sceneArr_[STAGE] = std::make_unique<StageScene>(winApp, directxBase);

	// 初期化シーンの設定
	currentSceneNo_ = TITLE;
	prevSceneNo_ = 1;
}

// デストラクタによるdelete処理
GameManager::~GameManager() {
#ifdef DEBUG
	debugController->Release();
#endif // DEBUG
	// WindowAPIの終了処理
	winApp->Finalize();
	// WindowAPI解放
	delete winApp;

	directxBase->Finalize();
	delete directxBase;
	
}

int GameManager::Run() {

	// ゲームループ
	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		} else {
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			// シーンのチェック
			prevSceneNo_ = currentSceneNo_;
			currentSceneNo_ = sceneArr_[currentSceneNo_]->GetSceneNo();

			// シーン変更チェック
			if (prevSceneNo_ != currentSceneNo_) {
				sceneArr_[currentSceneNo_]->Initialize();
			}

			// 更新処理
			sceneArr_[currentSceneNo_]->Update(); // シーンごとの更新処理

			// 描画処理
			sceneArr_[currentSceneNo_]->Draw();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directxBase->GetCommandList().Get());
		}
	}

	#ifdef DEBUG
	debugController->Release();
#endif // DEBUG
	// WindowAPIの終了処理
	winApp->Finalize();
	// WindowAPI解放
	delete winApp;
	
	return 0;
}