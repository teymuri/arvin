



int int_plus(int args[], int count)	/* integer only args */
{
  int out = 0;
  for (int i = 0; i < count; i++)
    out += args[i];
  return out;
}

double nonint_plus(double args[], int count) /* args with at least 1
						non-int number */
{
  double out = 0.0;
  for (int i = 0; i < count; i++)
    out += args[i];
  return out;
}
