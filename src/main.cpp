#include <iostream>
#include "core/utils/json.hpp"
#include <muflihun/easylogging++.h>

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  nlohmann::json json;

  json["type"] = "Welcome";
  json["value"] = std::vector<std::string>({ "Hello", "World" });

  Core::write_json_file("testFile.txt", json);
  
  auto readJson = Core::read_json_file("testFile.txt");

  LOG(INFO) << readJson;
  std::cin.get();
  return 0;
}
