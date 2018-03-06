 int NoCodeAllowedAfterBracket(int x, int y) {          
  if (x != y)
  {     
    x++;
  }  
  else if (y == 5) {            
    x = 8;
  }           

  for (int i = 0; i <= x; i++)  {   
    y++;
  }
  while (x == y)
  {           
    y++;
  }   

  return y;
}       

 int CodeAllowedAfterBracket(int x, int y) {  int w = 0;
  if (x != y)
  {  x++;  }  
  for (int i = 0; i < x; i++)  {  y++; 
  } return y;
 } int test(int x, int y) {
   return 0;
}
