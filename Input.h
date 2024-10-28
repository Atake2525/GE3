#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "WinApp.h"

// 入力
class Input {
public: // メンバ関数
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);
	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号( DIK_0 等)</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(BYTE keyNumber);
	/// <summary>
	/// キーのリターンをチェック
	/// </summary>
	/// <param name="keynumber">キー番号( DIK_0 等)</param>
	/// <returns>リターンしたか</returns>
	bool ReturnKey(BYTE keynumber);

private: // メンバ変数
	ComPtr<IDirectInput8> directInput;
	/// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	// 全キーの状態
	BYTE key[256] = {};
	// 前回の全キーの状態
	BYTE keyPre[256] = {};

	WinApp* winApp_ = nullptr;

};
