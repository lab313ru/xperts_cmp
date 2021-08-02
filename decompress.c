#include <stdint.h>
#include <string.h>

static const uint16_t masks[] = {
  0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
};

typedef struct item_data_t {
  uint16_t w0;
  uint16_t w2;
} item_data_t;

typedef struct item_t {
  uint16_t index;
  item_data_t items[9];
} item_t;

static const item_t table[] = {
  {0x0000, { {0x0001, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0001, { {0x0005, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0001, { {0x0009, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0002, { {0x001D, 0x0004}, {0x000E, 0x0003}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0002, { {0x002D, 0x0005}, {0x000E, 0x0003}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0002, { {0x0059, 0x0006}, {0x001A, 0x0004}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0002, { {0x00A9, 0x0007}, {0x002A, 0x0005}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000}, {0x0000, 0x0000} }},
  {0x0003, { {0x01FD, 0x0008}, {0x00FE, 0x0007}, {0x007E, 0x0006}, {0x003E, 0x0005}, {0x001E, 0x0004}, {0x000E, 0x0003}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x02FD, 0x0009}, {0x00FE, 0x0007}, {0x007E, 0x0006}, {0x003E, 0x0005}, {0x001E, 0x0004}, {0x000E, 0x0003}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x05ED, 0x000A}, {0x01EE, 0x0008}, {0x00EE, 0x0007}, {0x006E, 0x0006}, {0x002E, 0x0005}, {0x000E, 0x0003}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x0B75, 0x000B}, {0x0376, 0x0009}, {0x0176, 0x0008}, {0x0076, 0x0006}, {0x0036, 0x0005}, {0x0016, 0x0004}, {0x0006, 0x0002}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x16D9, 0x000C}, {0x06DA, 0x000A}, {0x02DA, 0x0009}, {0x00DA, 0x0007}, {0x005A, 0x0006}, {0x001A, 0x0004}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x2B59, 0x000D}, {0x0B5A, 0x000B}, {0x035A, 0x0009}, {0x015A, 0x0008}, {0x005A, 0x0006}, {0x001A, 0x0004}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0x55A9, 0x000E}, {0x15AA, 0x000C}, {0x05AA, 0x000A}, {0x01AA, 0x0008}, {0x00AA, 0x0007}, {0x002A, 0x0005}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000} }},
  {0x0003, { {0xAAA9, 0x000F}, {0x2AAA, 0x000D}, {0x0AAA, 0x000B}, {0x02AA, 0x0009}, {0x00AA, 0x0007}, {0x002A, 0x0005}, {0x000A, 0x0003}, {0x0002, 0x0001}, {0x0001, 0x0000} }}
};

static uint8_t read_byte(const uint8_t* src, int offset) {
  return src[offset];
}

static uint16_t read_word(const uint8_t* src, int offset) {
  uint8_t b1 = read_byte(src, offset++);
  uint8_t b2 = read_byte(src, offset);
  return (b1 << 8) | (b2 << 0);
}

static uint32_t read_dword(const uint8_t* src, int offset) {
  uint16_t w1 = read_word(src, offset); offset += 2;
  uint16_t w2 = read_word(src, offset);
  return (w1 << 16) | (w2 << 0);
}

static void write_byte(uint8_t* dst, int* offset, uint8_t value) {
  dst[*offset] = value;
  *offset += 1;
}

static void write_word(uint8_t* dst, int* offset, uint16_t value) {
  write_byte(dst, offset, (value >> 8) & 0xFF);
  write_byte(dst, offset, (value >> 0) & 0xFF);
}

static int getbit(const uint8_t* src, int* roff, int* bits, uint32_t* token) {
  *bits -= 1;

  if (*bits < 0) {
    *token = read_dword(src, *roff); *roff += 4;
    *bits = 0x1F;
  }

  return ((*token) & (1 << (*bits))) ? 1 : 0;
}

static uint32_t getbits(const uint8_t* src, int* roff, int count, int* bits, uint32_t* token) {
  uint32_t result = 0;

  for (int i = 0; i < count; ++i) {
    result = (result << 1) | getbit(src, roff, bits, token);
  }

  return result;
}

static uint16_t read_token(const uint8_t* src, int* roff, uint16_t value, int* bits, uint32_t* token) {
  const item_t* tbl = NULL;

  for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
    tbl = &table[i];

    if (tbl->items[0].w0 >= value) {
      break;
    }
  }

  if (tbl->index == 0) {
    return getbits(src, roff, tbl->items[0].w2, bits, token);
  }

  uint16_t v1 = getbits(src, roff, tbl->index, bits, token);

  if (v1 < masks[tbl->index]) {
    return tbl->items[v1 + 1].w0 + getbits(src, roff, tbl->items[v1].w2, bits, token);
  }

  return getbits(src, roff, tbl->items[v1].w2, bits, token);
}

static uint16_t read_count(const uint8_t* src, int* roff, int* bits, uint32_t* token) {
  uint16_t value = 0;

  while (!getbit(src, roff, bits, token)) {
    value += 1;
  }

  return value;
}

uint32_t get_decompressed_size(const uint8_t* src) {
  return read_dword(src, 0);
}

int decompress(const uint8_t* src, uint8_t* dst, uint32_t* src_size) {
  int roff = 0;
  int woff = 0;

  uint32_t left = read_dword(src, roff); roff += 4;
  uint32_t data_off = read_dword(src, roff) + 8; roff += 4;

  uint16_t max_from = read_word(src, roff); roff += 2;
  uint16_t max_count = read_word(src, roff); roff += 2;
  uint32_t token = read_dword(src, roff); roff += 4;

  int bits = 0x20;

  int word_mode = getbit(src, &roff, &bits, &token);
  left >>= word_mode ? 1 : 0;

  int unp_count = -1 - (word_mode ? 0 : 1);

  while (left) {
    uint16_t count = read_count(src, &roff, &bits, &token) + 1;

    int stop = count > left;
    left -= count;

    if (stop) {
      return -1;
    }

    unp_count += count;

    for (uint16_t i = 0; i < count; ++i) {
      if (word_mode) {
        uint16_t w0 = read_word(src, data_off);
        write_word(dst, &woff, w0);
      }
      else {
        uint8_t b0 = read_byte(src, data_off);
        write_byte(dst, &woff, b0);
      }

      data_off += word_mode ? 2 : 1;
    }

    if (left == 0) {
      break;
    }

    uint16_t pairs = read_count(src, &roff, &bits, &token) + 1;

    for (uint16_t i = 0; i < pairs; ++i) {
      uint16_t token_val = max_from;

      if (max_from > unp_count) {
        token_val = unp_count;
      }

      uint16_t from = read_token(src, &roff, token_val, &bits, &token);

      token_val = max_count;

      if (max_count > from) {
        token_val = from;
      }

      uint16_t count = read_token(src, &roff, token_val, &bits, &token) + 1 + (word_mode ? 0 : 1);

      stop = count > left;
      left -= count;

      if (stop) {
        return -1;
      }

      unp_count += count;
      int curr = woff - 2 - from * (word_mode ? 2 : 1);

      for (uint16_t j = 0; j < count; ++j) {
        if (word_mode) {
          uint16_t w0 = read_word(dst, curr + j * 2);
          write_word(dst, &woff, w0);
        }
        else {
          uint8_t b0 = read_byte(dst, curr + j);
          write_byte(dst, &woff, b0);
        }
      }
    }
  }

  *src_size = data_off;

  return woff;
}
