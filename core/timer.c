#include "timer.h"
#include "SDL3/SDL_timer.h"

void Timer_Init(Timer *timer){
    timer -> frequency = SDL_GetPerformanceFrequency();
    timer -> previous = SDL_GetPerformanceCounter();
    timer -> deltaTime = 0.0f;
}

void Timer_Update(Timer *timer){
    Uint64 current = SDL_GetPerformanceCounter();
    timer -> deltaTime = (float)(( current - timer -> previous) / (double)timer->frequency);
    timer -> previous = current;
}
