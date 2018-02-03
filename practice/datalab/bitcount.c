#include <stdio.h>
#include <limits.h>

int bitCount(int x)
{
  int c;
  /*c = ((unsigned)(x & 0xAAAAAAAA)>>1) + (x & 0x55555555); //step 1 
  c = ((unsigned)(c & 0xCCCCCCCC)>>2) + (c & 0x33333333); //step 2 
  c = ((unsigned)(c & 0xF0F0F0F0)>>4) + (c & 0x0F0F0F0F); //step 3 F0F0F0F0
  c = ((unsigned)(c & 0xFF00FF00)>>8) + (c & 0x00FF00FF); //step 4 FF00FF00
  c = ((unsigned)(c & 0xFFFF0000)>>16) + (c & 0x0000FFFF); //step 5 FFFF0000
  */
  c = ((x & 0xAAAAAAAA)>>1) + (x & 0x55555555); //step 1 
  c = ((c & 0xCCCCCCCC)>>2) + (c & 0x33333333); //step 2 
  c = ((c & 0xF0F0F0F0)>>4) + (c & 0x0F0F0F0F); //step 3 F0F0F0F0
  c = ((c & 0xFF00FF00)>>8) + (c & 0x00FF00FF); //step 4 FF00FF00
  c = ((c & 0xFFFF0000)>>16) + (c & 0x0000FFFF); //step 5 FFFF0000
  
  return c;
  
}
int main(int argc, char *argv[])
{
  int i = -1;
  printf("Bits set in %d are: %d\n", i, bitCount(i));
  i = INT_MIN;
  printf("Bits set in %d are: %d\n", i, bitCount(i));
  i = INT_MAX;
  printf("Bits set in %d are: %d\n", i, bitCount(i));
  i = 0;
  printf("Bits set in %d are: %d\n", i, bitCount(i));
  return 0;
}
