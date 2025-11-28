#include "game.h"

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  float x, y, w, h;
  float vx;
  int active;
  int type;
  // type:
  // 0 = estático
  // 1 = móvel horizontal
  // 2 = plataforma que cai
  // 3 = vine anchor (cipó)
} Obstacle;

#define MAX_OBS 20

static Obstacle obst[MAX_OBS];
static int num_obs = 0;

static void clear_obstacles() {
  num_obs = 0;
  memset(obst, 0, sizeof(obst));
}

static void spawn_for_phase(int phase) {
  clear_obstacles();

  if (phase == 1) {
    obst[0] = (Obstacle){400, 300, 80, 20, 0, 1, 0};
    obst[1] = (Obstacle){700, 300, 80, 20, 0, 1, 0};
    obst[2] = (Obstacle){1000, 260, 40, 40, 0, 1, 0};

    // plataforma móvel
    obst[3] = (Obstacle){1300, 300, 100, 20, -30, 1, 1};

    obst[4] = (Obstacle){1600, 260, 40, 40, 0, 1, 0};

    // âncora do cipó
    obst[5] = (Obstacle){2000, 240, 20, 20, 0, 1, 3};

    num_obs = 6;
  } else {
    obst[0] = (Obstacle){300, 300, 60, 20, 0, 1, 0};
    obst[1] = (Obstacle){600, 260, 40, 40, 0, 1, 0};
    obst[2] = (Obstacle){900, 300, 80, 20, 0, 1, 0};
    obst[3] = (Obstacle){1200, 240, 40, 40, 0, 1, 0};

    // plataforma que cai
    obst[4] = (Obstacle){1500, 300, 100, 20, 0, 1, 2};

    // âncora do cipó
    obst[5] = (Obstacle){1800, 240, 20, 20, 0, 1, 3};

    num_obs = 6;
  }
}

void game_init(Game *g) {
  player_init(&g->player);

  g->fase = 1;
  g->pausado = 0;
  g->scroll_x = 0;

  g->keys.left = ALLEGRO_KEY_LEFT;
  g->keys.right = ALLEGRO_KEY_RIGHT;
  g->keys.jump = ALLEGRO_KEY_SPACE;
  g->keys.crouch = ALLEGRO_KEY_DOWN;
  g->keys.pause = ALLEGRO_KEY_P;

  g->dificuldade = 1;
  g->has_double_jump_item = 0;
  g->font = NULL;

  spawn_for_phase(g->fase);
}

static int aabb(float ax, float ay, float aw, float ah, float bx, float by,
                float bw, float bh) {
  return !(ax + aw < bx || ax > bx + bw || ay + ah < by || ay > by + bh);
}

void game_update(Game *g, float dt) {
  if (g->pausado)
    return;

  float diff_mul = 1.0f + g->dificuldade * 0.25f;

  // Atualizar obstáculos
  for (int i = 0; i < num_obs; i++) {
    if (!obst[i].active)
      obst[i].active = 1;

    if (obst[i].type == 1) { // móvel horizontal
      obst[i].x += obst[i].vx * dt * diff_mul;

      if (obst[i].x < 1000)
        obst[i].vx *= -1;
      if (obst[i].x > 1700)
        obst[i].vx *= -1;
    }
  }

  player_update(&g->player, dt);

  // Scroll
  float center = 300.0f;
  if (g->player.x > center) {
    float diff = g->player.x - center;
    g->scroll_x += diff;
    g->player.x = center;
  }

  // Colisões
  for (int i = 0; i < num_obs; i++) {
    Obstacle *o = &obst[i];

    float ox = o->x - g->scroll_x;
    float oy = o->y;

    if (!o->active)
      continue;
    if (ox + o->w < -50 || ox > 850)
      continue;

    // CIPÓ (anchor)
    if (o->type == 3) {
      float world_px = g->player.x + g->scroll_x + g->player.w / 2;
      float world_py = g->player.y + g->player.h / 2;

      float dx = world_px - (o->x + o->w / 2);
      float dy = world_py - (o->y + o->h / 2);

      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < 160.0f && g->player.vy > 0) {
        // agarrar cipó
        g->player.on_vine = 1;

        g->player.vine_anchor_x = o->x + o->w / 2;
        g->player.vine_anchor_y = o->y + o->h / 2;

        float Lx = g->player.x + g->scroll_x - g->player.vine_anchor_x;
        float Ly = (g->player.y + g->player.h / 2) - g->player.vine_anchor_y;

        g->player.vine_angle = atan2f(Lx, Ly);
        g->player.vine_omega = g->player.vx / 60.0f;

        break;
      }
      continue;
    }

    // colisão normal
    if (aabb(g->player.x, g->player.y, g->player.w, g->player.h, ox, oy, o->w,
             o->h)) {

      // pousou sobre plataforma
      if (g->player.vy >= 0 && g->player.y + g->player.h <= oy + 10) {
        g->player.y = oy - g->player.h;
        g->player.vy = 0;
        g->player.na_chao = 1;
        g->player.pulos_feitos = 0;

        if (o->type == 2)
          o->type = 99; // agora ela cai
      } else {
        g->player.vida -= 20 * dt;
        if (g->player.vida < 0)
          g->player.vida = 0;
      }
    }
  }

  // plataforma que cai
  for (int i = 0; i < num_obs; i++) {
    if (obst[i].type == 99) {
      obst[i].y += 400.0f * dt;
      if (obst[i].y > 1000)
        obst[i].active = 0;
    }
  }

  // troca de fase
  if (g->scroll_x > 2600) {
    if (g->fase == 1) {
      g->fase = 2;
      spawn_for_phase(g->fase);
      g->scroll_x = 0;

      g->player.x = 100;
      g->player.y = 280;
      g->player.vx = 0;
      g->player.vy = 0;
    }
  }
}

void game_draw(Game *g) {
  if (g->fase == 1)
    al_clear_to_color(al_map_rgb(100, 149, 237));
  else
    al_clear_to_color(al_map_rgb(50, 30, 90));

  al_draw_filled_rectangle(0, 320, 800, 400, al_map_rgb(34, 139, 34));

  // Obstáculos
  for (int i = 0; i < num_obs; i++) {
    Obstacle *o = &obst[i];
    if (!o->active)
      continue;

    float ox = o->x - g->scroll_x;
    float oy = o->y;

    if (o->type == 3) {
      al_draw_filled_circle(ox + o->w / 2, oy + o->h / 2, 6,
                            al_map_rgb(80, 40, 10));

      if (g->player.on_vine) {
        al_draw_line(g->player.vine_anchor_x - g->scroll_x,
                     g->player.vine_anchor_y, g->player.x + g->player.w / 2,
                     g->player.y + g->player.h / 4, al_map_rgb(150, 75, 0), 2);
      }
    } else {
      al_draw_filled_rectangle(ox, oy, ox + o->w, oy + o->h,
                               al_map_rgb(180, 50, 50));
    }
  }

  player_draw(&g->player, g->scroll_x);

  char buf[128];
  sprintf(buf, "Vida: %d  Stamina: %.0f  Fase: %d", g->player.vida,
          g->player.stamina, g->fase);

  if (g->font)
    al_draw_text(g->font, al_map_rgb(255, 255, 255), 10, 10, 0, buf);
  else
    al_draw_text(al_create_builtin_font(), al_map_rgb(255, 255, 255), 10, 10, 0,
                 buf);

  if (g->pausado)
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 0), 400, 180, ALLEGRO_ALIGN_CENTER,
                 "PAUSADO");
}

void game_handle_key_down(Game *g, int keycode) {
  if (keycode == g->keys.left)
    player_move_left(&g->player);
  if (keycode == g->keys.right)
    player_move_right(&g->player);
  if (keycode == g->keys.jump)
    player_jump(&g->player);
  if (keycode == g->keys.crouch)
    player_crouch(&g->player, 1);
  if (keycode == g->keys.pause)
    g->pausado = !g->pausado;
}

void game_handle_key_up(Game *g, int keycode) {
  if (keycode == g->keys.left || keycode == g->keys.right)
    player_stop(&g->player);
  if (keycode == g->keys.crouch)
    player_crouch(&g->player, 0);
}
