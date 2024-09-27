#pragma once

#include <string>

class IApplication {
public:
    enum class DisplayMode {
        eWindowed,              // ���ڻ�
        eFullscreen,            // ȫ��
        eFullscreenWindowed,    // �ޱ߿�ȫ��
    };

public:
    static IApplication* Create();

    IApplication();
    virtual ~IApplication();

    void Run();
    void Exit();

protected:
    virtual void Initialize() = 0;
    virtual void FrameRun() = 0;
    virtual void Finalize() = 0;

protected:
    DisplayMode m_DisplayMode;
    int         m_Width;
    int         m_Height;
    std::string m_Title;
    uint32_t    m_LockFps;

private:
    bool        m_Running;
};

