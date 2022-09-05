#pragma once

#include "types.h"

#include <string>

void GenerateCInterface(std::string const &fname, std::string const &modname,
                        ParameterFields const &parameters,
                        DerivedTypes const &dtypes,
                        std::vector<std::string> const &Uses);