#include "FortranModuleGenerator.h"
#include "utils.h"

#include "fmt/os.h"

#include <map>

std::map<FieldType, std::string> FortranFieldTypes = {
    {FieldType::kInteger, "integer"},     {FieldType::kString, "character"},
    {FieldType::kCharacter, "character"}, {FieldType::kFloat, "real"},
    {FieldType::kDouble, "real"},
};

std::map<FieldType, std::string> FortranFieldKinds = {
    {FieldType::kInteger, "C_INT"},    {FieldType::kString, "C_CHAR"},
    {FieldType::kCharacter, "C_CHAR"}, {FieldType::kFloat, "C_FLOAT"},
    {FieldType::kDouble, "C_DOUBLE"},
};

void FortranFileHeader(fmt::ostream &os, std::string const &modname) {
  os.print("module {}\n  use iso_c_binding\n\n", modname);
}

void FortranModuleParameters(fmt::ostream &os,
                             ParameterFields const &ParameterFieldDescriptors) {
  for (auto const &ppair : ParameterFieldDescriptors) {
    auto const &p = ppair.second;
    std::string comment = SanitizeComment(p.comment, "  !");
    if (comment.length()) {
      os.print("  !{}\n", comment);
    }
    if (p.is_string()) {
      os.print("  {}(kind={},len=*), parameter :: {} = \"{}\"\n",
               FortranFieldTypes[p.type], FortranFieldKinds[p.type], p.name,
               p.value);
    } else {
      os.print("  {}(kind={}), parameter :: {} = {}\n",
               FortranFieldTypes[p.type], FortranFieldKinds[p.type], p.name,
               p.value);
    }
  }
  os.print("\n");
}

void FortranDerivedTypeHeader(fmt::ostream &os, std::string const &dtypename,
                              std::string comment) {

  comment = SanitizeComment(comment, "  !");
  if (comment.length()) {
    os.print("  !{}\n", comment);
  }
  os.print("  type, bind(C) :: t_{}\n", dtypename);
}

void FortranDerivedTypeField(fmt::ostream &os, FieldDescriptor const &fd) {

  std::string comment = SanitizeComment(fd.comment, "    !");
  if (comment.length()) {
    os.print("    !{}\n", comment);
  }
  os.print("    {}(kind={})", FortranFieldTypes[fd.type],
           FortranFieldKinds[fd.type]);
  if (fd.is_array()) {
    os.print(", dimension(");
    for (int i = 0; i < fd.size.size(); ++i) {
      os.print("{}{}", fd.get_dim_size_str(i),
               ((i + 1 == fd.size.size()) ? "" : ", "));
    }
    os.print(")");
  }
  os.print(" :: {}\n", fd.name);
}

void FortranDerivedTypeFooter(fmt::ostream &os, std::string const &dtypename) {
  os.print("\n  end type t_{0}\n\n  type (t_{0}), bind(C) :: {0}\n\n",
           dtypename);
}

void FortranFileFooter(fmt::ostream &os, std::string const &modname) {
  os.print("  save\nend module {}\n", modname);
}

void GenerateFortranModule(std::string const &fname, std::string const &modname,
                           ParameterFields const &parameters,
                           DerivedTypes const &dtypes) {

  auto out = fmt::output_file(fname);

  FortranFileHeader(out, modname);

  FortranModuleParameters(out, parameters);

  for (auto const &dt : dtypes) {

    FortranDerivedTypeHeader(out, dt.first, dt.second.comment);

    for (auto const &fdpair : dt.second.fields) {

      FortranDerivedTypeField(out, fdpair);
    }

    FortranDerivedTypeFooter(out, dt.first);
  }

  FortranFileFooter(out, modname);
}
