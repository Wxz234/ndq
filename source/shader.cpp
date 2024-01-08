module;

#include "predef.h"

export module ndq:shader;

import :platform;

namespace ndq
{
	export enum class ShaderType
	{
		Vertex,
		Pixel,
	};

	export enum class ShaderModel
	{
		SM_6_0,
		SM_6_1,
		SM_6_2,
		SM_6_3,
		SM_6_4,
		SM_6_5,
		SM_6_6,
	};

	class ShaderDesc
	{
	public:

		ShaderDesc& SetShaderType(ShaderType type)
		{
			Type = type;
			return *this;
		}

		ShaderDesc& SetShaderModel(ShaderModel model)
		{
			Model = model;
			return *this;
		}

		ShaderDesc& SetEntryPoint(const wchar_t* entryPoint)
		{
			EntryPointName = entryPoint;
			return *this;
		}

		ShaderDesc& AddDefinition(const wchar_t* definition)
		{
			Definitions.emplace_back(definition);
			return *this;
		}
	private:
		ShaderType Type = ShaderType::Vertex;
		ShaderModel Model = ShaderModel::SM_6_6;
		std::vector<std::wstring> Definitions;
		std::wstring EntryPointName;

		friend class InternalArguments;
	};

	class Shader
	{
	public:

	private:
		
		Shader() {}
		friend Shader* CompileShader(const wchar_t** pArguments, uint32 argCount);
	};
	namespace Internal
	{
		class InternalArguments
		{
		public:
			InternalArguments(const wchar_t* path, const ShaderDesc* pDesc)
			{

			}
			~InternalArguments()
			{

			}

			const wchar_t** GetArgs()
			{
				return nullptr;
			}
		private:

		};

		std::unique_ptr<InternalArguments> CreateInternalArguments(const wchar_t* path, const ShaderDesc* pDesc)
		{
			return std::unique_ptr<InternalArguments>(new InternalArguments(path, pDesc));
		}
	}

	export Shader* CompileShader(const wchar_t* path, const wchar_t** pArguments, uint32 argCount)
	{
		if (path == nullptr || pArguments == nullptr || argCount == 0)
		{
			return nullptr;
		}

		Microsoft::WRL::ComPtr<IDxcUtils> Utils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> Compiler;
		Microsoft::WRL::ComPtr<IDxcLibrary> Library;
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> Blob;
		Microsoft::WRL::ComPtr<IDxcResult> Result;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> IncludeHandler;

		BOOL Known{};
		UINT Encoding{};

		DxcBuffer Buffer{};

		auto Dll = GetDll(DllType::DXCOMPILER);
		auto _DxcCreateInstance = (DxcCreateInstanceProc)GetDllExport(Dll, "DxcCreateInstance");

		_DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils));
		_DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
		_DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));

		Library->CreateBlobFromFile(path, nullptr, &Blob);
		Utils->CreateDefaultIncludeHandler(&IncludeHandler);
		Blob->GetEncoding(&Known, &Encoding);

		Buffer.Ptr = Blob->GetBufferPointer();
		Buffer.Size = Blob->GetBufferSize();
		Buffer.Encoding = Encoding;

		auto r = Compiler->Compile(&Buffer, pArguments, argCount, IncludeHandler.Get(), IID_PPV_ARGS(&Result));
		Microsoft::WRL::ComPtr<IDxcBlobUtf8> Errors;
		Result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&Errors), nullptr);
		if (Errors && Errors->GetStringLength() > 0)
		{
			//MyLogFunction(Error, (char*)pErrors->GetBufferPointer());
			OutputDebugStringA((char*)Errors->GetBufferPointer());
		}

		Microsoft::WRL::ComPtr<IDxcBlob> oo;
		Result->GetResult(&oo);
		//if (oo.Get())
		//{
		//	char* ppp = (char*)oo->GetBufferPointer();
		//	ppp += 2000;
		//	auto ddd = oo->GetBufferSize();
		//	int z = 1;
		//}

		return nullptr;
	}

	export void RemoveShader(Shader* pShader)
	{
		delete pShader;
	}
}