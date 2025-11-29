#include "game.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define ANIM_FPS 30.0

int main() {
  if (!al_init() || !al_init_image_addon() || !al_install_keyboard()) {
    fprintf(stderr, "Erro inicializando Allegro\n");
    return -1;
  }
  al_init_primitives_addon();
  al_init_font_addon();
  al_init_ttf_addon();

  ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
  if (!display) {
    fprintf(stderr, "Erro criando display\n");
    return -1;
  }

  Game g;
  game_init(&g);

  ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
  ALLEGRO_TIMER *timer = al_create_timer(1.0 / ANIM_FPS);
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer));
  al_start_timer(timer);

  bool running = true, redraw = true;
  while (running) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      game_update(&g, 1.0 / ANIM_FPS);
      redraw = true;
    } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
        running = false;
      game_handle_key_down(&g, ev.keyboard.keycode);
    } else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
      game_handle_key_up(&g, ev.keyboard.keycode);
    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      running = false;
    }

    if (redraw) {
      redraw = false;
      al_clear_to_color(al_map_rgb(0, 0, 0));
      game_draw(&g);
      al_flip_display();
    }
  }

  al_destroy_display(display);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  return 0;
}
