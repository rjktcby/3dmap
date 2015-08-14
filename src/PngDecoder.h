#ifndef PNGDECODER_H_INCLUDED
#define PNGDECODER_H_INCLUDED

#include <png.h>
#include "buffer.hpp"
#include "types.h"

class PngDecoder
{
        buffer *buf;
        ImgInfo imgInfo;
        png_structp png_ptr;
        png_infop info_ptr, end_info;

public:
        PngDecoder (buffer *inBuf);
        ~PngDecoder ();
        ImgInfo getImgInfo() const;
        int validate();
        void decodeIntoBuf(buffer *outBuf);
};

#endif //PNGDECODER_H_INCLUDED