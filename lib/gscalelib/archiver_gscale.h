#ifndef ARCHIVER_GSCALE_H
#define ARCHIVER_GSCALE_H

#include <cstddef>

namespace gscale {

struct RawImageData {
int width; // image width in pixels
int height; // image height in pixels
unsigned char * data; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.
};

struct InOutData {
    long unsigned int size; //in bits
    unsigned char * data;
};

std::size_t compress(RawImageData& /*in*/, const std::size_t /*in_size_data*/,InOutData& /*out*/);
std::size_t decompress(RawImageData& /*out*/, InOutData& /*in*/);
void freeOutputData(InOutData&);

};

#endif // ARCHIVER_GSCALE_H
