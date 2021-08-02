#pragma once

#include <stdint.h>

int decompress(const uint8_t* src, uint8_t* dst, uint32_t* src_size);
int get_decompressed_size(const uint8_t* src);
