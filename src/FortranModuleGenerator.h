#pragma once

#include "types.h"

#include <string>

void GenerateFortranModule(std::string const &fname, std::string const &modname,
                           ParameterFields const &parameters,
                           DerivedTypes const &dtypes,
                           std::vector<std::string> const &Uses);