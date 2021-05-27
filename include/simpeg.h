#ifndef _SIMPEG_H_
#define _SIMPEG_H_
#include <cstdio>
#include <jpeglib.h>
#include <jerror.h>

namespace simpeg {

struct JpegData {
    unsigned char *data;
    unsigned int width, height;
    unsigned int n_channels;
    JpegData() : data(nullptr), width(0), height(0), n_channels(0) { }
};

bool read_jpeg(JpegData *, const char *, jpeg_error_mgr *);
bool write_jpeg(const char *dst_file, unsigned char *image, unsigned int width, unsigned int height,
        bool isRGB = true, unsigned char quality = 90, bool downsample = false);

};

#endif
