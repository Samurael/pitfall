#ifndef GAME_H
#define GAME_H

#include "player.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

typedef struct {
  int left, right, jump, crouch, pause;
} Keymap;

typedef struct {
  Player player;
  int fase;
  int pausado;
  float scroll_x;
  Keymap keys;
  int dificuldade;
  int has_double_jump_item;
  ALLEGRO_FONT *font;

// Parallax
#define NUM_LAYERS 5
  ALLEGRO_BITMAP *bg_layers[NUM_LAYERS];
  float bg_speeds[NUM_LAYERS];
} Game;

void game_init(Game *g);
void game_update(Game *g, float dt);
void game_draw(Game *g);
void game_handle_key_down(Game *g, int keycode);
void game_handle_key_up(Game *g, int keycode);

#endif
