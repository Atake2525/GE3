#pragma once
#include <string>
#include <wrl.h>
#include <d3d12.h>
#include <vector>

#include "externels/DirectXTex/DirectXTex.h"

#pragma comment(lib, "d3d12.lib")

class DirectXBase;

class TextureManager {
private: // 完全に理解するまで編集禁止
	static TextureManager* instance; 

	TextureManager() = default; // コンストラクタの隠蔽
	~TextureManager() = default; // デストラクタの隠蔽
	TextureManager(TextureManager&) = delete; // コピーコンストラクタの封印
	TextureManager& operator=(TextureManager&) = delete; // コピー代入演算子の封印

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directXBase);

	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	void LoadTexture(const std::string& filePath);

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

private: 
	// テクスチャデータ構造体
	struct TextureData {
		std::string filePath; // 画像ファイルのパス
		DirectX::TexMetadata metadata; // 画像の幅や高さなどの情報
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // テクスチャリソース
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; // SRV作成時に必要なCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; // 描画コマンドに必要なGPUハンドル
	};

	// テクスチャデータ
	std::vector<TextureData> textureDatas;

private:
	DirectXBase* directxBase_;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;
};
