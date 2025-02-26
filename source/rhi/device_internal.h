#pragma once

namespace ndq
{
    void SetDeviceHwndAndSize(void* hwnd, unsigned width, unsigned height);
    void DevicePresent();
    void DeviceFinalize();
}