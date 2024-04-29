#include "Application.h"

Application::Application(const char* title, int initWidth, int initHeight)
    : m_Title(title), m_Width(initWidth), m_Height(initHeight)
{
}

Application::Application(const char* title)
    : m_Title(title), m_Fullscreen(true)
{
}

Application::~Application()
{
}

