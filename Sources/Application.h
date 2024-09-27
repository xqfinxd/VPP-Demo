#pragma once

#include "IApplication.h"
#include <SDL_video.h>

class Application :
    public IApplication {
public:
    Application();
    ~Application();
    
protected:
    void Initialize() override;
    void FrameRun() override;
    void Finalize() override;

private:
    SDL_Window* m_Window;
};

