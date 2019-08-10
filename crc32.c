// Simple public domain implementation of the standard CRC32 checksum.

#include <stdio.h>
#include <stdlib.h>

unsigned int crc32_for_byte(unsigned int r)
  {
  int i = 0;
  while (i < 8)
    {
    r = (r & 1? 0: (unsigned int)0xEDB88320L) ^ r >> 1;
    ++i;
    }
  return r ^ (unsigned int)0xFF000000L;
  }

void crc32(const void *data, size_t n_bytes, unsigned int* crc)
  {
  static unsigned int table[0x100];
  size_t i = 0;
  size_t j = 0;

  if (!*table)
    while (i < 0x100)
      {
      table[i] = crc32_for_byte(i);
      ++i;
      }
  while (j < n_bytes)
    {
    *crc = table[(unsigned char)*crc ^ ((unsigned char*)data)[j]] ^ *crc >> 8;
    ++j;
    }
  }

int CheckCrc32(char *fi)
  {
  FILE *fp;
  char buf[1L << 15];
  unsigned int crc = 0;

  fp = fopen(fi, "rb");
  if (fp != NULL)
    while(!feof(fp) && !ferror(fp))
      crc32(buf, fread(buf, 1, sizeof(buf), fp), &crc);
  else
    return -1;
  fclose(fp);

  return crc;
  }