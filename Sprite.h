#pragma once
#include "kMath.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>

class SpriteBase;

class Sprite { // メンバ関数
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteBase* spriteBase, std::string textureFilePath);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(/*D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU*/);

	// getter

	// 平行移動
	const Vector2& GetPosition() const { return position; }
	// 回転
	const float& GetRotation() const { return rotation; }
	// 拡縮
	const Vector2& GetScale() const { return scale; }
	// 色
	const Vector4& GetColor() const { return materialData->color; }

	// アンカーポイント
	const Vector2& GetAnchorPoint() const { return anchorPoint; }

	// 左右フリップ
	const bool& GetIsFlipX() const { return isFlipX; }
	
	// 上下フリップ
	const bool& GetIsFlipY() const { return isFlipY; }

	// テクスチャ左上座標
	const Vector2& GetTextureLeftTop() const { return textureLeftTop; }
	// テクスチャ切り出しサイズ
	const Vector2& GetTextureSize() const { return textureSize; }
	 
	// setter
	
	// 平行移動
	void SetPosition(const Vector2& pos) { position = pos; }
	// 回転
	void SetRotation(const float& rot) { rotation = rot; }
	// 拡縮
	void SetScale(const Vector2& size) { scale = size; }
	// 色
	void SetColor(const Vector4& color) { materialData->color = color; }

	// Position、Rotate、Scale、Colorの更新を一つの関数で行う場合
	void SetPRSC(const Vector2& pos, const float& rotate, const Vector2& size, const Vector4& color);

	// setter アンカーポイント
	void SetAnchorPoint(const Vector2& ancPoint) { anchorPoint = ancPoint; }

	// 左右フリップ
	void SetFlipX(bool FlipX) { isFlipX = FlipX; }

	// 上下フリップ
	void SetFlipY(bool FlipY) { isFlipY = FlipY; }

	// テクスチャ左上座標
	void SetTextureLeftTop(Vector2 LeftTop) { textureLeftTop = LeftTop; }

	// テクスチャ切り出しサイズ
	void SetTextureSize(Vector2 Size) { textureSize = Size; }
	
	// テクスチャ変更
	void ChangeTexture(std::string textureFilePath);

private: // Transform + color
	Vector2 position = {0.0f, 0.0f};
	float rotation = 0.0f;
	Vector2 scale = {1.0f, 1.0f};
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};

	// アンカーポイント
	Vector2 anchorPoint = {0.0f, 0.0f};

	// 左右フリップ
	bool isFlipX = false;
	// 上下フリップ
	bool isFlipY = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop = {0.0f, 0.0f};
	// テクスチャ切り出しサイズ
	Vector2 textureSize = {100.0f, 100.0f};

	// テクスチャ番号
	uint32_t textureIndex = 0;

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

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();
	
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

};
