#pragma once

#include <Windows.h>
#include <concurrent_vector.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <winnt.h> // LARGE_INTEGER
#include <wrl.h>

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);