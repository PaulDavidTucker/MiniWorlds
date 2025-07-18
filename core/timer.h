#ifndef TIMER_H
#define TIMER_H

#include <SDL3/SDL.h>
#include <stdint.h>

#include <SDL3/SDL.h>

typedef struct Timer {
    Uint64 previous;
    Uint64 frequency;
    //Time passed since last render
    float deltaTime;
} Timer;

void Timer_Init(Timer *timer);
void Timer_Update(Timer *timer);

#endif
