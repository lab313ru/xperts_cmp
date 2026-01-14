#include "main.h"

#include <stdlib.h>
#include <string.h>

static void write_byte(uint8_t* dst, int* offset, uint8_t value) {
  dst[*offset] = value;
  *offset += 1;
}

static void write_word_be(uint8_t* dst, int* offset, uint16_t value) {
  write_byte(dst, offset, (uint8_t)((value >> 8) & 0xFF));
  write_byte(dst, offset, (uint8_t)((value >> 0) & 0xFF));
}

static void write_dword_be(uint8_t* dst, int* offset, uint32_t value) {
  write_word_be(dst, offset, (uint16_t)((value >> 16) & 0xFFFF));
  write_word_be(dst, offset, (uint16_t)((value >> 0) & 0xFFFF));
}

typedef struct bitwriter_t {
  uint8_t* dst;
  int* woff;
  uint32_t token;
  int bits_used;
  int token_pos;
} bitwriter_t;

static void bw_init(bitwriter_t* bw, uint8_t* dst, int* woff) {
  bw->dst = dst;
  bw->woff = woff;
  bw->token = 0;
  bw->bits_used = 0;
  bw->token_pos = *woff;
  write_dword_be(dst, woff, 0);
}

static void bw_flush_token(bitwriter_t* bw) {
  int tmp = bw->token_pos;
  write_dword_be(bw->dst, &tmp, bw->token);
  bw->token = 0;
  bw->bits_used = 0;
  bw->token_pos = *bw->woff;
  write_dword_be(bw->dst, bw->woff, 0);
}

static void bw_putbit(bitwriter_t* bw, int bit) {
  if (bw->bits_used == 32) {
    bw_flush_token(bw);
  }

  int pos = 31 - bw->bits_used;
  if (bit != 0) {
    bw->token |= (1u << pos);
  }
  bw->bits_used += 1;
}

static void bw_putbits(bitwriter_t* bw, uint32_t value, int count) {
  for (int i = count - 1; i >= 0; --i) {
    bw_putbit(bw, (int)((value >> i) & 1u));
  }
}

static void bw_finish(bitwriter_t* bw) {
  int tmp = bw->token_pos;
  write_dword_be(bw->dst, &tmp, bw->token);
}

static void write_count(bitwriter_t* bw, uint32_t zeros_before_one) {
  for (uint32_t i = 0; i < zeros_before_one; ++i) {
    bw_putbit(bw, 0);
  }
  bw_putbit(bw, 1);
}

static const item_t* find_tbl_for_value(uint16_t max_value) {
  int n = (int)(sizeof(table) / sizeof(table[0]));
  for (int i = 0; i < n; ++i) {
    if (table[i].items[0].w0 >= max_value) {
      return &table[i];
    }
  }
  return &table[n - 1];
}

static int write_token(bitwriter_t* bw, uint16_t max_value, uint16_t x) {
  const item_t* tbl = find_tbl_for_value(max_value);

  if (tbl->index == 0) {
    uint16_t nbits = tbl->items[0].w2;
    if (nbits == 0) {
      if (x != 0) {
        return -1;
      }
      return 0;
    }
    if ((uint32_t)x >= (1u << nbits)) {
      return -1;
    }
    bw_putbits(bw, x, nbits);
    return 0;
  }

  for (uint16_t v1 = 0; v1 < masks[tbl->index]; ++v1) {
    uint16_t base = tbl->items[v1 + 1].w0;
    uint16_t nbits = tbl->items[v1].w2;
    uint32_t max_extra = (nbits == 0) ? 0u : ((1u << nbits) - 1u);

    if (x >= base && (uint32_t)(x - base) <= max_extra) {
      bw_putbits(bw, v1, tbl->index);
      if (nbits != 0) {
        bw_putbits(bw, (uint32_t)(x - base), nbits);
      }
      return 0;
    }
  }

  for (uint16_t v1 = masks[tbl->index]; v1 < (1u << tbl->index); ++v1) {
    uint16_t nbits = tbl->items[v1].w2;
    uint32_t maxv = (nbits == 0) ? 0u : ((1u << nbits) - 1u);

    if ((uint32_t)x <= maxv) {
      bw_putbits(bw, v1, tbl->index);
      if (nbits != 0) {
        bw_putbits(bw, x, nbits);
      }
      return 0;
    }
  }

  return -1;
}

typedef struct match_t {
  uint16_t from;
  uint16_t len;
} match_t;

static int elements_equal(const uint8_t* a, const uint8_t* b, int word_mode) {
  if (word_mode != 0) {
    return (a[0] == b[0]) && (a[1] == b[1]);
  }
  return (a[0] == b[0]);
}

static int token_bit_cost(uint16_t max_value, uint16_t x) {
  const item_t* tbl = find_tbl_for_value(max_value);

  if (tbl->index == 0) {
    uint16_t nbits = tbl->items[0].w2;
    if (nbits == 0) return (x == 0) ? 0 : -1;
    if ((uint32_t)x >= (1u << nbits)) return -1;
    return (int)nbits;
  }

  for (uint16_t v1 = 0; v1 < masks[tbl->index]; ++v1) {
    uint16_t base = tbl->items[v1 + 1].w0;
    uint16_t nbits = tbl->items[v1].w2;
    uint32_t max_extra = (nbits == 0) ? 0u : ((1u << nbits) - 1u);
    if (x >= base && (uint32_t)(x - base) <= max_extra) {
      return (int)tbl->index + (int)nbits;
    }
  }

  for (uint16_t v1 = masks[tbl->index]; v1 < (1u << tbl->index); ++v1) {
    uint16_t nbits = tbl->items[v1].w2;
    uint32_t maxv = (nbits == 0) ? 0u : ((1u << nbits) - 1u);
    if ((uint32_t)x <= maxv) {
      return (int)tbl->index + (int)nbits;
    }
  }

  return -1;
}

static int pair_bit_cost(uint16_t max_from, uint16_t max_count, int word_mode, int unp_count, uint16_t from, uint16_t len) {
  uint16_t up = (unp_count < 0) ? 0u : (uint16_t)unp_count;

  uint16_t token_val_from = max_from;
  if (token_val_from > up) {
    token_val_from = up;
  }
  if (from > token_val_from) {
    return -1;
  }

  uint16_t token_val_cnt = max_count;
  if (token_val_cnt > from) {
    token_val_cnt = from;
  }

  uint16_t extra = (word_mode != 0) ? 0u : 1u;
  uint16_t min_len = (word_mode != 0) ? 1u : 2u;

  if (len < min_len) {
    return -1;
  }

  if (len <= (uint16_t)(1u + extra)) {
    return -1;
  }

  uint16_t count_token = (uint16_t)(len - 1u - extra);

  int c1 = token_bit_cost(token_val_from, from);
  int c2 = token_bit_cost(token_val_cnt, count_token);

  if (c1 < 0 || c2 < 0) {
    return -1;
  }

  return c1 + c2;
}

static match_t find_best_match_cost(const uint8_t* in, uint32_t pos, uint32_t total_elems, uint16_t max_from, uint16_t max_count, int word_mode, int unp_count) {
  match_t best = { 0, 0 };

  uint32_t stride = (word_mode != 0) ? 2u : 1u;
  uint32_t base = (word_mode != 0) ? 1u : 2u;
  uint32_t minlen = (word_mode != 0) ? 1u : 2u;

  if (pos < base) {
    return best;
  }

  uint16_t up = (unp_count < 0) ? 0u : (uint16_t)unp_count;
  uint16_t token_val_from = max_from;

  if (token_val_from > up) {
    token_val_from = up;
  }

  uint32_t max_from_u = (uint32_t)token_val_from;
  uint32_t lim = pos - base;

  if (max_from_u > lim) {
    max_from_u = lim;
  }

  double best_score = 1e100;

  for (uint32_t from = 0u; from <= max_from_u; ++from) {
    uint32_t src_pos = pos - base - from;

    uint16_t token_val_cnt = max_count;

    if (token_val_cnt > (uint16_t)from) {
      token_val_cnt = (uint16_t)from;
    }

    uint32_t extra = (word_mode != 0) ? 0u : 1u;
    uint32_t maxlen = (uint32_t)token_val_cnt + 1u + extra;
    
    if (maxlen > (total_elems - pos)) {
      maxlen = (total_elems - pos);
    }

    uint32_t len = 0u;
    while (len < maxlen) {
      const uint8_t* p1 = in + (pos + len) * stride;
      const uint8_t* p2 = in + (src_pos + len) * stride;

      if (word_mode != 0) {
        if (!(p1[0] == p2[0] && p1[1] == p2[1])) {
          break;
        }
      }
      else {
        if (!(p1[0] == p2[0])) {
          break;
        }
      }
      ++len;
    }

    if (len < minlen) {
      continue;
    }

    int bits = pair_bit_cost(max_from, max_count, word_mode, unp_count, (uint16_t)from, (uint16_t)len);
    
    if (bits < 0) {
      continue;
    }

    double score = (double)bits / (double)len;
    if (score < best_score || (score == best_score && len > (uint32_t)best.len)) {
      best_score = score;
      best.from = (uint16_t)from;
      best.len = (uint16_t)len;
    }
  }

  return best;
}

static int has_any_valid_pair(const uint8_t* src, uint32_t pos, uint32_t total_elems, uint16_t max_from, uint16_t max_count, int word_mode, int unp_count_at_pos) {
  match_t m = find_best_match_cost(src, pos, total_elems, max_from, max_count, word_mode, unp_count_at_pos);
  return (m.len != 0u);
}

static int compress_full(const uint8_t* src, uint32_t src_size, uint8_t* dst, uint16_t max_from, uint16_t max_count, int prefer_word_mode) {
  if (src == NULL || dst == NULL) {
    return -1;
  }

  int word_mode = 0;

  if (prefer_word_mode != 0 && (src_size % 2u) == 0u) {
    word_mode = 1;
  }

  uint32_t stride = (word_mode != 0) ? 2u : 1u;
  uint32_t total_elems = (word_mode != 0) ? (src_size / 2u) : src_size;

  int woff = 0;

  uint32_t left_field = (word_mode != 0) ? (total_elems << 1) : total_elems;
  write_dword_be(dst, &woff, left_field);

  int data_off_field_pos = woff;
  write_dword_be(dst, &woff, 0);

  write_word_be(dst, &woff, max_from);
  write_word_be(dst, &woff, max_count);

  bitwriter_t bw;
  bw_init(&bw, dst, &woff);


  bw_putbit(&bw, (word_mode != 0) ? 1 : 0);

  uint8_t* lit = (uint8_t*)malloc((src_size == 0u) ? 1u : src_size);

  if (lit == NULL) {
    return -1;
  }
  uint32_t lit_size = 0u;

  int unp_count = -1 - ((word_mode != 0) ? 0 : 1);

  uint32_t pos = 0u;
  while (pos < total_elems) {
    uint32_t start_pos = pos;
    int start_unp = unp_count;

    uint32_t lit_len = 1u;

    if (start_pos == 0u) {
      lit_len = (word_mode != 0) ? 1u : 2u;

      if (lit_len > (total_elems - start_pos)) {
        lit_len = (total_elems - start_pos);
      }
    }

    while (start_pos + lit_len < total_elems) {
      uint32_t after = start_pos + lit_len;
      int unp_after = start_unp + (int)lit_len;

      if (has_any_valid_pair(src, after, total_elems, max_from, max_count, word_mode, unp_after)) {
        break;
      }
      ++lit_len;
    }

    if (lit_len == 0u) {
      free(lit);
      return -1;
    }

    write_count(&bw, lit_len - 1u);

    uint32_t bytes = lit_len * stride;

    if (lit_size + bytes > src_size) {
      free(lit);
      return -1;
    }

    memcpy(lit + lit_size, src + start_pos * stride, bytes);
    lit_size += bytes;

    pos = start_pos + lit_len;
    unp_count = start_unp + (int)lit_len;

    if (pos >= total_elems) {
      break;
    }

    match_t pairs[256];
    uint32_t npairs = 0u;

    while (pos < total_elems && npairs < 256u) {
      match_t m0 = find_best_match_cost(src, pos, total_elems, max_from, max_count, word_mode, unp_count);
      if (m0.len == 0u) {
        break;
      }

      if (pos + 1u < total_elems) {
        match_t m1 = find_best_match_cost(src, pos + 1u, total_elems, max_from, max_count, word_mode, unp_count + 1);

        if (m1.len != 0u) {
          int bits0 = pair_bit_cost(max_from, max_count, word_mode, unp_count, m0.from, m0.len);
          int bits1 = pair_bit_cost(max_from, max_count, word_mode, unp_count + 1, m1.from, m1.len);

          double r0 = (bits0 > 0) ? ((double)bits0 / (double)m0.len) : 1e100;
          double r1 = (bits1 > 0) ? ((double)bits1 / (double)m1.len) : 1e100;

          if (r1 + 0.02 < r0 && npairs > 0u) {
            break;
          }
        }
      }

      pairs[npairs++] = m0;
      pos += m0.len;
      unp_count += (int)m0.len;

      if (pos >= total_elems) {
        break;
      }
    }

    if (npairs == 0u) {
      free(lit);
      return -1;
    }

    write_count(&bw, npairs - 1u);

    int sum_len = 0;

    for (uint32_t i = 0; i < npairs; ++i) {
      sum_len += (int)pairs[i].len;
    }

    int unp_enc = unp_count - sum_len;

    for (uint32_t i = 0u; i < npairs; ++i) {
      uint16_t up = (unp_enc < 0) ? 0u : (uint16_t)unp_enc;

      uint16_t token_val_from = max_from;

      if (token_val_from > up) {
        token_val_from = up;
      }

      if (write_token(&bw, token_val_from, pairs[i].from) != 0) {
        free(lit);
        return -1;
      }

      uint16_t token_val_cnt = max_count;

      if (token_val_cnt > pairs[i].from) {
        token_val_cnt = pairs[i].from;
      }

      uint16_t extra = (word_mode != 0) ? 0u : 1u;
      uint16_t count_token = (uint16_t)(pairs[i].len - 1u - extra);

      if (write_token(&bw, token_val_cnt, count_token) != 0) {
        free(lit);
        return -1;
      }

      unp_enc += (int)pairs[i].len;
    }
  }

  bw_finish(&bw);

  uint32_t data_off = (uint32_t)woff;
  uint32_t data_off_minus_8 = data_off - 8u;

  int tmp = data_off_field_pos;
  write_dword_be(dst, &tmp, data_off_minus_8);

  memcpy(dst + data_off, lit, lit_size);
  woff += (int)lit_size;

  free(lit);
  return woff;
}


typedef struct compress_choice_t {
  int word_mode;
  uint16_t max_from;
  uint16_t max_count;
  int compressed_size;
} compress_choice_t;

static uint32_t worst_case_bound(uint32_t src_size) {
  uint32_t a = src_size + 64u;
  uint32_t b = src_size / 4u;
  uint32_t c = a + b;
  if (c < a) {
    return 0xFFFFFFFFu;
  }
  return c;
}

static uint16_t clamp_u16(uint32_t v, uint16_t lo, uint16_t hi) {
  if (v < (uint32_t)lo) {
    return lo;
  }
  if (v > (uint32_t)hi) {
    return hi;
  }
  return (uint16_t)v;
}

static int choose_word_mode_by_size(const uint8_t* src, uint32_t src_size, uint8_t* tmp, uint16_t max_from, uint16_t max_count, int* out_word_mode) {
  int best_mode = 0;
  int best_size = -1;

  int s0 = compress_full(src, src_size, tmp, max_from, max_count, 0);
  if (s0 >= 0) {
    best_size = s0;
    best_mode = 0;
  }

  if ((src_size % 2u) == 0u) {
    int s1 = compress_full(src, src_size, tmp, max_from, max_count, 1);
    if (s1 >= 0) {
      if (best_size < 0) {
        best_size = s1;
        best_mode = 1;
      }
      else {
        if (s1 < best_size) {
          best_size = s1;
          best_mode = 1;
        }
      }
    }
  }

  if (best_size < 0) {
    return -1;
  }

  *out_word_mode = best_mode;
  return best_size;
}

int compress(const uint8_t* src, uint32_t src_size, uint8_t* dst) {
  if (src == NULL || dst == NULL) {
    return -1;
  }

  uint32_t tmp_cap = worst_case_bound(src_size);
  if (tmp_cap == 0xFFFFFFFFu) {
    return 1;
  }

  uint8_t* tmp = (uint8_t*)malloc(tmp_cap);
  if (tmp == NULL) {
    return 1;
  }

  int mode = 0;
  int size = choose_word_mode_by_size(src, src_size, tmp, 0xFFFF, 0xFFFF, &mode);

  int final_size = compress_full(src, src_size, dst, 0xFFFF, 0xFFFF, mode);
  if (final_size < 0) {
    free(tmp);
    return 1;
  }

  free(tmp);
  return final_size;
}

uint32_t max_compressed_size(uint32_t src_size) {
  uint32_t a = src_size + 64u;
  uint32_t b = src_size / 4u;
  uint32_t c = a + b;

  if (c < a) {
    return 0xFFFFFFFFu;
  }

  return c;
}
