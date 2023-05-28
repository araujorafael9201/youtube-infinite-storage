#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VIDEO_WIDTH 1280
#define VIDEO_HEIGHT 720
#define FRAMERATE 15

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <input file> <output file (without extension)>\n",
           argv[0]);
    return 1;
  }

  // Create fork proccess to run ffmpeg + pipe to send information

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    printf("Error: Could not create pipe");
    return EXIT_FAILURE;
  }

  pid_t pid = fork();

  if (pid == -1) {
    printf("Error: Could not create child proccess!");
    return EXIT_FAILURE;
  }

  ssize_t bytes_read;

  if (pid == 0) {
    // Parent proccess
    // Read file + Send bytes to child
    close(pipefd[0]);

    FILE *fd = fopen(argv[1], "rb");
    if (fd == NULL) {
      printf("Error: cannot open file %s\n", argv[1]);
      return 1;
    }

    uint32_t frame[VIDEO_WIDTH * VIDEO_HEIGHT];
    while (!feof(fd)) {
      bytes_read = fread(frame, sizeof(frame), 1, fd);
      write(pipefd[1], frame, sizeof(frame));
    }

    close(pipefd[0]);
    fclose(fd);
  } else {
    // Child proccess
    // Receive bytes + run ffmpeg to create video
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    char *output_file = malloc(32);
    sprintf(output_file, "%s.mp4", argv[2]);

    char *resolution = malloc(15);
    sprintf(resolution, "%dx%d", VIDEO_WIDTH, VIDEO_HEIGHT);

    char *framerate_str = malloc(5);
    sprintf(framerate_str, "%d", FRAMERATE);

    int r;
    if ((r = execlp("ffmpeg", "-y", "-f", "rawvideo", "-pix_fmt", "rgb32", "-s",
                    resolution, "-r", framerate_str, "-i", "-", "-c:v",
                    "libx264", output_file, NULL) == -1)) {
      printf("Error: Could not execute ffmpeg\n");
      return EXIT_FAILURE;
    }

    close(pipefd[0]);
  }

  return 0;
}
