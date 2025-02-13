#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
//#include <d3d12.h>
//#include <dxgi1_6.h>
#include <cassert>
//#include <dxgidebug.h>
//#include <dxcapi.h>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "kMath.h"
#include "Input.h"
#include "WinApp.h"
#include "Logger.h"
#include "StringUtility.h"
#include "D3DResourceLeakChecker.h"
#include "DirectXBase.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "SpriteBase.h"
#include "Sprite.h"
#include "TextureManager.h"
//#include "externels/DirectXTex/DirectXTex.h"



//#pragma comment(lib, "dxcompiler.lib")
//#pragma comment(lib, "d3d12.lib")
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "dxguid.lib")

//struct Material
//{
//	Vector4	color;
//	int32_t enableLighting;
//	float padding[3];
//	Matrix4x4 uvTransform;
//};

struct MaterialData
{
	std::string textureFilePath;
};

struct ModelData
{
	std::vector<VertexData> vertices;
	MaterialData material;
};

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 1, 中で必要となる変数の宣言
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ１行を格納するもの
	// 2, ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	// 3, 実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)){
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd"){
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath+"/"+textureFilename;
		}
	}
	// 4, MaterialDataを返す
	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	// 1. 中で必要となる変数の宣言
	ModelData modelData; // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	// 2. ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	// 3. 実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v"){
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt"){
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn"){
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f"){
			VertexData triangle[3];

			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex){
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element){
					std::string index;
					std::getline(v, index, '/'); // /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素を値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);
				position.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;
				normal.x *= -1.0f;

				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点を逆順で登録することで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	// 4. ModelDataを返す
	return modelData;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	//ディスクリプターヒープの作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc{};
	DescriptorHeapDesc.Type = heapType;
	DescriptorHeapDesc.NumDescriptors = numDescriptors;
	DescriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプターヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;

}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#pragma region 基盤システムの初期化
	// D3D12ResourceChecker
	D3DResourceLeakChecker d3dResourceLeakChecker;

	// ポインタ
	WinApp* winApp = nullptr;
	DirectXBase* directxBase = nullptr;

	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();
	// DirectXBaseの初期化
	directxBase = new DirectXBase();
	directxBase->Initialize(winApp);

	SpriteBase* spriteBase = nullptr;
	// スプライト共通部の初期化
	spriteBase = new SpriteBase;
	spriteBase->Initialize(directxBase);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(directxBase);
	
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	

#pragma endregion 基盤システムの初期化

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

	#endif
#pragma region 最初のシーンの初期化

	std::vector<Sprite*> sprites;
	for (uint32_t i = 0; i < 4; i++) {
		Sprite* sprite = new Sprite();
		if (i % 2 == 0) {
			//TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
			sprite->Initialize(spriteBase, "Resources/monsterBall.png");
		} else {
			//TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
			sprite->Initialize(spriteBase, "Resources/uvChecker.png");
		}
		sprite->SetPosition(Vector2{100.0f + 100.0f * float(i),10.0f * float(i) * 20});
		sprites.push_back(sprite);
	}
	Sprite* sprite_;
	sprite_ = new Sprite();
	sprite_->Initialize(spriteBase, "Resources/uvChecker.png");
	sprite_->SetPosition(Vector2{640.0f, 360.0f});


#pragma endregion 最初のシーンの終了
	// ポインタ
	Input* input = nullptr;

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);


	//出ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//// dxcCompilerを初期化
	//Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	//Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	//HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	//assert(SUCCEEDED(hr));
	//hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	//assert(SUCCEEDED(hr));

	//// 現時点でincludeはしないが、includeに対応するために設定を行っておく
	//Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	//hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	//assert(SUCCEEDED(hr));

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合また別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(VertexData) * 6;
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// モデル読み込み
	ModelData modelData = LoadObjFile("Resources", "axis.obj");
	DirectX::ScratchImage mipImages2 = directxBase->LoadTexture(modelData.material.textureFilePath);
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = directxBase->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点当たりのサイズ
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点データをリソースにコピー



	// RootSinature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRange[0].NumDescriptors = 1;    // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;  // バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	


	// RootParameter作成。複数設定できるので配列。今回は結果1つだけの長さ1の配列
	//D3D12_ROOT_PARAMETER rootParameter[1] = {};
	
	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                     // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;// VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;                    // レジスタ番号0を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;            // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ



	//// マテリアル用のリソースを作る。 今回はcolor1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = directxBase->CreateBufferResource(directxBase->GetDevice(), sizeof(Material));
	////// Sprite用のマテリアルリソースを作る
	////Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = directxBase->CreateBufferResource(directxBase->GetDevice(), sizeof(Material));
	//// マテリアルにデータを書き込む
	//Material* materialData = nullptr;
	/*Material* materialDataSprite = nullptr;*/
	// 書き込むためのアドレスを取得
	//materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//// データを書き込む
	//materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 今回は赤を書き込んでみる
	//materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };

	///materialData->uvTransform = MakeIdentity4x4();
	//materialDataSprite->uvTransform = MakeIdentity4x4();

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};
	
	HRESULT hr;
	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = directxBase->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	// InputLayout 
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//// Shaderをコンパイルする
	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = directxBase->CompileShader(L"Resources/Shaders/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = directxBase->CompileShader(L"Resources/Shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;  // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer() , vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };    // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ、三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;
	hr = directxBase->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPilelineState));
	assert(SUCCEEDED(hr));

	// Sprite用の頂点リソースを作る
	/*Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = directxBase->CreateBufferResource(directxBase->GetDevice(), sizeof(VertexData) * 6);
	hr = directxBase->GetDevice()->CreateCommittedResource(
	    &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResourceSprite));
	assert(SUCCEEDED(hr));*/

	//// 頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//// リソースの先頭のアドレスから使う
	//vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//// 使用するリソースのサイズは頂点6つ分のサイズ
	//vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//// 1頂点あたりのサイズ
	//vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//VertexData* vertexDataSprite = nullptr;
	//vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// spriteの設定
	// 1枚目の三角形
	//vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	//vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	//vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	//vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	//vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	//vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	// 2枚目の三角形
	//vertexDataSprite[3].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	//vertexDataSprite[3].texcoord = { 0.0f, 0.0f };
	//vertexDataSprite[4].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	//vertexDataSprite[4].texcoord = { 1.0f, 0.0f };
	//vertexDataSprite[5].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	//vertexDataSprite[5].texcoord = { 1.0f, 1.0f };

	//// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = directxBase->CreateBufferResource(directxBase->GetDevice(), sizeof(Matrix4x4));
	//// データを書き込む
	//Matrix4x4* tranformationMatrixDataSprite = nullptr;
	//// 書き込むためのアドレスを取得
	//transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&tranformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	//*tranformationMatrixDataSprite = MakeIdentity4x4();
	// CPUで動かすようのTransformを作る
	Transform transformSprite{ {1000.0f, 1000.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f ,0.0f} };

	

	
	/*// 頂点リソースを作る
	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * 6);
	// 頂点バッファービューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6; // 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点当たりのサイズ

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	// 上
	vertexData[1] = { 0.0f, 0.5f, 0.0f, 1.0f };
	// 右下
	vertexData[2] = { 0.5f, -0.5f, 0.0f, 1.0f };


	// 左下
	vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	// 上
	vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f, 0.0f };
	// 右下
	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };

	// 左下2
	vertexData[3].position = { -0.5f, -0.5f, 0.5f, 1.0f };
	vertexData[3].texcoord = { 0.0f, 1.0f };
	// 上2
	vertexData[4].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 0.5f, 0.0f };
	// 右下2
	vertexData[5].position = { 0.5f, -0.5f, -0.5f, 1.0f };
	vertexData[5].texcoord = { 1.0f, 1.0f };*/

	//// Textureを読んで転送する
	//DirectX::ScratchImage mipImages = directxBase->LoadTexture("Resources/uvChecker.png");
	//const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = directxBase->CreateTextureResource(metadata);
	//directxBase->UploadTextureData(textureResource, mipImages);


	//

	//// ShaderResourceVieを作る
	//// metaDataを基にSRVの設定
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	//srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//// SRVを作成するDescriptorHeapの場所を決める
	//D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = directxBase->GetSRVCPUDescriptorHandle(1);
	////D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = directxBase->GetSRVGPUDescriptorHandle(1);
	////D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//// 先頭はImGuiが使っているのでその次を使う
	//// SRVの作成
	//directxBase->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);


	///TransformationMatrix用のResourceを作る
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = directxBase->CreateBufferResource(sizeof(Matrix4x4));
	// データを書き込む
	Matrix4x4* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	*wvpData = MakeIdentity4x4();


	// Transform変数を作る
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f,-5.0f} };

	Matrix4x4 projectionMatrix = MakePrespectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = directxBase->CreateBufferResource(directxBase->GetDevice(), sizeof(uint32_t) * 6);

	//D3D12_INDEX_BUFFER_VIEW indexbufferViewSprite{};
	//// リソースの先頭のアドレスから使う
	//indexbufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//// 使用するリソースのサイズはインデックス6つ分のサイズ
	//indexbufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//// インデックスはuint32_tとする
	//indexbufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//// インデックスリソースにデータを書き込む
	//uint32_t* indexDataSprite = nullptr;
	//indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	//indexDataSprite[0] = 0;  indexDataSprite[1] = 1;  indexDataSprite[2] = 2;
	//indexDataSprite[3] = 1;  indexDataSprite[4] = 3;  indexDataSprite[5] = 2;
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector2 anchorPoint = {0.5f, 0.5f};
	bool isFlipX = false;
	bool isFlipY = false;
	Vector2 textureLeftTop = {0.0f, 0.0f};
	Vector2 textureSize = sprite_->GetTextureSize();
	//ゲームループ
	//ウィンドウの×ボタンが押されるまでループ
	while (true){
		//Windowにメッセージが来てたら最優先で処理させる
		if (winApp->ProcessMessage()){
			// ゲームループを抜ける
			break;
		}
		else
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			Vector2 position = sprite_->GetPosition();
			float rotation = transformSprite.rotate.z;
			Vector2 scale = {transformSprite.scale.x, transformSprite.scale.y};


			// Imguiの設定(Color&STR)
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.7f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.7f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(300, 400));
			ImGui::Begin("colorConfig");

			/*if (ImGui::TreeNode("ColorCode")){
				ImGui::DragFloat3("RGB", &materialData->color.x, 0.01f);
				ImGui::ColorEdit3("RGB", &materialData->color.x);
				ImGui::ColorEdit3("RGBSprite", &materialDataSprite->color.x);
				ImGui::SliderFloat("R", &materialData->color.x, 0.0f, 1.0f);
				ImGui::SliderFloat("G", &materialData->color.y, 0.0f, 1.0f);
				ImGui::SliderFloat("B", &materialData->color.z, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("colorPreset")){
				if (ImGui::Button("RED")) {
					materialData->color.x = 1.0f;
					materialData->color.y = 0.0f;
					materialData->color.z = 0.0f;
				}
				if (ImGui::Button("GREEN")) {
					materialData->color.x = 0.0f;
					materialData->color.y = 1.0f;
					materialData->color.z = 0.0f;
				}
				if (ImGui::Button("BLUE")) {
					materialData->color.x = 0.0f;
					materialData->color.y = 0.0f;
					materialData->color.z = 1.0f;
				}
				if (ImGui::Button("YELLOW")) {
					materialData->color.x = 1.0f;
					materialData->color.y = 1.0f;
					materialData->color.z = 0.0f;
				}
				if (ImGui::Button("WHITE")) {
					materialData->color.x = 1.0f;
					materialData->color.y = 1.0f;
					materialData->color.z = 1.0f;
				}
				if (ImGui::Button("BLACK")) {
					materialData->color.x = 0.0f;
					materialData->color.y = 0.0f;
					materialData->color.z = 0.0f;
				}
				ImGui::TreePop();
			}*/
			if (ImGui::TreeNode("STR"))
			{
				ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("SpriteSTR"))
			{
				ImGui::ColorEdit4("Color", &color.x);
				ImGui::DragFloat3("Scale", &transformSprite.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate", &transformSprite.rotate.x, 0.01f);
				ImGui::DragFloat2("Translate", &position.x, 1.0f);
				ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
				ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
				ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
				ImGui::DragFloat2("anchorPoint", &anchorPoint.x, 0.1f);
				ImGui::DragFloat2("TextureLeftTop", &textureLeftTop.x, 1.0f);
				ImGui::DragFloat2("TextureSize", &textureSize.x, 1.0f);
				ImGui::Checkbox("flipX", &isFlipX);
				ImGui::Checkbox("flipY", &isFlipY);
				ImGui::TreePop();
			}



			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(1080, 0));
			ImGui::SetNextWindowSize(ImVec2(200, 300));
			ImGui::Begin("Camera");

			ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("Translate", &cameraTransform.translate.x, 0.01f);

			ImGui::End();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			if (input->TriggerKey(DIK_0)) {
				sprite_->ChangeTexture("Resources/monsterBall.png");
				textureSize = sprite_->GetTextureSize();
			}

			//for (Sprite* sprite : sprites) {
			//	//sprite->SetTextureLeftTop(textureLeftTop);
			//	//sprite->SetTextureSize(textureSize);
			//	sprite->SetFlipX(isFlipX);
			//	sprite->SetFlipX(isFlipY);
			//	sprite->SetAnchorPoint(anchorPoint);
			//	sprite->SetRotation(rotation);
			//	sprite->SetScale(scale);
			//	sprite->SetColor(color);
			//	sprite->Update();
			//}
			sprite_->SetTextureLeftTop(textureLeftTop);
			sprite_->SetTextureSize(textureSize);
			sprite_->SetFlipX(isFlipX);
			sprite_->SetFlipX(isFlipY);
			sprite_->SetAnchorPoint(anchorPoint);
			sprite_->SetPosition(position);
			sprite_->SetRotation(rotation);
			sprite_->SetScale(scale);
			sprite_->SetColor(color);
			sprite_->Update();

			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に書き換える
			//ImGui::ShowDemoWindow();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			//// これから書き込むバックバッファのインデックスを取得
			//UINT backBufferIndex = directxBase->GetSwapChain()->GetCurrentBackBufferIndex();

			// 描画前処理。DirextXの描画準備。全ての描画に共通のグラフィックスコマンドを積む
			directxBase->PreDraw();

			
			// Spriteの描画準備、Spriteの描画に共通のグラフィックスコマンドを積む
			spriteBase->sharedDrawSet();
			
			// 入力の更新
			input->Update();

			/*Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;*/

			//ゲームの処理
			// Sprite用のWorldViewProjectionMatrixを作る
			// SpriteのTransform処理
			/*Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));*/
			//*tranformationMatrixDataSprite = worldViewProjectionMatrixSprite;

			
			// 3DのTransform処理
			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePrespectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			*wvpData = worldViewProjectionMatrix;


			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			//directxBase->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
			//directxBase->GetCommandList()->SetPipelineState(graphicsPilelineState.Get()); // PSOを設定
			//directxBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);   // VBVを設定
			// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
			//directxBase->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//// マテリアルCBufferの場所を設定
			//directxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
			//directxBase->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

			//directxBase->GetCommandList()->IASetIndexBuffer(&indexbufferViewSprite); // IBVを設定
			// 描画！(DrawCall/ドローコール) 6個のインデックスを使用し一つのインスタンスを描画。その他は当面0で良い
			//for (Sprite* sprite : sprites) {
			//	sprite->Draw(/*textureSrvHandleGPU*/);
			//}
			sprite_->Draw();
			//// wvp用のCBufferの場所を設定
			//directxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			// ModelData描画
			//directxBase->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
			// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			//directxBase->GetCommandList()->DrawInstanced(6, 1, 0, 0);

			//Spriteの描画。変更が必要なものだけ変更する
			//directxBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
			// マテリアルCBufferの場所を設定
			//directxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			// TransformationMatrixCBbufferの場所を設定
			//directxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			// 描画
			//directxBase->GetCommandList()->DrawInstanced(6, 1, 0, 0);

			 
			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directxBase->GetCommandList().Get());
			
			//assert(SUCCEEDED(hr));
			directxBase->PostDraw();
		}
	}

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	//ImGui_ImplDX12_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();
	
#ifdef DEBUG
	debugController->Release();
#endif // DEBUG
	   // directXBaseの終了処理
	// directXBaseの終了処理
	directxBase->Finalize();
	delete directxBase;
	// WindowAPIの終了処理
	winApp->Finalize();
	// WindowAPI解放
	delete winApp;
	// スプライト解放
	delete spriteBase;
	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();
	for (Sprite* sprite : sprites) {
		delete sprite;
	}


	// 入力解放
	delete input;

	return 0;
}