#include <iostream>
#include "core/utils/json.hpp"


int main(int argc, char* argv[]) {
  nlohmann::json json;

  json["type"] = "Welcome";
  json["value"] = std::vector<std::string>({ "Hello", "World" });

  Core::write_json_file("testFile.txt", json);
  
  auto readJson = Core::read_json_file("testFile.txt");

  std::cout << readJson;
  std::cin.get();
  return 0;
}
