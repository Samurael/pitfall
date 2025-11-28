#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include <stdio.h>

#include "game.h"
#include "util.h"

#define WIDTH 800
#define HEIGHT 400

int main() {
  // ----------------------------------
  // Inicialização da Allegro
  // ----------------------------------
  if (!al_init()) {
    printf("Erro ao iniciar Allegro!\n");
    return 1;
  }

  al_init_font_addon();
  al_init_ttf_addon();
  al_init_image_addon();
  al_init_primitives_addon();
  al_install_keyboard();

  ALLEGRO_DISPLAY *display = al_create_display(WIDTH, HEIGHT);
  if (!display) {
    printf("Erro ao criar display.\n");
    return 1;
  }

  ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
  ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60);

  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer));

  ALLEGRO_FONT *font = al_load_ttf_font("arial.ttf", 24, 0);
  if (!font) {
    font = al_create_builtin_font();
    printf("Fonte arial.ttf nao encontrada. Usando fonte padrao.\n");
  }

  // ----------------------------------
  // Estados
  // ----------------------------------
  Estado estado = MENU;
  Game game;

  game_init(&game);
  game.font = font;

  int running = 1;

  al_start_timer(timer);

  // ----------------------------------
  // Loop principal
  // ----------------------------------
  while (running) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(queue, &ev);

    if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      running = 0;

    // -----------------------------
    // INPUT
    // -----------------------------
    if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      if (estado == MENU) {
        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
          estado = JOGO;
        }
      } else if (estado == GAME_OVER) {
        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
          estado = MENU;
          game_init(&game);
          game.font = font;
        }
      } else if (estado == JOGO) {
        game_handle_key_down(&game, ev.keyboard.keycode);
      }
    }

    if (ev.type == ALLEGRO_EVENT_KEY_UP) {
      if (estado == JOGO)
        game_handle_key_up(&game, ev.keyboard.keycode);
    }

    // -----------------------------
    // UPDATE
    // -----------------------------
    if (ev.type == ALLEGRO_EVENT_TIMER) {
      if (estado == JOGO) {
        game_update(&game, 1.0 / 60.0);

        if (game.player.vida <= 0) {
          estado = GAME_OVER;
        }
      }

      // -----------------------------
      // DRAW
      // -----------------------------
      if (estado == MENU) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(font, al_map_rgb(255, 255, 255), WIDTH / 2, 120,
                     ALLEGRO_ALIGN_CENTER, "PLATAFORMA DE SOBREVIVENCIA");

        al_draw_text(font, al_map_rgb(255, 255, 255), WIDTH / 2, 240,
                     ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
      } else if (estado == JOGO) {
        game_draw(&game);
      } else if (estado == GAME_OVER) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(font, al_map_rgb(255, 0, 0), WIDTH / 2, 150,
                     ALLEGRO_ALIGN_CENTER, "GAME OVER");

        al_draw_text(font, al_map_rgb(255, 255, 255), WIDTH / 2, 260,
                     ALLEGRO_ALIGN_CENTER,
                     "Pressione ENTER para voltar ao menu");
      }

      al_flip_display();
    }
  }

  // ----------------------------------
  // Limpeza
  // ----------------------------------
  al_destroy_font(font);
  al_destroy_display(display);
  al_destroy_event_queue(queue);
  al_destroy_timer(timer);

  return 0;
}
