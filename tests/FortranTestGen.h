#include "types.h"

#include <string>

void GenFortranTest(std::string const &fname, std::string const &modname,
                    DerivedTypes const &dtypes);
void GenCTest(std::string const &fname, std::string const &modname,
              DerivedTypes const &dtypes);
void GenSteeringScript(std::string const &fname, std::string const &modname);
