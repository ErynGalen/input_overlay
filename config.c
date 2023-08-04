#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

struct config_t get_config() {
  struct config_t config = {
      .binding_count = 0,
      .bindings = NULL,
      .active_texture = "config/active.png",
      .inactive_texture = "config/inactive.png",
  };

  FILE *config_file = fopen("config/config.conf", "r");
  if (config_file == NULL) {
    printf("Couldn't open config file\n");
    return config;
  }

  // parse config file

  // parser state
  enum {
    AT_LINE_START,
    IN_NAME,
    IN_INACTIVE,
    IN_ACTIVE,
    IN_KEY_DEF,
  };
  int state = 0;

  // 0 = key name
  // 1-4 = x, y, w, h
  // only valid when state == IN_KEY_DEF
  int key_def_pos = 0;
  char token[1024];     // can't have file names longer than 1024
  int token_length = 0; // excluding the final \0

  int c = 0;
  int line = 1;
  while (1) {
    c = fgetc(config_file);
    if (token_length >= 1022) {
      printf("Too long token: %s\n", token);
      return config;
    }
    switch (c) {
    case ' ':
    case '\t':
      continue;
    case '\n':
    case EOF:
      switch (state) {
      case AT_LINE_START:
        break;
      case IN_ACTIVE:
        strncpy(config.active_texture, token, 1024);
        break;
      case IN_INACTIVE:
        strncpy(config.inactive_texture, token, 1024);
        break;
      case IN_NAME:
        printf("Unexpected EOL at line %d\n", line);
        return config;
      case IN_KEY_DEF:
        if (key_def_pos < 5) {
          printf("Unexpected EOL at line %d\n", line);
          return config;
        }
        break;
      }
      state = AT_LINE_START;
      token_length = 0;
      token[0] = 0;
      line++;
      break;
    case '=':
      if (state != IN_NAME) {
        printf("Error parsing config: unexpected '=' at line %d\n", line);
        return config;
      }
      if (strcmp(token, "inactive") == 0) {
        state = IN_INACTIVE;
        token_length = 0;
        token[0] = 0;
      } else if (strcmp(token, "active") == 0) {
        state = IN_ACTIVE;
        token_length = 0;
        token[0] = 0;
      } else if (strcmp(token, "key") == 0) {
        state = IN_KEY_DEF;
        key_def_pos = 0;
        token_length = 0;
        token[0] = 0;

        config.binding_count++;
        // very unefficient, but should be fine?
        config.bindings =
            realloc(config.bindings,
                    sizeof(struct binding_config_t) * config.binding_count);
        // append default binding to the config
        static struct binding_config_t default_binding = {
            .name = "",
            .x_pos = 0,
            .y_pos = 0,
            .width = 0,
            .height = 0,
        };
        config.bindings[config.binding_count - 1] = default_binding;
      } else {
        printf("Error parsing config: unknown ident '%s' at line %d\n", token,
               line);
        return config;
      }
      break;
    case ',':
      if (state != IN_KEY_DEF || key_def_pos > 4) {
        printf("Error parsing config: unexpected ',' at line %d\n", line);
        return config;
      }
      if (key_def_pos == 0) {
        strncpy(config.bindings[config.binding_count - 1].name, token,
                BINDING_NAME_LENGTH);
      } else {
        int value = atoi(token);
        switch (key_def_pos) {
        case 1:
          config.bindings[config.binding_count - 1].x_pos = value;
          break;
        case 2:
          config.bindings[config.binding_count - 1].y_pos = value;
          break;
        case 3:
          config.bindings[config.binding_count - 1].width = value;
          break;
        case 4:
          config.bindings[config.binding_count - 1].height = value;
          break;
        }
      }
      key_def_pos++;
      token_length = 0;
      token[0] = 0;
      break;
    default:
      if (state == AT_LINE_START) {
        state = IN_NAME;
      }
      token[token_length] = (char)c;
      token_length++;
      token[token_length] = 0;
    }
    if (c == EOF) {
      break;
    }
  }

  return config;
}
