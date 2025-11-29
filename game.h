#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "player.h"

#define NUM_LAYERS 5

typedef struct {
    int left, right, jump, crouch, pause;
} GameKeys;

typedef struct {
    Player player;
    int fase;
    float scroll_x;
    int pausado;
    ALLEGRO_BITMAP *bg_layers[NUM_LAYERS];  // Fundo parallax
    float bg_speeds[NUM_LAYERS];
    GameKeys keys;
    int dificuldade;
    int has_double_jump_item;
    ALLEGRO_FONT *font;
    ALLEGRO_BITMAP *platform_tileset;       // Tileset das plataformas (Jungle Pack)
} Game;

// Funções principais do jogo
void game_init(Game *g);
void game_update(Game *g, float dt);
void game_draw(Game *g);
void game_handle_key_down(Game *g, int keycode);
void game_handle_key_up(Game *g, int keycode);

#endif
