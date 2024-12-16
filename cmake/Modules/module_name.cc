#include "toml.hpp"

#include <iostream>

int main(int argc, char const *argv[]){
  std::cout << toml::find<std::string>(toml::parse(argv[1]),"module", "name") << std::endl;
  return 0;
}