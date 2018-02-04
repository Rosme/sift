/*
             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 2, December 2004

 Copyright (C) 2018 Jean-Sebastien Fauteux, Michel Rioux, Raphael Massabot

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.
*/

namespace OuterNamespace {

  namespace InnerNamespace {

    enum InsideInner {
      IIValue1,
      IIValue2,
      IIValue3
    };

  }

  enum InsideOuter : short {
    IOValue1,
    IOValue2,
    IOValue3
  };

}

enum class ECNoSpecialization {
  ECNSValue1,
  ECNSValue2,
  ECNSValue3
};

enum class ECSpecialization : unsigned int {
  ECSValue1,
  ECSValue2,
  ECSValue3
};

int main(int argc, char* argv[]) {

  return 0;
}