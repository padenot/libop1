#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cli.hpp"

int main(int argc, const char ** argv) 
{
  cli::Parser parser(argc, argv);

  parser.help() << R"(op1-dump
    Usage: op1-drum audio-file.aif [audio-file2.aif...]

    Dumps on stdout the proprietary JSON of a OP-1 drum or synth sample.)";

  parser.getRemainingArguments(argc, argv);

  for (int argindex = 1; argindex < argc - 1; argindex++) {
    FILE* f = fopen(argv[argindex], "r");

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

  }

  return 0;
}
