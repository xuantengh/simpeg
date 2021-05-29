#include "simpeg.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
    // receive and process command line options
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "command format: ./simpeg input.jpg output.jpg (quality)\n");
        exit(1);
    }
    const char *src_file = argv[1];
    const char *dst_file = argv[2];
    unsigned char quality = 90;
    if (argc == 4) {
        quality = static_cast<unsigned char>(atoi(argv[3]));
    }
    // load jpg file
    simpeg::JpegData record;
    jpeg_error_mgr jpeg_error;
    if (!simpeg::read_jpeg(&record, src_file, &jpeg_error)) {
        exit(1);
    }
    printf("input image size: (%d, %d, %d)\n", record.height, record.width, record.n_channels);
    printf("compress image with quality = %u\n", quality);
    // write jpg file
    bool isRGB = record.n_channels == 3;
    int n_bytes = simpeg::write_jpeg(dst_file, record.data, record.width, record.height, isRGB, quality);

    if (n_bytes != -1) {
        printf("jpeg data has been successfully written into %s with %d bytes \n", dst_file, n_bytes);
    } else {
        fprintf(stderr, "failed...\n");
        exit(1);
    }

    return 0;
}
