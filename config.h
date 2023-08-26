#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>

// should be largly enough to hold a key name
#define BINDING_NAME_LENGTH 128

struct binding_config_t {
  char name[BINDING_NAME_LENGTH];
  int x_pos, y_pos, width, height;
};

struct config_t {
  int binding_count;
  struct binding_config_t *bindings;
  char inactive_texture[1024]; // key that aren't pressed
  char active_texture[1024]; // that that are pressed
};

struct config_t get_config(void);

#endif
