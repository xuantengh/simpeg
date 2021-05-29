#include "simpeg.h"
#include <cmath>
#include <functional>

namespace {
using uint8_t = unsigned char;
using uint16_t = unsigned short;
using int16_t = short;
using int32_t = int;

const uint8_t kDefaultQuantLuminance[8 * 8] =
    {16, 11, 10, 16, 24, 40, 51, 61,
     12, 12, 14, 19, 26, 58, 60, 55,
     14, 13, 16, 24, 40, 57, 69, 56,
     14, 17, 22, 29, 51, 87, 80, 62,
     18, 22, 37, 56, 68,109,103, 77,
     24, 35, 55, 64, 81,104,113, 92,
     49, 64, 78, 87,103,121,120,101,
     72, 92, 95, 98,112,100,103, 99};

const uint8_t kDefaultQuantChrominance[8 * 8] =
    {17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99};

const uint8_t kZigZagInv[8*8] =
    { 0, 1, 8,16, 9, 2, 3,10,
     17,24,32,25,18,11, 4, 5,
     12,19,26,33,40,48,41,34,
     27,20,13, 6, 7,14,21,28,
     35,42,49,56,57,50,43,36,
     29,22,15,23,30,37,44,51,
     58,59,52,45,38,31,39,46,
     53,60,61,54,47,55,62,63};

// Huffman definitions for first DC/AC tables (luminance / Y channel)
const uint8_t kDcLuminanceCodesPerBitsize[16]   = { 0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0 };   // sum = 12
const uint8_t kDcLuminanceValues         [12]   = { 0,1,2,3,4,5,6,7,8,9,10,11 };         // => 12 codes
const uint8_t kAcLuminanceCodesPerBitsize[16]   = { 0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125 }; // sum = 162
const uint8_t kAcLuminanceValues        [162]   =                                        // => 162 codes
    { 0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,
      0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
      0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
      0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
      0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,
      0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
      0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };
// Huffman definitions for second DC/AC tables (chrominance / Cb and Cr channels)
const uint8_t kDcChrominanceCodesPerBitsize[16] = { 0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0 };   // sum = 12
const uint8_t kDcChrominanceValues         [12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };         // => 12 codes (identical to DcLuminanceValues)
const uint8_t kAcChrominanceCodesPerBitsize[16] = { 0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119 }; // sum = 162
const uint8_t kAcChrominanceValues        [162] =                                        // => 162 codes
    { 0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
      0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,
      0x27,0x28,0x29,0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,
      0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,
      0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,
      0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,
      0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA };
const int16_t kCodeWordLimit = 2048; // +/-2^11, maximum value after DCT

struct BitCode {
    BitCode() = default;
    BitCode(uint16_t code_, uint8_t n_bits_) : code(code_), n_bits(n_bits_) { }
    uint16_t code;  // true value
    uint8_t n_bits; // number of valid bits
};

struct BitWriter {
    BitWriter() = delete;
    BitWriter(const BitWriter&) = delete;
    BitWriter(std::function<void(uint8_t)>& byte_process) : call_back(byte_process) { }

    std::function<void(uint8_t)> call_back;

    struct BitBuffer {
        int32_t data = 0;
        uint8_t n_bits = 0;
    } buffer;

    // put the incoming data into buffer, and output any if the buffer is full
    BitWriter& operator<<(const BitCode& data) {
        buffer.n_bits += data.n_bits;
        buffer.data <<= data.n_bits;
        buffer.data |= data.code;

        while (buffer.n_bits >= 8) {
            // extract and output higher 8 bits
            buffer.n_bits -= 8;
            uint8_t one_byte = buffer.data >> buffer.n_bits;
            call_back(one_byte);

            if (one_byte == 0xff) {
                call_back(0);
            }
        }

        return *this;
    }

    // output all non-yet out bytes, fill gaps with 7 one bits
    void flush() {
        *this << BitCode(0x7f, 7);
    }

    // output a single byte immediately, and bypass the buffer
    BitWriter& operator<<(uint8_t one_byte) {
        call_back(one_byte);
        return *this;
    }

    // output an array of bytes directly
    template<typename T, int Size>
    BitWriter& operator<<(T (&many_bytes)[Size]) {
        for (auto& byte: many_bytes) {
            call_back(byte);
        }
        return *this;
    }

    // start a new JFIF block
    void add_marker(uint8_t id, uint16_t length) {
        call_back(0xff); call_back(id);
        // output length, big-endian
        call_back(uint8_t(length >> 8));
        call_back(uint8_t(length & 0xff));
    }
};  // end of BitWriter struct

// same as to np.clip
template<typename Number, typename Limit>
Number clip(Number value, Limit min, Limit max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// convert RGB pixel value to YCbCr, details are on https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
float rgb2Y(float r, float g, float b) { return 0.229f * r + 0.587f * g + 0.114f * b; }
float rgb2Cb(float r, float g, float b) { return -0.16874f * r - 0.33126f * g + 0.5f * b; }
float rgb2Cr(float r, float g, float b) { return 0.5f * r - 0.41869f * g - 0.08131f * b; }

void DCT(float block[8 * 8], uint8_t stride) {
    static const float kSqrtHalfSqrt = 1.306562965f;        // sqrt((2 + sqrt(2)) / 2) = cos(pi * 1 / 8) * sqrt(2)
    static const float kInverseSqrt = 0.707106781f;         // 1 / sqrt(2) = cos(pi * 2 / 8)
    static const float kHalfSqrtSqrt = 0.382683432f;        // sqrt(2 - sqrt(2)) / 2 = cos(pi * 3 / 8)
    static const float kInverseSqrtSqrt = 0.541196100f;     // 1 / sqrt(2 - sqrt(2)) = cos(pi * 3 / 8) * sqrt(2)

    auto& block0 = block[0];
    auto& block1 = block[1 * stride];
    auto& block2 = block[2 * stride];
    auto& block3 = block[3 * stride];
    auto& block4 = block[4 * stride];
    auto& block5 = block[5 * stride];
    auto& block6 = block[6 * stride];
    auto& block7 = block[7 * stride];

    auto add07 = block0 + block7; auto sub07 = block0 - block7;
    auto add16 = block1 + block6; auto sub16 = block1 - block6;
    auto add25 = block2 + block5; auto sub25 = block2 - block5;
    auto add34 = block3 + block4; auto sub34 = block3 - block4;

    auto add0347 = add07 + add34; auto sub07_34 = add07 - add34;
    auto add1256 = add16 + add25; auto sub16_25 = add16 - add25;

    block0 = add0347 + add1256; block4 = add0347 - add1256; // "phase 3"

    auto z1 = (sub16_25 + sub07_34) * kInverseSqrt;
    block2 = sub07_34 + z1; block6 = sub07_34 - z1;

    auto sub23_45 = sub25 + sub34;
    auto sub12_56 = sub16 + sub25;
    auto sub01_67 = sub16 + sub07;

    auto z5 = (sub23_45 - sub01_67) * kHalfSqrtSqrt;
    auto z2 = sub23_45 * kInverseSqrtSqrt + z5;
    auto z3 = sub12_56 * kInverseSqrt;
    auto z4 = sub01_67 * kSqrtHalfSqrt + z5;
    auto z6 = sub07 + z3;
    auto z7 = sub07 - z3;
    block1 = z6 + z4; block7 = z6 - z4;
    block5 = z7 + z2; block3 = z7 - z2;
}

// run DCT, quantization and write Huffman bit codes
int16_t encode_block(BitWriter& writer, float block[8][8], const float scaled[8 * 8], int16_t last_dc,
        const BitCode huffman_dc[256], const BitCode huffman_ac[256], const BitCode *codewords) {
    auto block64 = (float*)block; // linearize 2d block
    // DCT rows
    for (int offset = 0; offset < 8; ++offset) {
        DCT(block64 + offset * 8, 1);
    }
    // DCT column
    for (int offset = 0; offset < 8; ++offset) {
        DCT(block64 + offset * 1, 8);
    }

    // scale
    for (int i = 0; i < 8 * 8; ++i) {
        block64[i] *= scaled[i];
    }

    // encode dc
    int dc = int(block64[0] + (block64[0] >= 0 ? +0.5f : -0.5f));
    // encode ac components
    int last_non_zero_pos = 0;
    int16_t quantized[8 * 8];
    for (int i = 1; i < 8 * 8; ++i) {
        // find value on the current position of zigzag sequence
        float value = block64[kZigZagInv[i]];
        quantized[i] = int(value + (value >= 0 ? +0.5f : -0.5f));
        // update last_non_zero_pos
        if (quantized[i] != 0) {
            last_non_zero_pos = i;
        }
    }

    int dc_diff = dc - last_dc;
    if (dc_diff == 0) {
        writer << huffman_dc[0x00];
    } else {
        BitCode bits = codewords[dc_diff];
        writer << huffman_dc[bits.n_bits] << bits;
    }

    int offset = 0;  // the upper 4 bits are used as a counter of consecutive zero bits
    for (int i = 1; i <= last_non_zero_pos; ++i) {
        // zeros are encoded in a special way
        while (quantized[i] == 0) {
            offset += 0x10;
            if (offset > 0xf0) {
                writer << huffman_ac[0xf0];
                offset = 0;
            }
            i++;
        }

        // non-zero bit
        BitCode bits = codewords[quantized[i]];
        writer << huffman_ac[offset + bits.n_bits] << bits;
        offset = 0;
    }

    if (last_non_zero_pos < 8 * 8 - 1) {  // 63
        // add end-of-block if there are trailing zeros of a block
        writer << huffman_ac[0x00];
    }

    return dc;
} 


void generate_huffman_table(const uint8_t n_codes[16], const uint8_t *values, BitCode result[256]) {
    int huffman_code = 0;
    for (int n_bits = 1; n_bits <= 16; ++n_bits) {
        for (int i = 0; i < n_codes[n_bits - 1]; ++i) {
            result[*values++] = BitCode(huffman_code++, n_bits);
        }

        huffman_code <<= 1;
    }
}

};  // end of anonymous namespace


namespace simpeg {

bool read_jpeg(JpegData * record, const char * src_file, jpeg_error_mgr *jpeg_error) {
    jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(jpeg_error);
    // open source file in binary
    FILE *fp = fopen(src_file, "rb");
    if (fp == nullptr) {
        fprintf(stderr, "failed to open %s\n", src_file);
        return false;
    }
    // specify data source
    jpeg_stdio_src(&cinfo, fp);
    // read jpeg header
    jpeg_read_header(&cinfo, true);
    // jpeg information
    jpeg_start_decompress(&cinfo);
    record->height = cinfo.image_height;
    record->width = cinfo.image_width;
    record->n_channels = cinfo.num_components;
    record->data = new unsigned char[sizeof(unsigned char) * record->height * record->width * record->n_channels];
    // read RGB values row by row
    unsigned char *current_row = record->data;
    const int row_stride = record->width * record->n_channels;
    for (int y = 0; y < record->height; ++y) {
        jpeg_read_scanlines(&cinfo, &current_row, 1);
        current_row += row_stride;
    }
    // free resources
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return true;
}

int write_jpeg(const char *dst_file, const void *image, unsigned short width, unsigned short height,
        bool isRGB, unsigned char output_quanlity) {
    // reject invalid arguments
    if (dst_file == nullptr || image == nullptr || width * height == 0) {
        return -1;
    }
    // color or gray-scale image
    const int n_components = isRGB == true ? 3 : 1;

    // create output binary stream
    int writen_counter = 0;
    std::ofstream out_stream(dst_file, std::ios_base::out | std::ios_base::binary);
    std::function<void(uint8_t)> byte_writer = [&out_stream, &writen_counter](uint8_t c) {
        out_stream << c; 
        writen_counter++;
    };
    ::BitWriter bit_writer(byte_writer);

    // JFIF headers
    const uint8_t JFIF_header[2 + 2 + 16] = {
        0xFF, 0xD8,             // SOI marker (start of image)
        0xFF, 0xE0,             // JFIF APP0 tag
        0, 16,                  // length: 16 bytes (14 bytes payload + 2 bytes for this length field)
        'J', 'F', 'I', 'F', 0,  // JFIF identifier, zero-terminated
        1, 1,                   // JFIF version 1.1
        0,                      // no density units specified
        0, 1, 0, 1,             // density: 1 pixel "per pixel" horizontally and vertically
        0, 0 };

    bit_writer << JFIF_header;

    // clamp quality and get quality factor
    uint16_t quality = ::clip<uint16_t>(output_quanlity, 1, 100);
    quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

    uint8_t luminance_quan_table[8 * 8];
    uint8_t chrominance_quan_table[8 * 8];
    for (int i = 0; i < 8 * 8; ++i) {
        int l = (kDefaultQuantLuminance[kZigZagInv[i]] * quality + 50) / 100;
        int c = (kDefaultQuantChrominance[kZigZagInv[i]] * quality + 50) / 100;

        luminance_quan_table[i] = ::clip(l, 1, 255);
        chrominance_quan_table[i] = ::clip(c, 1, 255);
    }

    // write quantization table(s)
    bit_writer.add_marker(0xdb, 2 + (isRGB ? 2 : 1) * (1 + 8 * 8));
    bit_writer << 0x00 << luminance_quan_table;
    if (isRGB == true) {
        bit_writer << 0x01 << chrominance_quan_table;
    }

    // write image information, start of frame
    bit_writer.add_marker(0xc0, 2 + 6 + 3 * n_components);  // 6 bytes for general, 3 bytes per channel

    // 8 bits per channel
    bit_writer << 0x08 << (height >> 8) << (height & 0xff) << (width >> 8) << (width & 0xff);

    // sampling and quantization tables for each components
    bit_writer << n_components;
    for (uint8_t i = 1; i <= n_components; ++i) {
        // component id, 0x11 for YCbCr 4:4:4, quantization table id
        bit_writer << i << 0x11 << (i == 1 ? 0 : 1);
    }

    // Huffman table
    bit_writer.add_marker(0xc4, isRGB ? (2 + 208 + 208) : (2 + 208));  // 208 = (1 + 16 + 12) + (1 + 16 + 162)
    bit_writer << 0x00 << kDcLuminanceCodesPerBitsize << kDcLuminanceValues;
    bit_writer << 0x10 << kAcLuminanceCodesPerBitsize << kAcLuminanceValues;

    // Huffman code tables, luminance and chrominance if necessary
    BitCode huffman_dc_luminance[256];
    BitCode huffman_ac_luminance[256];
    generate_huffman_table(kDcLuminanceCodesPerBitsize, kDcLuminanceValues, huffman_dc_luminance);
    generate_huffman_table(kAcLuminanceCodesPerBitsize, kAcLuminanceValues, huffman_ac_luminance);

    BitCode huffman_dc_chromimance[256];
    BitCode huffman_ac_chromimance[256];
    if (isRGB == true) {
        bit_writer << 0x01 << kDcChrominanceCodesPerBitsize << kDcChrominanceValues;
        bit_writer << 0x11 << kAcChrominanceCodesPerBitsize << kAcChrominanceValues;

        generate_huffman_table(kDcChrominanceCodesPerBitsize, kDcChrominanceValues, huffman_dc_chromimance);
        generate_huffman_table(kAcChrominanceCodesPerBitsize, kAcChrominanceValues, huffman_ac_chromimance);
    }

    // start of scan, 2 bytes for length field, 1 byte for number of components,
    // 2 bytes for each component and 3 bytes for spectral selection
    bit_writer.add_marker(0xda, 2 + 1 + 2 * n_components + 3);

    bit_writer << n_components;
    for (uint8_t i = 1; i <= n_components; ++i) {
        // higher 4 bits: DC Huffman table, lower 4 bits: AC Huffman table
        // Y: tables 0 for DC and AC, Cb & Cr: tables 1 for DC and AC
        bit_writer << i << (i == 1 ? 0x00 : 0x11);
    }

    // standard for JPEG
    static const uint8_t kSpectral[3] = {0, 63, 0};
    bit_writer << kSpectral;

    // AAN DCT scaling factors
    // a[0] = 1, a[k] = cos(k * pi / 16) * sqrt(2)
    static const float kAANScaleFactors[8] = {1, 1.387039845f, 1.306562965f, 1.175875602f, 1, 0.785694958f, 0.541196100f, 0.275899379f};
    float scaled_luminance[8 * 8];
    float scaled_chromimance[8 * 8];
    for (int i = 0; i < 8 * 8; ++i) {
        int row = kZigZagInv[i] / 8, col = kZigZagInv[i] % 8;

        float factor = 1 / (kAANScaleFactors[row] * kAANScaleFactors[col] * 8);
        scaled_luminance[kZigZagInv[i]] = factor / luminance_quan_table[i];
        scaled_chromimance[kZigZagInv[i]] = factor / chrominance_quan_table[i];
    }

    // precompute JPEG codewords for quantized DCT
    BitCode codewords_array[2 * kCodeWordLimit];
    BitCode *codewords = codewords_array + kCodeWordLimit;
    uint8_t n_bits = 1;
    int32_t mask = 1;
    for (uint16_t value = 1; value < kCodeWordLimit; ++value) {
        // mask = (2 ^ n_bits) - 1
        if (value > mask) {
            n_bits++;
            mask = (mask << 1) | 1;  // append one more 1
        }

        codewords[-value] = BitCode(mask - value, n_bits);
        codewords[value] = BitCode(value, n_bits);
    }

    auto pixels = static_cast<const uint8_t *>(image);

    const int max_height = height - 1, max_width = width - 1;
    const int mcu_size = 8;

    uint16_t last_Y_dc = 0, last_Cb_dc = 0, last_Cr_dc = 0;
    // convert RGB to YCbCr
    float Y[8][8], Cb[8][8], Cr[8][8];

    // start encoding mcu blocks
    for (int mcu_y = 0; mcu_y < height; mcu_y += mcu_size) {
        for (int mcu_x = 0; mcu_x < width; mcu_x += mcu_size) {
            for (int dy = 0; dy < 8; dy++) {
                int col = std::min(mcu_x , width - 1); // must not exceed image borders, replicate last row/column if needed
                int row = std::min(mcu_y + dy, height - 1);
                for (auto dx = 0; dx < 8; dx++) {
                    // find actual pixel position within the current image
                    int pixel_pos = row * int(width) + col; // the cast ensures that we don't run into multiplication overflows
                    if (col < width - 1) {
                        col++;
                    }

                    // grayscale images have solely a Y channel which can be easily derived from the input pixel by shifting it by 128
                    if (!isRGB) {
                        Y[dy][dx] = pixels[pixel_pos] - 128.f;
                        continue;
                    }

                    // RGB: 3 bytes per pixel (whereas grayscale images have only 1 byte per pixel)
                    uint8_t r = pixels[3 * pixel_pos];
                    uint8_t g = pixels[3 * pixel_pos + 1];
                    uint8_t b = pixels[3 * pixel_pos + 2];

                    Y[dy][dx] = rgb2Y(r, g, b) - 128.f;
                    Cb[dy][dx] = rgb2Cb(r, g, b); // standard RGB-to-YCbCr conversion
                    Cr[dy][dx] = rgb2Cr(r, g, b);
                }
            }

            // encode Y channel
            last_Y_dc = encode_block(bit_writer, Y, scaled_luminance, last_Y_dc, huffman_dc_luminance, huffman_ac_luminance, codewords);

            // grayscale images don't need any Cb and Cr information
            if (!isRGB) { 
                continue;
            }
      
            // encode Cb and Cr block
            last_Cb_dc = encode_block(bit_writer, Cb, scaled_chromimance, last_Cb_dc, huffman_dc_chromimance, huffman_ac_chromimance, codewords);
            last_Cr_dc = encode_block(bit_writer, Cr, scaled_chromimance, last_Cr_dc, huffman_dc_chromimance, huffman_ac_chromimance, codewords);
        }
    }

    // output all bits in buffer
    bit_writer.flush();

    // end of image marker
    bit_writer << 0xff << 0xd9;

    return writen_counter;
}

};  // end of simpeg namespace

