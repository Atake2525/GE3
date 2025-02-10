#include "TextureManager.h"
#include "DirectXBase.h"
#include "StringUtility.h"

//using namespace Logger;
using namespace StringUtility;


TextureManager* TextureManager::instance = nullptr;

// Imguiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

void TextureManager::Initialize(DirectXBase* directXBasse) {
	directxBase_ = directXBasse;
	// SRVの数と同数
	textureDatas.reserve(DirectXBase::kMaxSRVCount);
}

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {

	// 読み込み済みテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(), 
		[&](TextureData& textureData) 
		{ 
			return textureData.filePath == filePath;
		});
	if (it != textureDatas.end()) {
		// 読み込み済みなら早期return
		return;
	}

	// テクスチャ枚数上限チェック
	assert(textureDatas.size() + kSRVIndexTop < DirectXBase::kMaxSRVCount);


	//// テクスチャファイルを読んでプログラムで扱えるようにする
	//DirectX::ScratchImage image{};
	//std::wstring filePathW = ConvertString(filePath);
	//HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	//assert(SUCCEEDED(hr));

	//// ミニマップの作成
	//DirectX::ScratchImage mipImages{};
	//hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	DirectX::ScratchImage mipImages{};
	mipImages = directxBase_->LoadTexture(filePath);

	// ミニマップ付きデータをメンバ変数に格納
	// テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();

	// テクスチャデータの読み込み
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = directxBase_->CreateTextureResource(textureData.metadata);

	directxBase_->UploadTextureData(textureData.resource, mipImages);

	
	// テクスチャデータの要素番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvHandleCPU = directxBase_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = directxBase_->GetSRVGPUDescriptorHandle(srvIndex);

	// SRVの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	
	// SRVの設定を行う
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	
	directxBase_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);


	// MipMap(ミニマップ) : 元画像より小さなテクスチャ群
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	// 読み込み済みのテクスチャデータを検索
	// 読み込み済みテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(), 
		textureDatas.end(), 
		[&](TextureData& textureData) { 
			return textureData.filePath == filePath;
		}
	);
	if (it != textureDatas.end()) {
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	// 範囲外指定違反チェック
	// テクスチャ枚数上限チェック
	assert(textureIndex < DirectXBase::kMaxSRVCount);

	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.at(textureIndex);
	//TextureData& textureData = textureDatas.back();
	return textureData.srvHandleGPU;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureIndex) {
	// 範囲指定外違反チェック
	assert(textureIndex < DirectXBase::kMaxSRVCount);

	TextureData& textureData = textureDatas.at(textureIndex);
	return textureData.metadata;
}