#pragma once

enum SCENE {TITLE, STAGE};

class IScene {
protected:
	// シーン番号を管理する変数
	static int sceneNo;

public:
	// 継承先で実装される関数
	// 中小クラスなので純粋仮想関数とする
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	
	// 仮想デストラクタを用意しないと警告される
	virtual ~IScene();

	// シーン番号のゲッター
	int GetSceneNo();
};
