#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
// Not sure how to handle dynamic pathing? not needed tho
#include "../core/timer.h"

#define MAX_ENTITIES 100
#define MAX_PARTICLES 500

typedef enum {
    ENTITY_WIZARD,
    ENTITY_ENEMY,
    ENTITY_PROJECTILE,
    ENTITY_LOOT
} EntityType;

/*
 * Position is x, y, w, h
 * For movement/projectiles we do new pos = current position +/- velocity
 * All entities have a lifecycle
 */
typedef struct {
    EntityType type;
    SDL_Texture* texture;
    SDL_FRect position;
    SDL_FPoint velocity;
    float health;
    float lifetime;
} Entity;

typedef struct {
    Timer timer;
    Entity entities[MAX_ENTITIES];
    int entityCount;
    int score;

    // Particle array def
    struct Particle {
        SDL_FPoint position;
        SDL_FPoint velocity;
        float lifetime;
        SDL_Color color;
    } particles[MAX_PARTICLES];

    int particleCount;

    // Camera
    SDL_FPoint camera;
} GameState;

void Game_Init(GameState* state, SDL_Renderer* renderer);
void Game_Update(GameState* state, float dt);
void Game_Render(GameState* state, SDL_Renderer* renderer, float fps, int score);
void Game_Shutdown(GameState* state);

void EmitParticles(GameState* state, int count, SDL_FPoint origin, SDL_Color color);

#endif
