#include <stdio.h>

int main()
{
  char *input_str = "1 2 z";
  int num1, num2, num3, ret_value;
  ret_value = sscanf(input_str, "%d %d %d", &num1, &num2, &num3);
  printf("ret_val=%d\n", ret_value);
  printf("num1=%d\n", num1);
  printf("num2=%d\n", num2);
  printf("num3=%d\n", num3);
  return 0;
}
