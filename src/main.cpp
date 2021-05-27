#include "simpeg.h"
#include <cstdio>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "command format: ./simpeg input.jpeg \n");
        exit(1);
    }
    const char *src_file = argv[1];
    simpeg::JpegData record;
    jpeg_error_mgr jpegError;
    if (!simpeg::read_jpeg(&record, src_file, &jpegError)) {
        exit(1);
    }
    printf("input image size: (%d, %d, %d)\n", record.height, record.width, record.n_channels);
    return 0;
}
