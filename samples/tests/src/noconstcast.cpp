#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
  std::string exampleString("Example");

  //int* b = const_cast<int*>(...);
  

  char* r = const_cast<char*>(exampleString.c_str());

  return 0;
}