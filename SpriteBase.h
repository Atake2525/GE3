#include <d3d12.h>
#include "WinApp.h"
#include <wrl.h>
#include <dxcapi.h>

#pragma once
class DirectXBase;

class SpriteBase {
public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directxBase);

	// 共通描画設定
	void sharedDrawSet();

	DirectXBase* GetDxBase() const { return directxBase_; }

private:
	HRESULT hr;
	DirectXBase* directxBase_;

	void CreateRootSignature();
	// RootSinature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	void CreateGraphicsPileLineState();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState;
	/*descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;*/
};
