#include "game.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "player.h"
#include "world.h"
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>  // For rand()

static SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        SDL_Log("Failed to load image: %s: %s", filePath, SDL_GetError());
        return NULL;
    }

    SDL_Log("DEBUG: Successfully loaded surface for %s", filePath);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!texture) {
        SDL_Log("ERROR: Failed to create texture from surface %s: %s", filePath, SDL_GetError());
        return NULL;
    }

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
            SDL_Log("WARNING: Failed to set blend mode for %s: %s", filePath, SDL_GetError());
    }

    SDL_Log("DEBUG: Successfully created texture for %s", filePath); // Log success

    return texture;
}

void LogEntities(const GameState* state) {
    SDL_Log("Current active entity count: %d", state->entityCount);

    // Correct way to iterate through active entities
    for (int i = 0; i < state->entityCount; i++) {
        const Entity* e = &state->entities[i]; // Get a pointer to the current entity for easier access

        SDL_Log("--- Entity %d ---", i);
        SDL_Log("  Type: %d (e.g., WIZARD=%d, ENEMY=%d, PROJECTILE=%d, LOOT=%d)",
                e->type, ENTITY_WIZARD, ENTITY_ENEMY, ENTITY_PROJECTILE, ENTITY_LOOT); // Log type enum value
        SDL_Log("  Position: (%.2f, %.2f, %.2f, %.2f)", e->position.x, e->position.y, e->position.w, e->position.h);
        SDL_Log("  Velocity: (%.2f, %.2f)", e->velocity.x, e->velocity.y);
        SDL_Log("  Health: %.2f", e->health);
        SDL_Log("  Lifetime: %.2f", e->lifetime);

        // Check if texture is loaded (pointer is not NULL)
        if (e->texture) {
            SDL_Log("  Texture: Loaded (pointer: %p)", (void*)e->texture); // Cast to void* for %p
            // You could get more info from the texture if needed, e.g., its width/height
            // int tex_w, tex_h;
            // SDL_QueryTexture(e->texture, NULL, NULL, &tex_w, &tex_h);
            // SDL_Log("  Texture dimensions: %dx%d", tex_w, tex_h);
        } else {
            SDL_Log("  Texture: NULL (not loaded or intentionally null)");
        }
    }
    SDL_Log("--------------------");
}

void Game_Init(GameState* state, SDL_Renderer* renderer) {
    Timer_Init(&state->timer);
    state->entityCount = 0;
    state->particleCount = 0;
    state->camera.x = 0.0f;
    state->camera.y = 0.0f;
    state->score = 0;

    // Initialize world (tiles, etc.)
    World_Init(renderer);

    // Add wizard
    Entity wizard = {0};
    wizard.type = ENTITY_WIZARD;
    wizard.texture = LoadTexture(renderer, "assets/wizard.png");

    if (!wizard.texture) {
        SDL_Log("FATAL: Wizard texture failed to load!");
        // Consider returning an error from Game_Init or exiting here
    }

    wizard.position = (SDL_FRect){100.0f, 100.0f, 64.0f, 64.0f};
    wizard.health = 100.0f;
    state->entities[state->entityCount++] = wizard;

    // Add an example enemy
    Entity enemy = {0};
    enemy.type = ENTITY_ENEMY;
    enemy.texture = LoadTexture(renderer, "assets/enemy.png");

    if (!enemy.texture) {
        SDL_Log("FATAL: Enemy texture failed to load!");
    }
    enemy.position = (SDL_FRect){300.0f, 200.0f, 64.0f, 64.0f};
    enemy.health = 50.0f;
    state->entities[state->entityCount++] = enemy;

    SDL_Log("Init Entity count and list: %d", state -> entityCount);

    LogEntities(state);
}

void Game_Update(GameState* state, float dt) {
    Timer_Update(&state->timer);
    // Update player (player index is always 0)
    Player_Update(&state->entities[0], dt, state);

    SDL_Log("DEBUG: Pre-update entityCount: %d", state->entityCount);  // Trace count
    for (int k = 0; k < state->entityCount; ++k) {
        SDL_Log("DEBUG: Pre-update entity %d type: %d", k, state->entities[k].type);  // List all
    }

    // Update entities
    for (int i = 1; i < state->entityCount;) {
        Entity* e = &state->entities[i];
        if (e->type == ENTITY_ENEMY) {
            // Simple AI: move toward wizard
            SDL_FPoint dir = {state->entities[0].position.x - e->position.x, state->entities[0].position.y - e->position.y};
            float dist = SDL_sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (dist > 0) {
                dir.x /= dist;
                dir.y /= dist;
            }
            e->position.x += dir.x * 100.0f * dt;
            e->position.y += dir.y * 100.0f * dt;

            // Check collision with wizard (damage)
            if (SDL_HasRectIntersectionFloat(&state->entities[0].position, &e->position)) {
                state->entities[0].health -= 10.0f * dt;  // Damage over time
                if (state->entities[0].health <= 0) {
                    SDL_Log("Game Over! Wizard defeated.");
                    // TODO: Quit or restart
                }
            }
        } else if (e->type == ENTITY_PROJECTILE) {
            // Move projectile
            e->position.x += e->velocity.x * dt;
            e->position.y += e->velocity.y * dt;
            e->lifetime -= dt;
            if (e->lifetime <= 0) {
                // Remove projectile
                state->entities[i] = state->entities[--state->entityCount];
                continue;
            }
            // Check collision with enemies
            for (int j = 1; j < state->entityCount; ++j) {
                if (state->entities[j].type == ENTITY_ENEMY && SDL_HasRectIntersectionFloat(&e->position, &state->entities[j].position)) {
                    state->entities[j].health -= 20.0f;  // Damage enemy
                    if (state->entities[j].health <= 0) {
                        // Spawn loot and particles on death
                        Entity loot = {0};
                        loot.type = ENTITY_LOOT;
                        loot.texture = NULL;  // Placeholder (yellow rect)
                        loot.position = state->entities[j].position;
                        loot.health = 1.0f;  // Collectible
                        state->entities[state->entityCount++] = loot;
                        EmitParticles(state, 50, (SDL_FPoint){e->position.x, e->position.y}, (SDL_Color){255, 255, 0, 255});  // Yellow explosion
                        // Remove enemy
                        state->entities[j] = state->entities[--state->entityCount];
                    }
                    // Remove projectile after hit
                    state->entities[i] = state->entities[--state->entityCount];
                    continue;
                }
            }
        } else if (e->type == ENTITY_LOOT) {
            // Check collection by wizard
            if (SDL_HasRectIntersectionFloat(&state->entities[0].position, &e->position)) {
                state->score += 10;  // Increase score
                // Remove loot
                state->entities[i] = state->entities[--state->entityCount];
                continue;
            }
        }
        ++i;  // Only increment if no removal

        SDL_Log("DEBUG: Post-update entityCount: %d", state->entityCount);  // Trace after
        for (int k = 0; k < state->entityCount; ++k) {
            SDL_Log("DEBUG: Post-update entity %d type: %d", k, state->entities[k].type);  // List all
        }
    }

    // Update particles
    for (int i = 0; i < state->particleCount; ) {
            struct Particle* p = &state->particles[i];
            p->position.x += p->velocity.x * dt;
            p->position.y += p->velocity.y * dt;
            p->lifetime -= dt;
            if (p->lifetime <= 0) {
                state->particles[i] = state->particles[--state->particleCount];
            } else {
                ++i;
            }
        }
}

void Game_Render(GameState* state, SDL_Renderer* renderer, float fps, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Log("DEBUG: Rendering with entityCount: %d", state->entityCount);  // Trace

        // Render world (tiles)
        World_Render(renderer, &state->camera);

        // Render entities with camera offset (fallback to colored rects if no texture)
        for (int i = 0; i < state->entityCount; ++i) {
            Entity* e = &state->entities[i];
            SDL_FRect renderPos = {e->position.x - state->camera.x, e->position.y - state->camera.y, e->position.w, e->position.h};

            SDL_Log("DEBUG: Rendering entity type %d at pos (%.2f, %.2f) size (%.2f, %.2f)",
                    e->type, renderPos.x, renderPos.y, renderPos.w, renderPos.h);  // Enhanced log for tracing

            if (e->texture) {
                SDL_Log("DEBUG: Using texture for entity type %d", e->type);
                if (SDL_RenderTexture(renderer, e->texture, NULL, &renderPos) != 0) {
                    SDL_Log("ERROR: SDL_RenderTexture failed for type %d: %s", e->type, SDL_GetError());
                }
            } else {
                SDL_Log("WARNING: No texture for entity type %d, using fallback color", e->type);
                // Placeholder colors
                if (e->type == ENTITY_WIZARD) SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue
                else if (e->type == ENTITY_ENEMY) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red
                else if (e->type == ENTITY_PROJECTILE) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green
                else if (e->type == ENTITY_LOOT) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow
                else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White for unknown
                SDL_RenderFillRect(renderer, &renderPos);
            }
        }

        // Render particles
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int i = 0; i < state->particleCount; ++i) {
            struct Particle* p = &state->particles[i];
            SDL_SetRenderDrawColor(renderer, p->color.r, p->color.g, p->color.b, (Uint8)(255 * (p->lifetime / 2.0f)));  // Fade
            SDL_FRect rect = {p->position.x - state->camera.x - 2, p->position.y - state->camera.y - 2, 4, 4};
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // Simple UI: FPS and Score (top-left)
        char fpsText[64];
        SDL_snprintf(fpsText, sizeof(fpsText), "FPS: %.2f Score: %d", fps, score);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 10.0f, 10.0f, fpsText);

        SDL_RenderPresent(renderer);
}

void Game_Shutdown(GameState* state) {
    for (int i = 0; i < state->entityCount; ++i) {
        SDL_DestroyTexture(state->entities[i].texture);
    }
    // TODO: Free world resources
}

void EmitParticles(GameState* state, int count, SDL_FPoint origin, SDL_Color color) {
    for (int i = 0; i < count && state->particleCount < MAX_PARTICLES; ++i) {
        struct Particle p = {0};
        p.position = origin;
        p.velocity.x = (float)(rand() % 10 - 5);
        p.velocity.y = (float)(rand() % 10 - 5);
        p.lifetime = 1.0f + (float)(rand() % 100) / 100.0f;
        p.color = color;
        state->particles[state->particleCount++] = p;
    }
}
