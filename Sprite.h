#pragma once
#include "kMath.h"
#include <d3d12.h>
#include <wrl.h>

class SpriteBase;

class Sprite { // メンバ関数
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteBase* spriteBase);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU);

private:
	HRESULT hr;
	SpriteBase* spriteBase_ = nullptr;

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		//Matrix4x4 World;
	};

	//Matrix4x4 WVP;

	// VertexResourceの作成
	void CreateVertexResource();
	// IndexResourceの作成
	void CreateIndexResource();
	// VertexBufferViewの作成
	void CreateVertexbufferView();
	// IndexBufferViewの作成
	void CreateIndexBufferView();
	// MaterialResourceの作成
	void CreateMaterialResource();
	// MaterialResourceにデータを書きこっむためのアドレスを取得してmaterialDataに割り当てる
	void CreateMapMaterialData();
	// TransformationMatrixの作成
	void CreateTransformationMatrix();
	// TransformationMatrixResourceにデータを書き込む溜めのアドレスを取得してTransformationMatrixDataに割り当てる
	void CreateMapTransformationMatrixData();
	
	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	// Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;
	// データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexbufferView{};

	//// インデックスリソースにデータを書き込む
	//indexDataSprite[0] = 0;
	//indexDataSprite[1] = 1;
	//indexDataSprite[2] = 2;
	//indexDataSprite[3] = 1;
	//indexDataSprite[4] = 3;
	//indexDataSprite[5] = 2;
};
