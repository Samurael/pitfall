#ifndef GAME_H
#define GAME_H

#include "player.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define NUM_LAYERS 5

typedef struct {
  int left, right, jump, crouch, pause;
} GameKeys;

typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER } GameState;

typedef struct {
  Player player;
  int fase;
  float scroll_x;
  int pausado;
  ALLEGRO_BITMAP *bg_layers[NUM_LAYERS];
  float bg_speeds[NUM_LAYERS];
  GameKeys keys;
  int dificuldade;
  int has_double_jump_item;
  ALLEGRO_FONT *font;
  ALLEGRO_BITMAP *platform_tileset;

  // Novos campos
  GameState state;
  int win; // 1 = vitória, 0 = derrota
} Game;

// Funções principais
void game_init(Game *g);
void game_update(Game *g, float dt);
void game_draw(Game *g);
void game_handle_key_down(Game *g, int keycode);
void game_handle_key_up(Game *g, int keycode);

#endif