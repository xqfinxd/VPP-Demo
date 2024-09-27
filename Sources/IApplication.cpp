#include "IApplication.h"

#include <chrono>
#include <thread>

IApplication::IApplication() :
    m_DisplayMode(DisplayMode::eWindowed),
    m_Width(1600),
    m_Height(900),
    m_Title(),
    m_LockFps(60),
    m_Running(false) {
}

IApplication::~IApplication() {
}

void IApplication::Run() {
    using MicroSec = std::chrono::duration<double, std::micro>;

    const auto kOneSec = MicroSec(1'000'000.0);
    const auto kMaxElapsed = m_LockFps ?
        (kOneSec / m_LockFps) : MicroSec::zero();

    try {
        Initialize();
        m_Running = true;
    }
    catch (const std::exception&) {
        m_Running = false;
    }

    while (m_Running) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        FrameRun();

        auto frameEnd = std::chrono::high_resolution_clock::now();

        auto frameElapsed = frameEnd - frameStart;
        if (frameElapsed < kMaxElapsed)
            std::this_thread::sleep_for(kMaxElapsed - frameElapsed);
    }

    Finalize();
}

void IApplication::Exit() {
    m_Running = false;
}
