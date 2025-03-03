#pragma once

namespace ndq
{
    void setDeviceHwndAndSize(void* hwnd, unsigned width, unsigned height);
    void devicePresent();
    void deviceFinalize();
}