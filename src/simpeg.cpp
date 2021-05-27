#include "simpeg.h"

namespace simpeg {

bool read_jpeg(JpegData * record, const char * src_file, jpeg_error_mgr *jpegError) {
    jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(jpegError);

    FILE *fp = fopen(src_file, "rb");
    if (fp == nullptr) {
        fprintf(stderr, "failed to open %s\n", src_file);
        return false;
    }

    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, true);

    jpeg_start_decompress(&cinfo);
    record->height = cinfo.image_height;
    record->width = cinfo.image_width;
    record->n_channels = cinfo.num_components;
    record->data = new unsigned char[sizeof(unsigned char) * record->height * record->width * record->n_channels];

    unsigned char *currentRow = record->data;
    const int rowStride = record->width * record->n_channels;
    for (int y = 0; y < record->height; ++y) {
        jpeg_read_scanlines(&cinfo, &currentRow, 1);
        currentRow += rowStride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    
    return true;
}

bool write_jpeg(const char *dst_file, unsigned char *image, unsigned int width, unsigned int height, 
        bool isRBG, unsigned char quanlity, bool dowmsample) {
    return true;
}

};
