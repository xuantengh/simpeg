#ifndef _SIMPEG_H_
#define _SIMPEG_H_
#include <cstdio>
#include <fstream>
#include <jpeglib.h>
#include <jerror.h>

namespace simpeg {

struct JpegData {
    unsigned char *data;
    unsigned int width, height;
    unsigned int n_channels;
    JpegData() : data(nullptr), width(0), height(0), n_channels(0) { }
    ~JpegData() {
        if (data != nullptr) {
            delete[] data;
        }
    }
};

bool read_jpeg(JpegData *, const char *, jpeg_error_mgr *);
bool write_jpeg(const char *dst_file, const unsigned char *image, unsigned int width, unsigned int height,
        bool isRGB = true, unsigned char output_quality = 90);

};  // namespace simpeg

#endif
