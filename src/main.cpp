#include "simpeg.h"
#include <cstdio>
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "command format: ./simpeg input.jpeg output.jpeg \n");
        exit(1);
    }
    const char *src_file = argv[1];
    const char *dst_file = argv[2];
    simpeg::JpegData record;
    jpeg_error_mgr jpeg_error;
    if (!simpeg::read_jpeg(&record, src_file, &jpeg_error)) {
        exit(1);
    }
    printf("input image size: (%d, %d, %d)\n", record.height, record.width, record.n_channels);
    bool isRGB = record.n_channels == 3;
    int n_bytes = simpeg::write_jpeg(dst_file, record.data, record.width, record.height, isRGB);
    if (n_bytes != -1) {
        printf("jpeg data has been successfully written into %s with %d bytes \n", dst_file, n_bytes);
    } else {
        fprintf(stderr, "failed...\n");
        exit(1);
    }
    return 0;
}
