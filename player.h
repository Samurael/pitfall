#ifndef PLAYER_H
#define PLAYER_H

#include <allegro5/allegro.h>

typedef enum { P_IDLE, P_RUN, P_JUMP, P_CROUCH, P_VINE } PlayerState;

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

  ALLEGRO_BITMAP *spr_idle;
  ALLEGRO_BITMAP *spr_run[4];
  ALLEGRO_BITMAP *spr_jump;
  ALLEGRO_BITMAP *spr_crouch;
  ALLEGRO_BITMAP *spr_vine[3];

  float anim_time;
  int anim_frame;

  int on_vine;
  float vine_anchor_x, vine_anchor_y;
  float vine_angle;
  float vine_omega;
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
