#ifndef UTIL_H
#define UTIL_H

typedef enum { MENU, JOGO, GAME_OVER } Estado;

typedef struct {
  int left, right, jump, crouch, pause;
} Keymap;

typedef struct {
  int fase;
  int vida;
  int desbloqueou_duplo_pulo;
} SaveData;

int save_game(const char *filename, SaveData *s);
int load_game(const char *filename, SaveData *s);

#endif
