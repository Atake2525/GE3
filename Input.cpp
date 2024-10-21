#include "Input.h"
#include "cassert"

void Input::Initialize() {
	// DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	WNDCLASS wc{};
	HRESULT hr = DirectInput8Create(wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));
	// DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput = nullptr;
}