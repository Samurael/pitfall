#include "player.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>

void safe_load(ALLEGRO_BITMAP **bmp, const char *path) {
  *bmp = al_load_bitmap(path);
  if (!*bmp) {
    fprintf(stderr, "Aviso: %s não encontrado, usando placeholder.\n", path);
    *bmp = al_create_bitmap(32, 32);
    al_set_target_bitmap(*bmp);
    al_clear_to_color(al_map_rgb(255, 0, 255));
    al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
  }
}

void player_init(Player *p) {
  p->x = 100;
  p->y = 280;
  p->vx = p->vy = 0;
  p->w = p->h = 32;
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

  p->facing_right = 1;

  // Carregar sprites
  safe_load(&p->spr_idle, "sprites/idle.png");
  p->idle_frames = 12;

  safe_load(&p->spr_run, "sprites/run.png");
  p->run_frames = 8;

  safe_load(&p->spr_jump_start, "sprites/jump_start.png");
  safe_load(&p->spr_midair, "sprites/midair.png");
  p->midair_frames = 2;

  safe_load(&p->spr_jump_land, "sprites/jump_land.png");

  safe_load(&p->spr_crouch, "sprites/crouch.png");
  safe_load(&p->spr_vine, "sprites/vine.png");
}

void update_animation(Player *p, float dt) {
  p->anim_time += dt;
  float frame_dur = 0.12f;
  if (p->state == P_RUN)
    frame_dur = 0.09f;
  if (p->state == P_VINE)
    frame_dur = 0.15f;

  if (p->anim_time >= frame_dur) {
    p->anim_time -= frame_dur;
    p->anim_frame++;

    switch (p->state) {
    case P_RUN:
      p->anim_frame %= p->run_frames;
      break;
    case P_IDLE:
      p->anim_frame %= p->idle_frames;
      break;
    case P_MIDAIR:
      p->anim_frame %= p->midair_frames;
      break;
    default:
      p->anim_frame = 0;
      break;
    }
  }
}

void player_update(Player *p, float dt) {
  if (p->on_vine) {
    float g = 1200.0f, base_len = 120.0f;
    float torque = -(g / base_len) * sinf(p->vine_angle);
    p->vine_omega += torque * dt;
    p->vine_omega *= 0.995f;
    p->vine_angle += p->vine_omega * dt;

    p->x = p->vine_anchor_x + base_len * sinf(p->vine_angle);
    p->y = p->vine_anchor_y + base_len * cosf(p->vine_angle) - p->h / 2;

    p->vx = p->vy = 0;
    p->state = P_VINE;
    update_animation(p, dt);
    return;
  }

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

  // Definir estado
  if (!p->na_chao) {
    if (p->vy < 0)
      p->state = P_JUMP_START;
    else
      p->state = P_MIDAIR;
  } else if (p->pulos_feitos > 0) {
    p->state = P_JUMP_LAND;
  } else if (p->crouch) {
    p->state = P_CROUCH;
  } else if (p->vx != 0) {
    p->state = P_RUN;
  } else {
    p->state = P_IDLE;
  }

  p->stamina += (p->vx == 0 ? 15.0f : -25.0f) * dt;
  if (p->stamina < 0)
    p->stamina = 0;
  if (p->stamina > 100)
    p->stamina = 100;

  update_animation(p, dt);
}

void player_draw(Player *p, float scroll_x) {
  ALLEGRO_BITMAP *b = NULL;
  int frames = 1;

  switch (p->state) {
  case P_IDLE:
    b = p->spr_idle;
    frames = p->idle_frames;
    break;
  case P_RUN:
    b = p->spr_run;
    frames = p->run_frames;
    break;
  case P_JUMP_START:
    b = p->spr_jump_start;
    break;
  case P_MIDAIR:
    b = p->spr_midair;
    frames = p->midair_frames;
    break;
  case P_JUMP_LAND:
    b = p->spr_jump_land;
    break;
  case P_CROUCH:
    b = p->spr_crouch;
    break;
  case P_VINE:
    b = p->spr_vine;
    break;
  }

  if (b) {
    int fw = al_get_bitmap_width(b) / frames;
    int fh = al_get_bitmap_height(b);
    int flags = p->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;

    al_draw_bitmap_region(b, p->anim_frame * fw, 0, fw, fh, p->x, p->y, flags);
  } else {
    al_draw_filled_rectangle(p->x, p->y, p->x + p->w, p->y + p->h,
                             al_map_rgb(255, 255, 255));
  }
}

void player_move_left(Player *p) {
  if (p->stamina > 0) {
    p->vx = -200;
    p->facing_right = 0;
  }
}

void player_move_right(Player *p) {
  if (p->stamina > 0) {
    p->vx = 200;
    p->facing_right = 1;
  }
}

void player_stop(Player *p) { p->vx = 0; }

void player_jump(Player *p) {
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