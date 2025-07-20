#include "world.h"
#include "SDL3/SDL_log.h"
#include <SDL3_image/SDL_image.h>

// Simple tile grid (0 = empty, 1 = grass, etc.)
static int tiles[WORLD_HEIGHT][WORLD_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

static SDL_Texture* tileTexture = NULL;  // Placeholder

void World_Init(SDL_Renderer* renderer) {
    tileTexture = IMG_LoadTexture(renderer, "assets/grass.png");  // Load actual texture

    if (!tileTexture){
        SDL_Log("DEBUG: Successfully loaded surface for world - %s", "assets/grass.png");
    }
}

void World_Render(SDL_Renderer* renderer, SDL_FPoint* camera) {
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            if (tiles[y][x] == 1) {
                SDL_FRect rect = {(float)(x * TILE_SIZE) - camera->x, (float)(y * TILE_SIZE) - camera->y, TILE_SIZE, TILE_SIZE};
                if (tileTexture) {
                    SDL_RenderTexture(renderer, tileTexture, NULL, &rect);
                } else {
                    // Placeholder: Green rect if no texture
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
}
