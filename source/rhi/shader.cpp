#include "ndq/rhi/shader.h"

#include <Windows.h>

#include <combaseapi.h>
#include <dxcapi.h>
#include <wrl/client.h>

namespace ndq
{
    IBlob* _CreateBlob(IDxcBlob* pRawBlob);

    TRefCountPtr<IBlob> LoadShaderFromPath(const wchar_t* path, const wchar_t** pArguments, unsigned argCount)
    {
        Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));

        Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

        Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
        pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource;
        pUtils->LoadFile(path, nullptr, &pSource);
        DxcBuffer Source;
        Source.Ptr = pSource->GetBufferPointer();
        Source.Size = pSource->GetBufferSize();
        Source.Encoding = DXC_CP_ACP;

        Microsoft::WRL::ComPtr<IDxcResult> pResult;
        pCompiler->Compile(
            &Source,
            pArguments,
            argCount,
            pIncludeHandler.Get(),
            IID_PPV_ARGS(&pResult)
        );

        IDxcBlob* pBlob;
        Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
        pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pBlob), &pShaderName);
        auto pTempPtr = _CreateBlob(pBlob);
        TRefCountPtr<IBlob> pret = pTempPtr;

        pBlob->Release();
        pTempPtr->Release();

        return pret;
    }
}