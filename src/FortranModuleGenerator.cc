#include "FortranModuleGenerator.h"
#include "utils.h"

#include <map>

std::map<FieldType, std::string> FortranFieldTypes = {
    {FieldType::kInteger, "integer(kind=C_INT)"},
    {FieldType::kString, "character(kind=C_CHAR)"},
    {FieldType::kCharacter, "character(kind=C_CHAR)"},
    {FieldType::kFloat, "real(kind=C_FLOAT)"},
    {FieldType::kDouble, "real(kind=C_DOUBLE)"},
};

void FortranFileHeader(std::ostream &os, std::string const &modname) {
  os << "module " << modname << "\n";
  os << "  use iso_c_binding"
     << "\n\n";
}

void FortranModuleParameters(std::ostream &os,
                             ParameterFields const &ParameterFieldDescriptors) {
  for (auto const &ppair : ParameterFieldDescriptors) {
    auto const &p = ppair.second;
    std::string comment = SanitizeComment(p.comment, "  !");
    if (comment.length()) {
      os << "  !" << comment << std::endl;
    }
    if (p.type == FieldType::kString) {
      os << "  " << FortranFieldTypes[p.type]
         << ",len=*), parameter :: " << p.name << " = \"" << p.value << "\""
         << std::endl;
    } else {
      os << "  " << FortranFieldTypes[p.type] << ", parameter :: " << p.name
         << " = " << p.value << std::endl;
    }
  }
  os << std::endl;
}

void FortranDerivedTypeHeader(std::ostream &os, std::string const &dtypename,
                              std::string comment) {

  comment = SanitizeComment(comment, "  !");
  if (comment.length()) {
    os << "  !" << comment << std::endl;
  }
  os << "  type, bind(C) :: t_" << dtypename << "\n";
}

void FortranDerivedTypeField(std::ostream &os, FieldDescriptor const &fd) {

  std::string comment = SanitizeComment(fd.comment, "    !");
  if (comment.length()) {
    os << "    !" << comment << std::endl;
  }
  os << "    ";
  os << FortranFieldTypes[fd.type];
  if (fd.is_array()) {
    os << ", dimension(";
    for (int i = 0; i < fd.size.size(); ++i) {
      auto dim = fd.size[i];
      if (dim.index() == FieldDescriptor::kSizeString) {
        os << std::get<FieldDescriptor::kSizeString>(dim)
           << ((i + 1 == fd.size.size()) ? "" : ", ");
      } else {
        os << std::get<FieldDescriptor::kSizeInt>(dim)
           << ((i + 1 == fd.size.size()) ? "" : ", ");
      }
    }
    os << ")";
  }
  os << " :: " << fd.name;
  os << "\n";
}

void FortranDerivedTypeFooter(std::ostream &os, std::string const &dtypename) {
  os << "\n  end type t_" << dtypename << "\n\n";
  os << "  type (t_" << dtypename << "), bind(C) :: " << dtypename << "\n\n";
}

void FortranFileFooter(std::ostream &os, std::string const &modname) {
  os << "  save"
     << "\n";
  os << "end module " << modname << "\n";
}

void GenerateFortranModule(std::string const &fname, std::string const &modname,
                           ParameterFields const &parameters,
                           DerivedTypes const &dtypes) {

  std::ofstream out(fname);

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
