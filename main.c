#include "config.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct binding_t {
  char *name;
  int x_pos, y_pos, w, h;
  bool is_active;
};

struct state_t {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *active_texture;
  SDL_Texture *inactive_texture;

  bool running;
  bool changed;

  struct binding_t *bindings;
  int bindings_count;
};

void cleanup_state(struct state_t *state) {
  if (state->window) {
    SDL_DestroyWindow(state->window);
    state->window = NULL;
  }
  if (state->renderer) {
    SDL_DestroyRenderer(state->renderer);
    state->renderer = NULL;
  }
  if (state->bindings) {
    free(state->bindings);
    state->bindings = NULL;
  }
  state->bindings_count = 0;
}

void poll_events(struct state_t *state) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      state->running = false;
      break;
    case SDL_KEYDOWN:
      break;
    default:
      break;
    }
  }
}

void draw(SDL_Renderer *renderer, struct state_t *state) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(state->renderer, state->inactive_texture, NULL, NULL);

  for (int b = 0; b < state->bindings_count; b++) {
    if (state->bindings[b].is_active) {
      SDL_Rect rect;
      rect.h = state->bindings[b].h;
      rect.w = state->bindings[b].w;
      rect.x = state->bindings[b].x_pos;
      rect.y = state->bindings[b].y_pos;

      SDL_RenderCopy(state->renderer, state->active_texture, &rect, &rect);
    }
  }

  SDL_RenderPresent(renderer);
}

void update_bindings(struct state_t *state) {
  struct pollfd pfd[1];
  pfd[0].fd = 0; // STDIN
  pfd[0].events = POLLIN;
  poll(pfd, 1, 10); // timeout = 10
  if ((pfd[0].revents & POLLIN)) {
    char buffer[256] = {0};
    fgets(buffer, 256, stdin);
    bool key_pressed;
    if (buffer[0] == '+') {
      key_pressed = true;
    } else if (buffer[0] == '-') {
      key_pressed = false;
    } else {
      printf("Unrecognized action : '%c'\n", buffer[0]);
      cleanup_state(state);
      exit(-1);
    }
    char *key_str = buffer + 1;
    key_str[strcspn(key_str, "\n")] = 0; // cut before newline
    for (int b = 0; b < state->bindings_count; b++) {
      if (strcmp(key_str, state->bindings[b].name) == 0) {
        state->bindings[b].is_active = key_pressed;
        state->changed = true;
      }
    }
  }
}

// must be called after the renderer has been initialized
int load_textures(struct state_t *state, char *active_name,
                  char *inactive_name) {
  state->inactive_texture = IMG_LoadTexture(state->renderer, inactive_name);
  state->active_texture = IMG_LoadTexture(state->renderer, active_name);
  if (state->inactive_texture == NULL) {
    printf("Couldn't load inactive texture: %s\n", inactive_name);
    return 1;
  }
  if (state->active_texture == NULL) {
    printf("Couldn't load active texture: %s\n", active_name);
    return 1;
  }
  return 0;
}

int main() {
  struct state_t state;
  struct config_t config = get_config();

  state.bindings_count = config.binding_count;
  state.bindings = malloc(sizeof(struct binding_t) * state.bindings_count);
  if (!state.bindings) {
    printf("Couldn't allocate memory for bindings\n");
    cleanup_state(&state);
    exit(-1);
  }

  for (int b = 0; b < state.bindings_count; b++) {
    state.bindings[b].is_active = false;
    state.bindings[b].x_pos = config.bindings[b].x_pos;
    state.bindings[b].y_pos = config.bindings[b].y_pos;
    state.bindings[b].h = config.bindings[b].height;
    state.bindings[b].w = config.bindings[b].width;
    state.bindings[b].name = config.bindings[b].name;
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't init SDL\n");
    exit(-1);
  }

  state.window = SDL_CreateWindow("Input Overlay", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, 0, 0,
                                  0); // we set the size later
  if (!state.window) {
    printf("Couldn't create SDL window: %s\n", SDL_GetError());
    exit(-1);
  }

  state.renderer = SDL_CreateRenderer(state.window, -1, 0);
  if (!state.renderer) {
    printf("Couldn't create SDL renderer: %s\n", SDL_GetError());
    cleanup_state(&state);
    exit(-1);
  }

  if (load_textures(&state, config.active_texture, config.inactive_texture)) {
    cleanup_state(&state);
    exit(-1);
  }

  int window_width;
  int window_height;
  if (SDL_QueryTexture(state.inactive_texture, NULL, NULL, &window_width,
                       &window_height)) {
    printf("Couldn't calculate window size\n");
    cleanup_state(&state);
    exit(-1);
  }
  SDL_SetWindowSize(state.window, window_width, window_height);

  state.running = true;
  state.changed = true;
  while (state.running) {
    if (state.changed)
      draw(state.renderer, &state);
    state.changed = false;
    poll_events(&state);
    update_bindings(&state);
  }

  cleanup_state(&state);
  return 0;
}
