int test()
{
  int i = 0;
  if (i == 0)
  {
    goto test;
  }

test:
  i = 1;
  return 0;
}

int noGotoFunction()
{
  int gototest = 0;
  int testgoto = 0;
  int testgototest = 0;

  return gototest;
}