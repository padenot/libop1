#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  FILE* f = fopen(argv[1], "r");

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t* buf = (uint8_t*)malloc(fsize);
  fread(buf, fsize, 1, f);
  fclose(f);

  size_t i = 0;
  while (i < fsize - 4) {
    if (buf[i + 0] == 'A' &&
        buf[i + 1] == 'P' &&
        buf[i + 2] == 'P' &&
        buf[i + 3] == 'L') {
      break;
    }
    i++;
  }

  uint32_t chunk_size_index = i + 4;

  int32_t chunk_size = (buf[chunk_size_index + 3] << 0)
                     | (buf[chunk_size_index + 2] << 8)
                     | (buf[chunk_size_index + 1] << 16)
                     | (buf[chunk_size_index + 0] << 24);

  for (uint32_t i = 0; i < chunk_size - 4; i++) {
    printf("%c", buf[chunk_size_index + 8 + i]);
  }
  printf("\n");

  return 0;
}
