#include <iostream>

int main(int argc, char **argv)
{
  std::cout << "Hello, World!" << std::endl;
  return 0;
}

/*
extern "C"
{
  void _exit(int status);
}

void _exit(int status)
{
  (void)status;
  while (1) {} // wait for somebody to stop us with jtag
}
*/
