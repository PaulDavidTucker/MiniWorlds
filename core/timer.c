#include "timer.h"
#include "SDL3/SDL_timer.h"

Uint64 lastFrameTime = 0;
double targetFrameTime = 1000.0 / 60.0; // 60Hz

double getDeltaTime() {
    Uint64 current = SDL_GetTicks();
    double dt = (double)(current - lastFrameTime);
    lastFrameTime = current;
    return dt;
}

void delayToCapFPS(double elapsed) {
    double wait = targetFrameTime - elapsed;
    if (wait > 0) SDL_Delay((Uint32)wait);
}
