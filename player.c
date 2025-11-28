#include "player.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>

#define DEG_TO_RAD (3.14159265f / 180.0f)

void safe_load(ALLEGRO_BITMAP **bmp, const char *path) {
  *bmp = al_load_bitmap(path);
}

void player_init(Player *p) {
  p->x = 100;
  p->y = 280;
  p->vx = 0;
  p->vy = 0;
  p->w = 32;
  p->h = 32;
  p->vida = 100;
  p->na_chao = 1;
  p->crouch = 0;
  p->dupla_pulo = 0;
  p->pulos_feitos = 0;
  p->stamina = 100.0f;

  p->state = P_IDLE;
  p->anim_time = 0;
  p->anim_frame = 0;

  p->on_vine = 0;
  p->vine_angle = 0;
  p->vine_omega = 0;

  safe_load(&p->spr_idle, "sprites/idle.png");

  for (int i = 0; i < 4; i++) {
    char buf[128];
    snprintf(buf, 128, "sprites/run_%d.png", i + 1);
    safe_load(&p->spr_run[i], buf);
  }

  safe_load(&p->spr_jump, "sprites/jump.png");
  safe_load(&p->spr_crouch, "sprites/crouch.png");

  for (int i = 0; i < 3; i++) {
    char buf[128];
    snprintf(buf, 128, "sprites/vine_swing_%d.png", i + 1);
    safe_load(&p->spr_vine[i], buf);
  }
}

void update_animation(Player *p, float dt) {
  p->anim_time += dt;

  float frame_dur = (p->state == P_RUN)    ? 0.09f
                    : (p->state == P_VINE) ? 0.15f
                                           : 0.12f;

  if (p->anim_time >= frame_dur) {
    p->anim_time -= frame_dur;
    p->anim_frame++;

    if (p->state == P_RUN)
      p->anim_frame %= 4;
    else if (p->state == P_VINE)
      p->anim_frame %= 3;
    else
      p->anim_frame = 0;
  }
}

void player_update(Player *p, float dt) {
  // ======== SE ESTIVER NO CIPÓ ==========
  if (p->on_vine) {
    float g = 1200.0f;
    float base_len = 120.0f;

    float torque = -(g / base_len) * sinf(p->vine_angle);
    p->vine_omega += torque * dt;
    p->vine_omega *= 0.995f;
    p->vine_angle += p->vine_omega * dt;

    p->x = p->vine_anchor_x + base_len * sinf(p->vine_angle);
    p->y = p->vine_anchor_y + base_len * cosf(p->vine_angle) - p->h / 2;

    p->vx = 0;
    p->vy = 0;

    p->state = P_VINE;
    update_animation(p, dt);
    return;
  }

  // ======== FÍSICA NORMAL ==========
  if (!p->na_chao)
    p->vy += 1200.0f * dt;

  p->x += p->vx * dt;
  p->y += p->vy * dt;

  if (p->y >= 320 - p->h) {
    p->y = 320 - p->h;
    p->vy = 0;
    p->na_chao = 1;
    p->pulos_feitos = 0;
  }

  if (!p->na_chao)
    p->state = P_JUMP;
  else if (p->crouch)
    p->state = P_CROUCH;
  else if (p->vx != 0)
    p->state = P_RUN;
  else
    p->state = P_IDLE;

  p->stamina += (p->vx == 0 ? 15.0f : -25.0f) * dt;
  if (p->stamina < 0)
    p->stamina = 0;
  if (p->stamina > 100)
    p->stamina = 100;

  update_animation(p, dt);
}

void player_draw(Player *p, float scroll_x) {
  float sx = p->x;
  float sy = p->y;

  ALLEGRO_BITMAP *b = NULL;

  switch (p->state) {
  case P_IDLE:
    b = p->spr_idle;
    break;
  case P_RUN:
    b = p->spr_run[p->anim_frame];
    break;
  case P_JUMP:
    b = p->spr_jump;
    break;
  case P_CROUCH:
    b = p->spr_crouch;
    break;
  case P_VINE:
    b = p->spr_vine[p->anim_frame];
    break;
  }

  if (b)
    al_draw_bitmap(b, sx, sy, 0);
  else
    al_draw_filled_rectangle(sx, sy, sx + p->w, sy + p->h,
                             al_map_rgb(255, 255, 255));
}

void player_move_left(Player *p) {
  if (p->stamina > 0)
    p->vx = -200;
}
void player_move_right(Player *p) {
  if (p->stamina > 0)
    p->vx = 200;
}
void player_stop(Player *p) { p->vx = 0; }

void player_jump(Player *p) {
  if (p->on_vine) {
    float base_len = 120.0f;
    float tang = base_len * p->vine_omega;

    p->vx = cosf(p->vine_angle) * tang * 0.2f;
    p->vy = -350 - sinf(p->vine_angle) * tang * 0.2f;

    p->on_vine = 0;
    p->na_chao = 0;
    p->pulos_feitos = 1;
    return;
  }

  if (p->na_chao) {
    p->vy = -420;
    p->na_chao = 0;
    p->pulos_feitos = 1;
  } else if (p->dupla_pulo && p->pulos_feitos == 1) {
    p->vy = -380;
    p->pulos_feitos = 2;
  }
}

void player_crouch(Player *p, int on) { p->crouch = on; }
