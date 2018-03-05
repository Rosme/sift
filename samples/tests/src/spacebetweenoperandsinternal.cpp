int NoSpace( int x,int y ) {
  if (x!=y)
  {
    x++;
  }

  for ( int i=0;i<=x;i++ )
  {
    y++;
  }
  while (x==y)
  {
    y++;
  }
  return y;
}

int Space(int x, int y) {
  if (x != y)
  {
    x++;
  }

  for (int i = 0; i < x; i++)
  {
    y++;
  }
  return y;
}
