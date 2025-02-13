#include "Sprite.h"
#include "SpriteBase.h"
#include <cassert>
#include "DirectXBase.h"
#include "TextureManager.h"

using namespace Microsoft::WRL;

void Sprite::Initialize(SpriteBase* spriteBase, std::string textureFilePath) {
	spriteBase_ = spriteBase;
	// VertexResourceの作成
	CreateVertexResource();
	// IndexResourceの作成
	CreateIndexResource();
	// VertexBufferViewの作成
	CreateVertexbufferView();
	// IndexBufferViewの作成
	CreateIndexBufferView();

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// マテリアルリソースの作成
	CreateMaterialResource();
	// MaterialResourceにデータを書きこっむためのアドレスを取得してmaterialDataに割り当てる
	CreateMapMaterialData();

	// MaterialDataの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	// TransformationMatrixResourceの作成
	CreateTransformationMatrix();
	// TransformationMatrixResourceにデータを書き込む溜めのアドレスを取得してTransformationMatrixDataに割り当てる
	CreateMapTransformationMatrixData();
	
	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	//transformationMatrixData->World = MakeIdentity4x4();

	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	AdjustTextureSize();
}

void Sprite::SetPRSC(const Vector2& pos, const float& rotate, const Vector2& size, const Vector4& color) {
	position = pos;
	rotation = rotate;
	scale = size;
	materialData->color = color;
}

void Sprite::AdjustTextureSize() {
	// テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
}


void Sprite::Update() {

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	// 左右反転
	if (isFlipX) {
		left = -left;
		right = -right;
	}
	// 上下反転
	if (isFlipY) {
		top = -top;
		bottom = -bottom;
	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	vertexData[0].position = {left, top, 0.0f, 1.0f}; // 左上
	vertexData[0].texcoord = { tex_left, tex_top };
	vertexData[1].position = {right, top, 0.0f, 1.0f }; // 右上
	vertexData[1].texcoord = { tex_right, tex_top };
	vertexData[2].position = {right, bottom, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { tex_right, tex_bottom };
	vertexData[3].position = {left, bottom, 0.0f, 1.0f}; // 左下
	vertexData[3].texcoord = {tex_left, tex_bottom};
	
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 3;
	indexData[3] = 3;
	indexData[4] = 1; 
	indexData[5] = 2;
	
	Transform uvTransform{
	    {1.0f, 1.0f, 1.0f},
	    {0.0f, 0.0f, 0.0f},
	    {0.0f, 0.0f, 0.0f},
	};

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	materialData->uvTransform = uvTransformMatrix;

	// Transform変数を作る
	Transform transform{
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	Transform cameraTransform{
	    {1.0f, 1.0f, 1.0f },
        {0.0f, 0.0f, 0.0f },
        {0.0f, 0.0f, -5.0f}
    };

	transform.translate = {position.x, position.y};
	transform.rotate.z = rotation;
	transform.scale = {scale.x / textureSize.x * textureSize.x, scale.y / textureSize.y * textureSize.y, 1.0f};

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.1f, 100.0f);

	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//transformationMatrixData->World = worldMatrix;
}

void Sprite::ChangeTexture(std::string textureFilePath) {
	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	AdjustTextureSize();

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	// 左右反転
	if (isFlipX) {
		left = -left;
		right = -right;
	}
	// 上下反転
	if (isFlipY) {
		top = -top;
		bottom = -bottom;
	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureIndex);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	vertexData[0].position = {left, top, 0.0f, 1.0f}; // 左上
	vertexData[0].texcoord = {tex_left, tex_top};
	vertexData[1].position = {right, top, 0.0f, 1.0f}; // 右上
	vertexData[1].texcoord = {tex_right, tex_top};
	vertexData[2].position = {right, bottom, 0.0f, 1.0f}; // 右下
	vertexData[2].texcoord = {tex_right, tex_bottom};
	vertexData[3].position = {left, bottom, 0.0f, 1.0f}; // 左下
	vertexData[3].texcoord = {tex_left, tex_bottom};

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 3;
	indexData[3] = 3;
	indexData[4] = 1;
	indexData[5] = 2;
}

void Sprite::Draw(/*D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU*/) { 
	// VertexBufferViewを設定
	CreateVertexbufferView();
	spriteBase_->GetDxBase()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	// IndexBufferViewを設定
	CreateIndexBufferView();
	spriteBase_->GetDxBase()->GetCommandList()->IASetIndexBuffer(&indexbufferView); // VBVを設定

	// マテリアルCBufferの場所を設定
	spriteBase_->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// 座標変換行列CBufferの場所を設定
	// wvp用のCBufferの場所を設定
	spriteBase_->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	//spriteBase_->GetDxBase()->GetCommandList()->SetGraphicsRootDescriptorTable(2, spriteBase_->GetDxBase()->GetSRVGPUDescriptorHandle(1));
	spriteBase_->GetDxBase()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

	// 描画! (DrawCall)
	spriteBase_->GetDxBase()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexResource() {
	// Sprite用の頂点リソースを作る
	vertexResource = spriteBase_->GetDxBase()->CreateBufferResource(sizeof(VertexData) * 6);
}

void Sprite::CreateVertexbufferView() {
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateIndexResource() { 
	indexResource = spriteBase_->GetDxBase()->CreateBufferResource(sizeof(uint32_t) * 6); 
}

void Sprite::CreateIndexBufferView() {
	// リソースの先頭のアドレスから使う
	indexbufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexbufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexbufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::CreateMaterialResource() {
	// Sprite用のマテリアルリソースを作る
	materialResource = spriteBase_->GetDxBase()->CreateBufferResource(sizeof(Material));
}

void Sprite::CreateMapMaterialData() {
	// データを書き込む
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
}

void Sprite::CreateTransformationMatrix() {
	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	//transformationMatrixResource = spriteBase_->GetDxBase()->CreateBufferResource(spriteBase_->GetDxBase()->GetDevice(), sizeof(TransformationMatrix));
	transformationMatrixResource = spriteBase_->GetDxBase()->CreateBufferResource(sizeof(TransformationMatrix));
}

void Sprite::CreateMapTransformationMatrixData() { 
	//transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
}