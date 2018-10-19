#include <stdio.h>
#include <math.h>

double arc_length(double pitch, double theta)
{
  double x = sqrt(1.0 + (theta * theta));
  return (0.5 * pitch * ((theta * x) + log(theta + x)));
}

double search_theta(double pitch, double length)
{
  int min = 0;
  int max = 1 << 18;
  double middle;
  double guess;

  while(min < (max - 1))
  {
    middle = (max + min) / 2.0;

    guess = arc_length(pitch, middle);

    if(guess < length)
      min = middle;
    else
      max = middle;
  }

  return middle;
}

int main(int argc, char *argv[])
{
  int sector_number = strtol(argv[1], NULL, 16);

  double correct_arc = 17.33 * sector_number;
  double radians, rotations;
  double pitch = 1.5 / 1000.0;
  double search_length;

  pitch = pitch / (2.0 * M_PI);

  double null_length = arc_length(pitch, 25.0 / pitch);

  printf("whole length: %lf\n", arc_length(pitch, 58.0 / pitch) / 1000.0);

  search_length = search_theta(pitch, correct_arc + null_length);

  printf("theta: %lf (%x), r: %lf\n",
   search_length, (int)(search_length / (2.0 * M_PI)),
   search_length * pitch);
}
