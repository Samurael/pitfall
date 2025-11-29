#ifndef PLAYER_H
#define PLAYER_H

#include <allegro5/allegro.h>

typedef enum {
  P_IDLE,
  P_RUN,
  P_JUMP_START,
  P_MIDAIR,
  P_JUMP_LAND,
  P_CROUCH,
  P_VINE
} PlayerState;

typedef struct {
  float x, y;
  float vx, vy;
  int w, h;
  int vida;
  int na_chao;
  int crouch;
  int dupla_pulo;
  int pulos_feitos;
  float stamina;
  PlayerState state;

  // Sprites
  ALLEGRO_BITMAP *spr_idle;
  ALLEGRO_BITMAP *spr_run;
  ALLEGRO_BITMAP *spr_jump_start;
  ALLEGRO_BITMAP *spr_midair;
  ALLEGRO_BITMAP *spr_jump_land;
  ALLEGRO_BITMAP *spr_crouch;
  ALLEGRO_BITMAP *spr_vine;

  int idle_frames;
  int run_frames;
  int midair_frames;

  float anim_time;
  int anim_frame;

  // Vine
  int on_vine;
  float vine_anchor_x, vine_anchor_y;
  float vine_angle;
  float vine_omega;

  // Direção
  int facing_right; // 1 = direita, 0 = esquerda
} Player;

void player_init(Player *p);
void player_update(Player *p, float dt);
void player_draw(Player *p, float scroll_x);
void player_move_left(Player *p);
void player_move_right(Player *p);
void player_stop(Player *p);
void player_jump(Player *p);
void player_crouch(Player *p, int on);

#endif
