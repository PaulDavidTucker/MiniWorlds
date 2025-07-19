#ifndef WORLD_H
#define WORLD_H

#include <SDL3/SDL.h>

#define WORLD_WIDTH 10
#define WORLD_HEIGHT 10
#define TILE_SIZE 64

void World_Init(SDL_Renderer* renderer);
void World_Render(SDL_Renderer* renderer, SDL_FPoint* camera);

#endif
