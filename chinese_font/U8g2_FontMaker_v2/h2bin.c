/* h2bin: bdfconv 生成的 u8g2_font_xxx.h  ->  u8g2_font.bin (字库字节 + 4 字节 CRC32 小端)
   用法: h2bin.exe u8g2_font.h u8g2_font.bin */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint32_t crc32_calc(const unsigned char* d, size_t n) {
  uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) {
    c ^= d[i];
    for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
  }
  return ~c;
}
static int isoct(int c) { return c >= '0' && c <= '7'; }
static int hexv(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

int main(int argc, char** argv) {
  if (argc != 3) { fprintf(stderr, "usage: h2bin in.h out.bin\n"); return 2; }
  FILE* f = fopen(argv[1], "rb");
  if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 1; }
  fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
  char* t = (char*)malloc(sz + 1);
  if (fread(t, 1, sz, f) != (size_t)sz) { fprintf(stderr, "read error\n"); return 1; }
  t[sz] = 0; fclose(f);

  /* 找 "[N]" 之后的 "U8G2_FONT_SECTION(...)" 和 "=" */
  char* p = strstr(t, "U8G2_FONT_SECTION");
  if (!p) { fprintf(stderr, "U8G2_FONT_SECTION not found\n"); return 1; }
  char* br = p;
  while (br > t && *br != '[') br--;
  long expect = atol(br + 1);
  p = strchr(p, '=');
  if (!p) { fprintf(stderr, "'=' not found\n"); return 1; }
  p++;

  unsigned char* out = (unsigned char*)malloc(expect + 16);
  long n = 0;
  while (*p && *p != ';') {
    if (*p == '"') {
      p++;
      while (*p && *p != '"') {
        int v;
        if (*p != '\\') { v = (unsigned char)*p++; }
        else {
          p++;
          if (isoct(*p)) { v = 0; int k = 0; while (k < 3 && isoct(*p)) { v = v * 8 + (*p - '0'); p++; k++; } }
          else if (*p == 'x') { p++; v = 0; while (hexv(*p) >= 0) { v = v * 16 + hexv(*p); p++; } }
          else {
            switch (*p) {
              case 'n': v = 10; break; case 't': v = 9; break; case 'r': v = 13; break;
              case 'a': v = 7; break;  case 'b': v = 8; break; case 'f': v = 12; break;
              case 'v': v = 11; break; default: v = (unsigned char)*p; break;
            }
            p++;
          }
        }
        if (n >= expect + 8) { fprintf(stderr, "too many bytes\n"); return 1; }
        out[n++] = (unsigned char)(v & 255);
      }
      if (*p == '"') p++;
    } else p++;
  }
  if (n == expect - 1) out[n++] = 0; /* C 字符串隐含的结尾 \0 */
  if (n != expect) { fprintf(stderr, "length mismatch: parsed %ld, declared %ld\n", n, expect); return 1; }

  uint32_t crc = crc32_calc(out, n);
  FILE* o = fopen(argv[2], "wb");
  if (!o) { fprintf(stderr, "cannot write %s\n", argv[2]); return 1; }
  fwrite(out, 1, n, o);
  unsigned char c4[4] = { crc & 255, (crc >> 8) & 255, (crc >> 16) & 255, (crc >> 24) & 255 };
  fwrite(c4, 1, 4, o);
  fclose(o);
  printf("OK: %ld bytes + CRC32 0x%08X -> %s\n", n, crc, argv[2]);
  return 0;
}
