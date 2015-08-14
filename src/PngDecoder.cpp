#include "PngDecoder.h"
#include "buffer.hpp"

#include <iostream>

#define PNG_HEADER_SIZE 8

// #define _PNGDECODER_DEBUG 1

void pngDecoderReadFunction(png_structp pngPtr, png_bytep data, png_size_t length) {

        png_voidp a = png_get_io_ptr(pngPtr);
        buffer *buf = (buffer *)a;

        memcpy ((char *)data, buf->data(), length);
        buf->drain(length);
}

PngDecoder::PngDecoder (buffer *inBuf)
{
        buf = new buffer(*inBuf);
}

PngDecoder::~PngDecoder ()
{
        if (buf)
                delete buf;
}

int PngDecoder::validate()
{
        if (png_sig_cmp((png_bytep)buf->data(), 0, PNG_HEADER_SIZE)) {
                 std::cout << "[PngDecoder] error: %s is not a PNG." << std::endl;
                return -1;
        }
        buf->drain(PNG_HEADER_SIZE);

        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
                std::cout << "[PngDecoder] error: png_create_read_struct returned 0." << std::endl;
                return -1;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
                std::cout << "[PngDecoder] error: png_create_info_struct returned 0." << std::endl;
                png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
                return -1;
        }

        end_info = png_create_info_struct(png_ptr);
        if (!end_info) {
                std::cout << "[PngDecoder] error: png_create_info_struct returned 0." << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
                return -1;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
                std::cout << "[PngDecoder] error from libpng" << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                return -1;
        }

        png_set_read_fn(png_ptr, buf, pngDecoderReadFunction);

        png_set_sig_bytes(png_ptr, PNG_HEADER_SIZE);

        png_read_info(png_ptr, info_ptr);

        png_set_expand(png_ptr);
        png_set_strip_16(png_ptr);
        png_set_gray_to_rgb(png_ptr);
        png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_set_interlace_handling(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        imgInfo.width    = png_get_image_width(png_ptr, info_ptr);
        imgInfo.height   = png_get_image_height(png_ptr, info_ptr);

        imgInfo.color_type = png_get_color_type(png_ptr, info_ptr);
        imgInfo.channels = png_get_channels(png_ptr, info_ptr);
        imgInfo.bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        imgInfo.interlance_type = png_get_interlace_type(png_ptr, info_ptr);
        imgInfo.rowbytes = png_get_rowbytes(png_ptr, info_ptr);

#ifdef _PNGDECODER_DEBUG
        std::cout << "[PngDecoder] Extracted w=" << imgInfo.width
                << " h="  << imgInfo.height
                << " color_type=" << imgInfo.color_type
                << " bit_depth="<< imgInfo.bit_depth
                << " rowbytes="<< imgInfo.rowbytes
                << " channels="<< imgInfo.channels
                << " interlance_type="<< imgInfo.interlance_type
                << std::endl;
#endif

        if (imgInfo.bit_depth != 8) {
                std::cout << "[PngDecoder] error: Unsupported bit depth=" << imgInfo.bit_depth <<". Must be 8." << std::endl;
                return -1;
        }

        switch(imgInfo.color_type) {
            case PNG_COLOR_TYPE_RGB:
                imgInfo.format = GL_RGB;
                break;
            case PNG_COLOR_TYPE_RGB_ALPHA:
                imgInfo.format = GL_RGBA;
                break;
            case PNG_COLOR_TYPE_PALETTE:
                if (imgInfo.color_type & PNG_COLOR_MASK_ALPHA) {
                        imgInfo.format = GL_RGBA;
                } else if (imgInfo.color_type & PNG_COLOR_MASK_COLOR) {
                        imgInfo.format = GL_RGB;
                }
                break;
            default:
                std::cout << "[PngDecoder] error: unknown libpng color type=" << imgInfo.color_type << " rgb=" << PNG_COLOR_TYPE_RGB << " rgba=" << PNG_COLOR_TYPE_RGBA << " palette=" << PNG_COLOR_TYPE_PALETTE << std::endl;
                return -1;
        }

        return 0;
}

ImgInfo PngDecoder::getImgInfo() const
{
        return imgInfo;
}

void PngDecoder::decodeIntoBuf(buffer *outBuf)
{
        int rowbytes = imgInfo.rowbytes;

        outBuf->size(rowbytes * imgInfo.height+15);
        png_byte * image_data = (png_byte *) outBuf->data();
        memset(image_data, 255, rowbytes*imgInfo.height);
        if (image_data == NULL) {
                std::cout << "[PngDecoder] error: could not allocate memory for PNG image data" << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                return;
        }

        png_byte ** row_pointers = new png_bytep[imgInfo.height];
        if (row_pointers == NULL) {
                std::cout << "[PngDecoder] error: could not allocate memory for PNG row pointers" << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                return;
        }

        for (unsigned int i = 0; i < imgInfo.height; i++) {
                row_pointers[i] = image_data + i * rowbytes;
        }

        png_read_image(png_ptr, row_pointers);

        png_read_end(png_ptr, info_ptr);

        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

        delete[] row_pointers;

        return;
}


