#include "ndq/ApplicationWindow.h"
#include "ndq/Root.h"
#include "ndq/SceneManager.h"

using namespace ndq;

struct MainWindow : ApplicationWindow
{
    MainWindow() : ApplicationWindow(800, 600, L"Mesh"), mSceneManager(nullptr) {}

    void initialize()
    {
        mSceneManager = Root::getRoot()->createSceneManager();
    }

    void update(float)
    {

    }

    void finalize()
    {
        Root::getRoot()->destroySceneManager(mSceneManager);
    }

    SceneManager* mSceneManager;
};

NDQ_WIN_MAIN_MACRO(MainWindow)