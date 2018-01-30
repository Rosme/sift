/*
             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.



 Author: zadig
*/

#include <iostream>
#include <fstream>
#include "config.h"

namespace brightnessmanager {

long get_value(const char *filename, long default_value) {
  std::filebuf fb;
  char value[8] = {0};
  char *ptr;
  long lvalue;
  if (fb.open(filename, std::ios::in)) {
    std::istream is(&fb);
    is.getline(value, 8);
    lvalue = strtol(value, &ptr, 10);
    if (value == ptr) 
      lvalue = default_value;
    fb.close();
  } else {
    lvalue = default_value;
  }
  return lvalue;
}

bool write_value(const char *filename, long value) {
  std::filebuf fb;
  bool result;
  std::string str_value = std::to_string(value);

  if (fb.open(filename, std::ios::out)) {
    std::ostream os(&fb);
    os << str_value;
    fb.close();
    result = true;
  } else {
    std::cerr << "Unable to write to " << filename << std::endl;
    result = false;
  }

  return result;
}

long get_delta(const char *argv, bool *error) {
  char *ptr;
  long value;
  *error = false;

  value = strtol(argv, &ptr, 10);
  if (argv == ptr) {
    *error = true;
    value = 0;
  }

  return value;
} 

}

int main(int argc, const char *argv[]) {
  int ret = 0;
  long current, max;
  long percent, final;
  long delta;
  bool error = false;
  if (argc == 2) {
    current = brightnessmanager::get_value(CURRENT_BRIGHTNESS_FILE, -1);
    max = brightnessmanager::get_value(MAX_BRIGHTNESS_FILE, -1);
    if (current == -1 || max == -1 || max == 0) {
      std::cerr << "Unable to open " << ((current == -1)?CURRENT_BRIGHTNESS_FILE:MAX_BRIGHTNESS_FILE) << std::endl;
      ret = -1;
    } else {
      percent = current * 100 / max;
      delta = brightnessmanager::get_delta(argv[1], &error);
      if (error) {
        std::cerr << "Unable to understand your delta." << std::endl;
        ret = -1;
      } else {
        std::cout << "Current: " << percent << std::endl;
        if (*argv[1] == '+' || *argv[1] == '-')
          percent += delta;
        else
          percent = delta;
			
        if (percent > 100 || percent < 0) {
          std::cerr << "Out of range." << std::endl;
          ret = -1;
        } else {
          final = percent * max;
          final /= 100;
          std::cout << "    New: " << percent << std::endl;
          brightnessmanager::write_value(CURRENT_BRIGHTNESS_FILE, final);
          ret = 0;
        }
      }
    }
  } else {
    std::cerr << "too few arguments." << std::endl;
    ret = -1;
  }
  return ret;
}
