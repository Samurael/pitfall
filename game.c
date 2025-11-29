#include "game.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  float x, y, w, h;
  float vx;
  int active;
  int type; // 0=estático, 1=móvel horiz, 2=plataforma cai, 3=cipó
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
    obst[3] = (Obstacle){1300, 300, 100, 20, -30, 1, 1};
    obst[4] = (Obstacle){1600, 260, 40, 40, 0, 1, 0};
    obst[5] = (Obstacle){2000, 240, 20, 20, 0, 1, 3};
    num_obs = 6;
  } else {
    obst[0] = (Obstacle){300, 300, 60, 20, 0, 1, 0};
    obst[1] = (Obstacle){600, 260, 40, 40, 0, 1, 0};
    obst[2] = (Obstacle){900, 300, 80, 20, 0, 1, 0};
    obst[3] = (Obstacle){1200, 240, 40, 40, 0, 1, 0};
    obst[4] = (Obstacle){1500, 300, 100, 20, 0, 1, 2};
    obst[5] = (Obstacle){1800, 240, 20, 20, 0, 1, 3};
    num_obs = 6;
  }
}

void game_init(Game *g) {
  player_init(&g->player);
  g->fase = 1;
  g->pausado = 0;
  g->scroll_x = 0;
  g->keys.left = ALLEGRO_KEY_A;
  g->keys.right = ALLEGRO_KEY_D;
  g->keys.jump = ALLEGRO_KEY_W;
  g->keys.crouch = ALLEGRO_KEY_S;
  g->keys.pause = ALLEGRO_KEY_P;
  g->dificuldade = 1;
  g->has_double_jump_item = 0;
  g->font = NULL;

  g->state = STATE_MENU;
  g->win = 0;

  const char *paths[NUM_LAYERS] = {
      "assets/backgrounds/layer1.png", "assets/backgrounds/layer2.png",
      "assets/backgrounds/layer3.png", "assets/backgrounds/layer4.png",
      "assets/backgrounds/layer5.png"};
  float speeds[NUM_LAYERS] = {0.2f, 0.4f, 0.6f, 0.8f, 1.0f};

  for (int i = 0; i < NUM_LAYERS; i++) {
    g->bg_layers[i] = al_load_bitmap(paths[i]);
    if (!g->bg_layers[i]) {
      fprintf(stderr, "Aviso: %s não encontrado, usando placeholder.\n",
              paths[i]);
      g->bg_layers[i] = al_create_bitmap(800, 600);
      al_set_target_bitmap(g->bg_layers[i]);
      al_clear_to_color(al_map_rgb(100 + i * 20, 100 + i * 20, 255));
      al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
    }
    g->bg_speeds[i] = speeds[i];
  }

  spawn_for_phase(g->fase);
}

static int aabb(float ax, float ay, float aw, float ah, float bx, float by,
                float bw, float bh) {
  return !(ax + aw < bx || ax > bx + bw || ay + ah < by || ay > by + bh);
}

void game_update(Game *g, float dt) {
  if (g->state != STATE_PLAYING || g->pausado)
    return;

  float diff_mul = 1.0f + g->dificuldade * 0.25f;

  for (int i = 0; i < num_obs; i++) {
    if (!obst[i].active)
      obst[i].active = 1;
    if (obst[i].type == 1) {
      obst[i].x += obst[i].vx * dt * diff_mul;
      if (obst[i].x < 100 || obst[i].x > 1700)
        obst[i].vx *= -1;
    }
  }

  player_update(&g->player, dt);

  float center = 300.0f;
  if (g->player.x > center) {
    float diff = g->player.x - center;
    g->scroll_x += diff;
    g->player.x = center;
  }

  int on_obstacle = 0; // Verifica se o player está sobre algum obstáculo

  for (int i = 0; i < num_obs; i++) {
    Obstacle *o = &obst[i];
    float ox = o->x - g->scroll_x;
    float oy = o->y;
    if (!o->active)
      continue;
    if (ox + o->w < -50 || ox > 850)
      continue;

    if (o->type == 3) { // Cipó
      float world_px = g->player.x + g->scroll_x + g->player.w / 2;
      float world_py = g->player.y + g->player.h / 2;
      float dx = world_px - (o->x + o->w / 2);
      float dy = world_py - (o->y + o->h / 2);
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < 160.0f && g->player.vy > 0) {
        g->player.on_vine = 1;
        g->player.vine_anchor_x = o->x + o->w / 2;
        g->player.vine_anchor_y = o->y + o->h / 2;
        float Lx = g->player.x + g->player.w / 2 - g->player.vine_anchor_x;
        float Ly = g->player.y + g->player.h / 2 - g->player.vine_anchor_y;
        g->player.vine_angle = atan2f(Lx, Ly);
        g->player.vine_omega = g->player.vx / 60.0f;
        break;
      }
      continue;
    }

    // Colisão AABB
    if (aabb(g->player.x, g->player.y, g->player.w, g->player.h, ox, oy, o->w,
             o->h)) {
      if (g->player.vy >= 0 && g->player.y + g->player.h <= oy + 10) {
        g->player.y = oy - g->player.h;
        g->player.vy = 0;
        g->player.na_chao = 1;
        g->player.pulos_feitos = 0;
        on_obstacle = 1;
        if (o->type == 2)
          o->type = 99;
      } else {
        g->player.vida -= 20 * dt;
        if (g->player.vida <= 0) {
          g->player.vida = 0;
          g->state = STATE_GAMEOVER;
          g->win = 0;
        }
      }
    }
  }

  // Se não estiver sobre obstáculo nem chão, cair normalmente
  if (!on_obstacle && g->player.y + g->player.h < 320) {
    g->player.na_chao = 0;
  }

  for (int i = 0; i < num_obs; i++) {
    if (obst[i].type == 99) {
      obst[i].y += 400.0f * dt;
      if (obst[i].y > 1000)
        obst[i].active = 0;
    }
  }

  if (g->scroll_x > 2600 && g->fase == 2) {
    g->state = STATE_GAMEOVER;
    g->win = 1;
  } else if (g->scroll_x > 2600) {
    g->fase = 2;
    spawn_for_phase(g->fase);
    g->scroll_x = 0;
    g->player.x = 100;
    g->player.y = 280;
    g->player.vx = 0;
    g->player.vy = 0;
  }
}

void game_draw(Game *g) {
  int screen_w = 800, screen_h = 600;

  if (g->state == STATE_MENU) {
    al_clear_to_color(al_map_rgb(50, 50, 100));
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 0), screen_w / 2, screen_h / 3,
                 ALLEGRO_ALIGN_CENTER, "PITFALL");
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 255), screen_w / 2, screen_h / 2,
                 ALLEGRO_ALIGN_CENTER, "Pressione ENTER para iniciar");
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 255), screen_w / 2, screen_h / 1.8,
                 ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");
    return;
  }

  if (g->state == STATE_GAMEOVER) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    if (g->win) {
      al_draw_text(g->font ? g->font : al_create_builtin_font(),
                   al_map_rgb(0, 255, 0), screen_w / 2, screen_h / 3,
                   ALLEGRO_ALIGN_CENTER, "VOCÊ VENCEU!");
    } else {
      al_draw_text(g->font ? g->font : al_create_builtin_font(),
                   al_map_rgb(255, 0, 0), screen_w / 2, screen_h / 3,
                   ALLEGRO_ALIGN_CENTER, "GAME OVER!");
    }
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 255), screen_w / 2, screen_h / 2,
                 ALLEGRO_ALIGN_CENTER, "Pressione ENTER para voltar ao menu");
    return;
  }

  // ===== Fundo parallax =====
  for (int i = 0; i < NUM_LAYERS; i++) {
    float offset_x = g->scroll_x * g->bg_speeds[i];
    ALLEGRO_BITMAP *bmp = g->bg_layers[i];
    int w = al_get_bitmap_width(bmp);
    int h = al_get_bitmap_height(bmp);

    float scale_y = (float)screen_h / h;
    float scaled_w = w * scale_y;
    int repeat = ceil(screen_w / scaled_w) + 2;

    for (int t = -1; t < repeat; t++) {
      al_draw_scaled_bitmap(bmp, 0, 0, w, h,
                            t * scaled_w - fmod(offset_x, scaled_w), 0,
                            scaled_w, screen_h, 0);
    }
  }

  al_draw_filled_rectangle(0, 320, screen_w, screen_h, al_map_rgb(34, 139, 34));

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

  // ===== Barras de Vida e Stamina =====
  int bar_w = 200, bar_h = 20;
  int x_offset = 10, y_offset = 10;

  // Vida (vermelha)
  float vida_perc = g->player.vida / 100.0f;
  al_draw_filled_rectangle(x_offset, y_offset, x_offset + bar_w * vida_perc,
                           y_offset + bar_h, al_map_rgb(200, 0, 0));
  al_draw_rectangle(x_offset, y_offset, x_offset + bar_w, y_offset + bar_h,
                    al_map_rgb(255, 255, 255), 2);

  // Stamina (verde)
  float stamina_perc = g->player.stamina / 100.0f;
  al_draw_filled_rectangle(x_offset, y_offset + 25,
                           x_offset + bar_w * stamina_perc,
                           y_offset + 25 + bar_h, al_map_rgb(0, 200, 0));
  al_draw_rectangle(x_offset, y_offset + 25, x_offset + bar_w,
                    y_offset + 25 + bar_h, al_map_rgb(255, 255, 255), 2);

  if (g->pausado) {
    al_draw_text(g->font ? g->font : al_create_builtin_font(),
                 al_map_rgb(255, 255, 0), screen_w / 2, screen_h / 3,
                 ALLEGRO_ALIGN_CENTER, "PAUSADO");
  }
}

void game_handle_key_down(Game *g, int keycode) {
  if (g->state == STATE_MENU) {
    if (keycode == ALLEGRO_KEY_ENTER) {
      g->state = STATE_PLAYING;
    }
    if (keycode == ALLEGRO_KEY_ESCAPE) {
      exit(0);
    }
    return;
  }

  if (g->state == STATE_GAMEOVER) {
    if (keycode == ALLEGRO_KEY_ENTER) {
      g->state = STATE_MENU;
      g->fase = 1;
      g->scroll_x = 0;
      player_init(&g->player);
      spawn_for_phase(g->fase);
    }
    return;
  }

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
