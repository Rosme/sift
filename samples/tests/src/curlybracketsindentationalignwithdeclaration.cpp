
//Fine
namespace MyNamespace
{
  //Fine
  class MyClass 
  {

  };
}

namespace BadNamespace
{
  }

void bothFn()
  {
  }

//Error
class WrongClass {

};

//Error
void fn() {

}

//Error
int main() {

  int i = 5;
  //Error
  if(i < 5) {

  } 
  else 
  {
    //Fine
  }

  return 0;
}