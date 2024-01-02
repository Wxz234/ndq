module;

#include "predef.h"

export module ndq:shader;

import :platform;

namespace ndq
{
	enum class ShaderType
	{
		Vertex,
		Pixel,
	};

	enum class ShaderModel
	{
		SM_6_0,
		SM_6_1,
		SM_6_2,
		SM_6_3,
		SM_6_4,
		SM_6_5,
		SM_6_6,
	};

	class Shader
	{
	public:
	private:
		Shader() {}


	};

	export Shader* CompileShaderFromPath(const char* path, ShaderType type, ShaderModel model, const char* entry)
	{
		auto DxcompilerDll = GetDllHandleFromWindowsSDK("dxcompiler.dll");
		FreeDllHandle(DxcompilerDll);
		return nullptr;
	}

	export void RemoveShader(Shader* pShader)
	{
		delete pShader;
	}
}