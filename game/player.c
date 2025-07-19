#include "player.h"
#include <stdlib.h>

void Player_Update(Entity* player, float dt, GameState* state) {
    const _Bool* keyboard = SDL_GetKeyboardState(NULL);
    float speed = 200.0f * dt;

    if (keyboard[SDL_SCANCODE_W]) player->position.y -= speed;
    if (keyboard[SDL_SCANCODE_S]) player->position.y += speed;
    if (keyboard[SDL_SCANCODE_A]) player->position.x -= speed;
    if (keyboard[SDL_SCANCODE_D]) player->position.x += speed;

    // Camera follows player (centered; assumes window size 800x600)
    state->camera.x = player->position.x - 400.0f;
    state->camera.y = player->position.y - 300.0f;

    // Spell casting: Press space to fire projectile (random direction for demo)
    static float cooldown = 0.0f;
    cooldown -= dt;
    if (keyboard[SDL_SCANCODE_SPACE] && cooldown <= 0 && state->entityCount < MAX_ENTITIES) {
        Entity projectile = {0};
        projectile.type = ENTITY_PROJECTILE;
        projectile.texture = NULL;  // Placeholder (green rect)
        projectile.position = (SDL_FRect){player->position.x, player->position.y, 16.0f, 16.0f};
        projectile.velocity = (SDL_FPoint){ (float)(rand() % 200 - 100), (float)(rand() % 200 - 100) };  // Random direction
        projectile.lifetime = 2.0f;  // 2 seconds
        state->entities[state->entityCount++] = projectile;
        EmitParticles(state, 50, (SDL_FPoint){player->position.x, player->position.y}, (SDL_Color){255, 0, 0, 255});
        cooldown = 0.5f;  // Cooldown
    }
}
