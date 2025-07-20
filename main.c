#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

#include "game/game.h"

#define DEFAULT_FPS_CAP 60.0f

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Define long options
// {name, has_arg, flag, val}
static struct option long_options[] = {
    {"fps", required_argument, 0, 'f'},
    {0, 0, 0, 0}                         // Sentinel to mark the end
};

typedef struct AppState {
    Uint64 lastCount;
    Uint64 frameStartCounter;
    Uint64 freq;
    int framecount;
    float fps;
    float userGivenCap;
    GameState gameState;
} AppState;

SDL_AppResult CreateMainWindowAndRender(void **appstate, float cap){
    // Create a state object which we can access. Allocates stack space the size of the app state enum!
    AppState *state = SDL_calloc(1, sizeof(AppState));

    state -> lastCount = SDL_GetPerformanceCounter();
    state -> freq = SDL_GetPerformanceFrequency();  // ticks per second
    state -> frameStartCounter = state -> lastCount;
    state -> userGivenCap = cap;

    if (!state){
        // write logs here for failed app state
        SDL_Log("App state failed to init!");
        return SDL_APP_FAILURE;
    }

    *appstate = state; // Make available when we pass this into loops later.

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("MiniWorlds", 800, 600, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    float cap = DEFAULT_FPS_CAP;

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    int opt;

    while ((opt = getopt_long(argc, argv, "f:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f': {
                // optarg contains the argument for the option
                char *endptr;
                errno = 0;

                float parsed_cap = strtof(optarg, &endptr);
                if (*endptr != '\0') {
                    fprintf(stderr, "Error: Invalid value for --fps/-f: %s\nFalling back to default %.2f FPS\n", optarg, DEFAULT_FPS_CAP);
                    cap = DEFAULT_FPS_CAP;
                    break;
                }

                if (errno == ERANGE) {
                    fprintf(stderr, "Error: --fps/-f value %s is out of float range.\nFalling back to default %.2f FPS\n", optarg, DEFAULT_FPS_CAP);
                    return SDL_APP_FAILURE;
                }

                if (parsed_cap <= 0.0f || parsed_cap > 1000.0f) {
                    fprintf(stderr, "Warning: --fps/-f value %.2f is outside typical range (0-1000). Using default %f FPS.\n", parsed_cap, DEFAULT_FPS_CAP);
                    cap = DEFAULT_FPS_CAP;
                } else {
                    cap = parsed_cap;
                }
                cap = parsed_cap; // Assign the parsed value to 'cap'
                break;
            }
            case '?': {
                // getopt_long prints an error message for unknown options
                SDL_Log("Unknown option %d", opt);
                return SDL_APP_FAILURE;
            }
            default: {
                // Should not happen for standard getopt_long usage
                return SDL_APP_FAILURE;
            }
        }
    }

    SDL_Log("FPS set to: %.2f", cap);

    for (int i = optind; i < argc; i++) {
        printf("Non-option argument: %s\n", argv[i]);
    }

    SDL_AppResult init_Result = CreateMainWindowAndRender(appstate, cap);

    // Now initialize game on the REAL appstate (set by CreateMainWindowAndRender)
    AppState *realState = (AppState *)*appstate;  // Cast the void* to access
    Game_Init(&realState->gameState, renderer);

    return init_Result;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event-> type ==SDL_EVENT_KEY_DOWN){
        //if ( key pressed is == SDLK_Q)
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    //Cast given appstate to appstate enum type
    AppState *state = (AppState *)appstate;
    const double TARGET_FRAME_TIME = 1.0 / state ->userGivenCap;

    Uint64 current = SDL_GetPerformanceCounter();
    double elapsed = (double)(current - state->frameStartCounter) / state->freq;

    // Only continue if it's time for the next frame
    if (elapsed < TARGET_FRAME_TIME) {
        // Sleep for the remaining time in ms
        Uint32 delayTimeMs = (Uint32)((TARGET_FRAME_TIME - elapsed) * 1000.0);

        // use delay here to sleep execution
        SDL_Delay(delayTimeMs);
        return SDL_APP_CONTINUE;
    }

    state->frameStartCounter = SDL_GetPerformanceCounter();

    // increment frame count for this iteration
    state->framecount++;

    double timeSinceLastFpsUpdate = (double)(current - state->lastCount) / state->freq;

    if (timeSinceLastFpsUpdate >= 1.0) {
        state->fps = (float)(state->framecount / timeSinceLastFpsUpdate);
        state->framecount = 0;
        state->lastCount = current;
    }

    float dt = (float)elapsed;

    Game_Update(&state->gameState, dt);
    Game_Render(&state->gameState, renderer, state ->fps, state-> gameState.score);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    AppState *state = (AppState *)appstate;
    Game_Shutdown(&state->gameState);
    SDL_free(state);
    // Freed memory
}
