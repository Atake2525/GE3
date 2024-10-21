#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// 入力
class Input {
public: // メンバ関数
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private: // メンバ変数
	/// キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
};
