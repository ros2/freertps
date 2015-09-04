// initial minimal program to sanity-check systems

int main(int argc, char **argv)
{
  while (1)
  {
    for (volatile int i = 0; i < 10000000; i++);
    led_toggle();
  }
  return 0;
}

