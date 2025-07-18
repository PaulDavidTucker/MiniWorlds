#include "world.h"

// Simple tile grid (0 = empty, 1 = grass, etc.)
static int tiles[WORLD_HEIGHT][WORLD_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    // ... fill the rest as needed
};

static SDL_Texture* tileTexture = NULL;  // Placeholder

void World_Init(void) {
    // Load tile texture (in real init, pass renderer)
    // tileTexture = LoadTexture(renderer, "assets/grass.png");  // Call from Game_Init instead
}

void World_Render(SDL_Renderer* renderer, SDL_FPoint* camera) {
    for (int y = 0; y < WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            if (tiles[y][x] == 1) {
                SDL_FRect rect = {(float)(x * TILE_SIZE) - camera->x, (float)(y * TILE_SIZE) - camera->y, TILE_SIZE, TILE_SIZE};
                SDL_RenderTexture(renderer, tileTexture, NULL, &rect);
            }
        }
    }
}
