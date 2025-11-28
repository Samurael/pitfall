#include "util.h"
#include <stdio.h>

int save_game(const char *filename, SaveData *s) {
  FILE *f = fopen(filename, "wb");
  if (!f)
    return 0;
  fwrite(s, sizeof(SaveData), 1, f);
  fclose(f);
  return 1;
}

int load_game(const char *filename, SaveData *s) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return 0;
  fread(s, sizeof(SaveData), 1, f);
  fclose(f);
  return 1;
}
