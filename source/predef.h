#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <combaseapi.h>
#include <concurrent_vector.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <winrt/base.h>
#include <winrt/windows.foundation.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#define NDQ_DEFAULT_WIDTH 800
#define NDQ_DEFAULT_HEIGHT 600

#define NDQ_NODEMASK 1