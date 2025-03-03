#include "ndq/ApplicationWindow.h"

struct MainWindow : ndq::ApplicationWindow
{
    MainWindow() : ndq::ApplicationWindow(800, 600, L"Window") {}
    void initialize() {}
    void update(float) {}
    void finalize() {}
};

WIN_MAIN_MACRO(MainWindow)