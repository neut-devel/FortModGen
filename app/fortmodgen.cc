#include "CInterfaceGenerator.h"
#include "FortranModuleGenerator.h"
#include "types.h"

#include "toml.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <string>

void Usage(char const *argv[]) {
  std::cout << "[USAGE]: " << argv[0] << std::endl;
}

std::string fin, outstub;

void ParseOpts(int argc, char const *argv[]) {
  for (int opt_it = 1; opt_it < argc; opt_it++) {
    std::string arg = argv[opt_it];
    if ((arg == "-h") || (arg == "-?") || (arg == "--help")) {
      Usage(argv);
      exit(0);
    } else if ((opt_it + 1) < argc) {
      if (arg == "-i") {
        fin = argv[++opt_it];
      } else if (arg == "-o") {
        outstub = argv[++opt_it];
      }
    }
  }
  if (!fin.size() || !outstub.length()) {
    std::cerr << "[ERROR]: Not all required options recieved: (-i, -o)."
              << std::endl;
    Usage(argv);
    exit(1);
  }
}

int main(int argc, char const *argv[]) {
  ParseOpts(argc, argv);

  toml::value fmod_descriptor;
  try {
    auto doc = toml::parse(fin);
    fmod_descriptor = toml::find(doc, "module");
  } catch (std::runtime_error const &e) {
    std::cout << "[ERROR]: Failed to parse toml file: " << fin
              << ", with error: " << e.what() << std::endl;
    abort();
  }

  std::string modname = toml::find<std::string>(fmod_descriptor, "name");
  auto dtypenames =
      toml::find<std::vector<std::string>>(fmod_descriptor, "derivedtypes");

  auto ParameterFieldDescriptors =
      toml::find_or<std::vector<ParameterFieldDescriptor>>(fmod_descriptor,
                                                           "parameters", {});

  auto Uses =
      toml::find_or<std::vector<std::string>>(fmod_descriptor, "uses", {});

  std::cout << "Found module descriptor for module: " << modname << " with "
            << dtypenames.size() << " defined derived types and "
            << ParameterFieldDescriptors.size() << " parameters." << std::endl
            << std::endl;

  std::cout << "Parameters: " << std::endl;
  for (auto const &p : ParameterFieldDescriptors) {
    std::cout << "  " << p << std::endl;
  }

  std::cout << std::endl << "Derived types: " << std::endl;

  DerivedTypes TypeFieldDescriptors;
  for (auto const &dtypename : dtypenames) {
    std::cout << "\t" << dtypename << std::endl;
    auto dtype_table = toml::find(fmod_descriptor, dtypename);

    TypeFieldDescriptors[dtypename].comment =
        toml::find_or<std::string>(dtype_table, "comment", "");

    for (auto const &fd :
         toml::find<std::vector<FieldDescriptor>>(dtype_table, "fields")) {
      TypeFieldDescriptors[dtypename].fields.push_back(fd);
      std::cout << "\t\t" << fd << std::endl;

      if (fd.size.size()) {
        for (auto const &dim : fd.size) {

          if (dim.index() == FieldDescriptor::kSizeString) {

            bool found = false;
            for (auto const &p : ParameterFieldDescriptors) {
              if (std::get<FieldDescriptor::kSizeString>(dim) == p.name) {
                if (!p.is_integer()) {
                  std::cout << "[ERROR]: Field \"" << fd.name << "\" on type \""
                            << dtypename << "\" has dimension parameter: \""
                            << std::get<FieldDescriptor::kSizeString>(dim)
                            << "\", which is a non-integer type: " << p.type
                            << std::endl;
                  abort();
                }
                found = true;
              }
            }

            if (!found) {
              std::cout << "[ERROR]: Field \"" << fd.name << "\" on type \""
                        << dtypename << "\" has dimension parameter: \""
                        << std::get<FieldDescriptor::kSizeString>(dim)
                        << "\", which is not a declared parameter."
                        << std::endl;
              abort();
            }
          }
        }
      }
    }
  }

  GenerateFortranModule(outstub + ".f90", modname, ParameterFieldDescriptors,
                        TypeFieldDescriptors, Uses);
  GenerateCInterface(outstub + ".h", modname, ParameterFieldDescriptors,
                     TypeFieldDescriptors, Uses);
}
