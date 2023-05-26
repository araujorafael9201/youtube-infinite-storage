#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define max_columns 1280
#define max_rows 720

FILE *get_output_file(char *basename, int c) {
  if (c == 0) {
    mkdir(basename, 0700);
  }

  char *n = malloc(sizeof(char) * strlen(basename) + 5);
  sprintf(n, "./%s/%d.ppm", basename, c);

  FILE *f = fopen(n, "wb+");

  return f;
}

void close_file(FILE *f, char *header, int row_count, int column_count) {
  printf("%d\n", row_count);

  // Fill the rest of the row with black
  while (column_count <= max_columns) {
    fwrite("0 0 0  ", strlen("0 0 0  "), 1, f);
    column_count++;
  }

  // Write ppm header
  fseek(f, 0, SEEK_SET);
  sprintf(header, "P3\n%u %u\n255\n", column_count, row_count);
  fwrite(header, strlen(header), 1, f);
  fclose(f);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <input file> <output file>\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    printf("Error: cannot open file %s\n", argv[1]);
    return 1;
  }

  uint32_t column_count = 0;
  uint32_t row_count = 1;
  size_t output_file_count = 0;

  FILE *output = get_output_file(argv[2], output_file_count);
  uint16_t *buffer = malloc(sizeof(uint8_t));

  // Write PPM Header
  char *header = malloc(sizeof("P3\n1280 720\n255\n"));
  fseek(output, strlen(header), SEEK_CUR);

  uint8_t value_counter = 0;
  char *w = malloc(sizeof("255  "));

  while (feof(fp) == 0) {
    fread(buffer, sizeof(uint8_t), 1, fp);
    sprintf(w, "%u ", *buffer);

    fwrite(w, strlen(w), 1, output);

    if (value_counter++ == 3) {
      value_counter = 0;
      column_count++;
    }

    if (column_count == max_columns) {
      // Next Row
      column_count = 0;
      fwrite("\n", sizeof(char), 1, output);
      row_count++;
    }

    if (row_count >= 720) {
      close_file(output, header, row_count, column_count);

      output_file_count++;
      output = get_output_file(argv[2], output_file_count);
      fseek(output, strlen(header), SEEK_CUR);
      row_count = 0;
    }
  }

  free(w);
  free(buffer);

  close_file(output, header, row_count, column_count);

  free(fp);
  free(header);

  return 0;
}
