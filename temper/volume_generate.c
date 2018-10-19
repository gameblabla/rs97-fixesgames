int main()
{
  unsigned int i;
  int x;

  for(i = 0; i < 92; i++)
  {
    x = (1 << 15) *
     pow(10.0, (double)(91 - i) * (-1.5) / 20.0);

    printf("%d, \n", x);
  }

  return 0;
}

