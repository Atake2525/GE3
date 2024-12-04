#pragma once
#include "WinApp.h"
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <array>
#include <dxcapi.h>


class DirectXBase {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドル
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	// DSVとRTVも作る

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// getter
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device.Get(); }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return commandQueue.Get(); }
	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return commandAllocator.Get(); }

	// スワップチェイン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() const { return swapChain; }
	// SwapChainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> GetSwapChainResources(UINT i) const { return swapChainResources[i].Get(); };

	/// <summary>
	/// ディスクリプターヒープの作成関数
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

private:
	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> ComplieShader(const std::wstring& filePath, const wchar_t* profile);
	/// <summary>
	/// デバイス初期化
	/// </summary>
	void InitializeDevice();
	/// <summary>
	/// スワップチェイン初期化
	/// </summary>
	void CreateSwapChain();
	/// <summary>
	/// 深度バッファ初期化
	/// </summary>
	void CreateDepthBuffer();
	/// <summary>
	/// コマンド関連の初期化
	/// </summary>
	void InitializeCommands();
	/// <summary>
	/// 各種ディスクリプターヒープの作成
	/// </summary>
	void MakeDescriptorHeap();
	/// <summary>
	/// レンダーターゲットビューの初期化
	/// </summary>
	void InitializeRenderTargetView();
	/// <summary>
	/// 深度ステンシルビューの初期化
	/// </summary>
	void InitializeDepthStenCilView();
	/// <summary>
	/// フェンスの初期化
	/// </summary>
	void InitializeFence();
	/// <summary>
	///  ビューポート矩形の初期化
	/// </summary>
	void InitializeViewPortRect();
	/// <summary>
	/// シザー矩形の初期化
	/// </summary>
	void InitializeScissorRect();
	/// <summary>
	/// DXCコンパイラの生成
	/// </summary>
	void CreateDXCCompiler();
	/// <summary>
	/// ImGuiの初期化
	/// </summary>
	void InitializeImgui();
	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	HRESULT hr; 
	// DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// ポインタ
	WinApp* winApp = nullptr;
	// ビューポート矩形
	D3D12_VIEWPORT viewPort{};
	// シザー矩形
	D3D12_RECT scissor{};
	// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	// スワップチェイン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// SwapChainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources = nullptr;
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	// RTV様のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;

	// SRV様のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、shaderVisibleはtrue
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはFalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
};