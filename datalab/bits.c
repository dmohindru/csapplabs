/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* We use here demorgan law to solve the problem
   * NOT(NOT(A) OR NOT(B))
   * NOT(NOT(A)) AND NOT(NOT(B)) 
   * A AND B */
  
  return ~((~x)|(~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /* Multiply n by eigth by shifting 3 place to left
   * then move x to left number of place we get by multipling n
   * by eight. Then And the least significant byte with 0XFF
   */
  return (x >> (n << 3)) & 0xFF;

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /* 1. Firt create mask of format 011....11
   * 2. Then right shift mask by n which give format 00011...1
   * 3. Then add 1 to above mask to give format 00100....0, which makes
   *    up for the extra zero introduced in step 1
   * 4. Then OR the result with mask generated in step 2, which gives
   *    desired mask
   * 5. right shift x and perform AND operation with mask.
   * 
   */
  int mask = (~0 ^ (1 << 31));
  mask = (~0 & mask) >> n;
  mask = mask | (mask + 1);
  return (x >> n) & mask;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /* Idea that we follow to solve this problem
   * is to count bits using divide and rule principle.
   * As illustration for 8 bit number
   * Step1: Add bits 8+7, 6+5, 4+3, 2+1
   * Step2: Add bits 8-7 + 6-5, 4-3 + 2-1
   * Step3: Add bits 8-7-6-4 + 4-3-2-1. And this is a result. 
   * As bits increases these steps increases
   * for 16 bits: 4 steps
   * for 32 bits: 5 steps as demonstrated below
   */
  
  int c, mask1, mask2, mask3, mask4, mask5;
  mask1 = (0x55 << 8) | 0x55;
  mask1 = (mask1 << 16) | mask1; //mask: 0X55555555
  
  mask2 = (0x33 << 8) | 0x33;   
  mask2 = (mask2 << 16) | mask2; //mask: 0X33333333
  
  mask3 = (0x0F << 8) | 0x0F;
  mask3 = (mask3 << 16) | mask3; //mask: 0X0F0F0F0F
  
  mask4 = 0xFF;
  mask4 = (mask4 << 16) | mask4; //mask 0x00FF00FF    
  
  mask5 = (0xFF << 8) | 0xFF;   //mask 0x0000FFFF
  
  c = ((x >> 1) & mask1) + (x & mask1); //step 1 
  c = ((c >> 2) & mask2) + (c & mask2); //step 2 
  c = ((c >> 4) & mask3) + (c & mask3); //step 3 
  c = ((c >> 8) & mask4) + (c & mask4); //step 4 
  c = ((c >> 16) & mask5) + (c & mask5); //step 5
  
  return c;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /* Using observation (x | ~x + 1) will be all one for any number
   * other than zero.
   * For zero (x | ~x + 1) will be all zero.
   * Hence if all one adding 1 to it will generate all zeros as desired
   * If all zero and 1 to it will generate 1 as desired
   */
  return ((x | (~x + 1)) >> 31) + 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /* Moving 1 to MSB using left shift */
  return 1 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  /* Idea is move x 32-n bits to left
   * then move it back to right with 32-n bits
   * and xor with original value to see if any
   * of the bits are lost/modified in move operation.
   * If bits are lost x cannot be represented as n bit 2s
   * complement representation.
   */
  int a = (33 + (~n)); //calculate 32-n bits
  int b = (x << a);    //move x 32-n bits to left
  int c = (b >> a);    //move back x 32-n bits to right
  int e = !(c ^ x);    //check if we lost some bits, if yes then we cannot represent
  return  e;           // x in n bits
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  /* Divide by power of two is basically right shift a number by n.
   * For positive number is just the right shift.
   * For negetive number is right shift + bias.
   * e.g. assuming 8 bit representatoin
   * if number is 1111 1000 divide by 8 or right shift by 3.
   * we need to add bais of form 111 (n bits) to a number.
   * if any lower n bits contain any 1s then effect of adding bais will
   * increment the shifted number by one
   * e.g. for 8 bit number 1111 1000 adding bais 0000 0111 will be 
   * 1111 1111 >> 3 = 1111 1111
   * for 8 bit number 1111 1010 adding bais 0000 0111 will be 
   * 1110 1001 >> 3 =  
   */
    int sign = (x>>31)&1;
    int bias = (sign<<n) + ~sign + 1;
    return (x + bias)>>n; 
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /* Observation: as negetive of x is ~x + 1 */
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  /* Check if number is -ve or number is zero
   * if both is false then number is greater 0
   */
  return !((x & (1 << 31)) | !x);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int mask = x ^ y;
  int diff_sign = ((x >> 31) ^ (y >> 31)) & 0x1;
  mask = (mask >> 1) | mask;
  mask = (mask >> 2) | mask;
  mask = (mask >> 4) | mask;
  mask = (mask >> 8) | mask;
  mask = (mask >> 16) | mask;
  mask = (mask & (~mask >> 1)) | (diff_sign << 31);
  return !(x & mask) ^ diff_sign; 
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  /* To calculate log2(num) count number of digits
   * left to the most significant 1 in a number
   * e.g
   * 2  = 0b10       -----> log2(2)  = 1
   * 16 = 0b10000    -----> log2(16) = 4
   * 64 = 0b1000000  -----> log2(64) = 6
   * 78 = 0b1001110  -----> log2(78) = 6
   * So the solve this problem we first generate a mask 
   * which has 1 in most significant place and rest all zero
   * to right of it
   * e.g. 78 = 0b1001110 -----> (mask) = 0b1000000
   * then invert this mask into form 0b0111111
   * finally count number of 1s in above generated mask
   * using function bitCount  
   * 
   */
  int c, mask1, mask2, mask3, mask4, mask5;
  
  x = (x >> 1) | x;
  x = (x >> 2) | x;
  x = (x >> 4) | x;
  x = (x >> 8) | x;
  x = (x >> 16) | x;
  x = x & (~x >> 1); 
  
  
  mask1 = (0x55 << 8) | 0x55;
  mask1 = (mask1 << 16) | mask1; //mask: 0X55555555
  
  mask2 = (0x33 << 8) | 0x33;   
  mask2 = (mask2 << 16) | mask2; //mask: 0X33333333
  
  mask3 = (0x0F << 8) | 0x0F;
  mask3 = (mask3 << 16) | mask3; //mask: 0X0F0F0F0F
  
  mask4 = 0xFF;
  mask4 = (mask4 << 16) | mask4; //mask 0x00FF00FF    
  
  mask5 = (0xFF << 8) | 0xFF;   //mask 0x0000FFFF
  
  x = (~x) + (x << 1);
  c = ((x >> 1) & mask1) + (x & mask1); //step 1 
  c = ((c >> 2) & mask2) + (c & mask2); //step 2 
  c = ((c >> 4) & mask3) + (c & mask3); //step 3 
  c = ((c >> 8) & mask4) + (c & mask4); //step 4 
  c = ((c >> 16) & mask5) + (c & mask5); //step 5
  
  return c;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  unsigned exp = uf >> 23 & 0xFF;
  unsigned frac = uf & 0x7FFFFF;
  if (exp == 0xFF && frac != 0) //its a NaN
    return uf;
  else
    return uf ^ 0x80000000;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    int e = 158;
    int mask = 1<<31;
    int sign = x&mask;
    int frac;
    if (x == mask)
        return mask | (158<<23);
    if (!x)
        return 0;
    if (sign)
        x = ~x + 1;
    while (!(x&mask)) {
        x = x<<1;
        e = e -1;
    }
    frac = (x&(~mask)) >> 8;
    if (x&0x80 && ((x&0x7F) > 0 || frac&1)) //round off condition
        frac = frac + 1;
    return sign + (e<<23) + frac;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = uf >> 23 & 0xFF;
  unsigned frac = uf & 0x7FFFFF;
  if (exp == 0xFF) //its a NaN or oo
    return uf;
  else if (exp == 0) //its a denormalized number
    frac <<= 1;
  else if(exp == 0xFF - 1)//max exponent
  {
    exp = 0xFF;
    frac = 0;
  }
  else //normal number
    exp += 1;
  return (sign << 31 | exp << 23 | frac);
}
