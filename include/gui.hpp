#pragma once

#ifndef __IBM_GUI_HPP__
#define __IBM_GUI_HPP__

#include "std.hpp"

#include "imgui.h"

#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class IApplication
{
public:
    virtual bool init() = 0;
    virtual void run() = 0;
    virtual void draw() = 0;
    virtual void dispose() = 0;
};

class Application : public IApplication
{
private:
    const char *glslVersion;
    bool inited;

public:
    GLFWwindow *window;
    const char *windowName;
    ImVec4 clearColor;

public:
    Application(const char *windowName);

    virtual ~Application();

    virtual bool init();
    virtual void run();
    virtual void draw();
    virtual void dispose();

protected:
    bool initGLFW();
    bool initImGui();
};

#endif
