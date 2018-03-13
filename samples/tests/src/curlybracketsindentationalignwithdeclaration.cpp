
//Fine
namespace MyNamespace
{
  //Fine
  class MyClass 
  {

  };
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
  //
  if(i < 5) {

  } 
  else 
  {
    //Fine
  }

  return 0;
}