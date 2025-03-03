#include "ndq/ApplicationWindow.h"

using namespace ndq;

struct MainWindow : ApplicationWindow
{
    MainWindow() : ApplicationWindow(800, 600, L"Window") {}
    void initialize() {}
    void update(float) {}
    void finalize() {}
};

WIN_MAIN_MACRO(MainWindow)