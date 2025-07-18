#include "player.h"

void Player_Update(Entity* player, float dt, GameState* state) {
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);
    float speed = 200.0f * dt;

    if (keyboard[SDL_SCANCODE_W]) player->position.y -= speed;
    if (keyboard[SDL_SCANCODE_S]) player->position.y += speed;
    if (keyboard[SDL_SCANCODE_A]) player->position.x -= speed;
    if (keyboard[SDL_SCANCODE_D]) player->position.x += speed;

    // Camera follows player (centered; assumes window size 800x600)
    state->camera.x = player->position.x - 400.0f;
    state->camera.y = player->position.y - 300.0f;

    // Example spell: Press space to cast (emit particles for now; add projectile later)
    if (keyboard[SDL_SCANCODE_SPACE]) {
        EmitParticles(state, 50, (SDL_FPoint){player->position.x, player->position.y}, (SDL_Color){255, 0, 0, 255});
    }
}
