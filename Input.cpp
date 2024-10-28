#include "Input.h"
#include "cassert"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


void Input::Initialize(WinApp* winApp) {
	this->winApp_ = winApp;
	// DirectInputのインスタンス生成
	HRESULT hr = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));
	// キーボードデバイス生成
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));
	// 入力データー形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	// 排他的制御レベルのセット
	hr = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

}

void Input::Update() {

	// 前回のキー入力の保存
	memcpy(keyPre, key, sizeof(key));

	// キーボード情報
	keyboard->Acquire();
	// 全キーの入力情報を取得する
	keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber) {
	// 指定のキーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false; 
}

bool Input::TriggerKey(BYTE keyNumber) { 
	// 指定のキーをトリガー状態ならtrueを返す
	if (key[keyNumber] && !keyPre[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::ReturnKey(BYTE keyNumber) {
	// 指定のキーをトリガー状態ならtrueを返す
	if (!key[keyNumber] && keyPre[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}