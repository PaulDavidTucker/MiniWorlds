#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

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
    if (!SDL_CreateWindowAndRenderer("MiniWorlds", 800, 600, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    float cap = DEFAULT_FPS_CAP;

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

    return CreateMainWindowAndRender(appstate, cap);;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
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
        SDL_Log("FPS: %.2f", state->fps);
        state->framecount = 0;
        state->lastCount = current;
    }

    const char *message = "Hello World!";

    char fpsText[64];
    SDL_snprintf(fpsText, sizeof(fpsText), "FPS: %.2f", state->fps);

    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    /* Center the message and scale it up */
    SDL_GetRenderOutputSize(renderer, &w, &h);
    SDL_SetRenderScale(renderer, scale, scale);
    x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    /* Draw the message */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, x, y, message);
    SDL_RenderDebugText(renderer, x, y-35, fpsText);
    SDL_RenderPresent(renderer);


    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    AppState *state = (AppState *)appstate;
    SDL_free(state);
    // Freed memory
}
