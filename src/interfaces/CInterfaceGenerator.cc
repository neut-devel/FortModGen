#include "CInterfaceGenerator.h"

#include "utils.h"

#include <map>

std::map<FieldType, std::string> CFieldTypes = {
    {FieldType::kInteger, "int"},    {FieldType::kString, "char"},
    {FieldType::kCharacter, "char"}, {FieldType::kFloat, "float"},
    {FieldType::kDouble, "double"},
};

void CFileHeader(std::ostream &os, std::string const &modname) {
  os << "#pragma once"
     << "\n\n";
  os << "extern \"C\" { "
     << "\n\n";
}

void CModuleParameters(std::ostream &os,
                       ParameterFields const &ParameterFieldDescriptors) {
  for (auto const &ppair : ParameterFieldDescriptors) {
    auto const &p = ppair.second;
    std::string comment = SanitizeComment(p.comment, "//");
    if (comment.length()) {
      os << "//" << comment << std::endl;
    }
    if (p.is_string()) {
      os << CFieldTypes[p.type] << "* const " << p.name << " = \"" << p.value
         << "\""
         << ";" << std::endl;
    } else {
      os << CFieldTypes[p.type] << " const " << p.name << " = " << p.value
         << ";" << std::endl;
    }
    os << std::endl;
  }
}

void CDerivedTypeHeader(std::ostream &os, std::string const &dtypename,
                        std::string comment) {

  comment = SanitizeComment(comment, "//");
  if (comment.length()) {
    os << "//" << comment << std::endl;
  }
  os << "extern struct {\n\n";
}

void CDerivedTypeField(std::ostream &os, FieldDescriptor const &fd) {
  std::string comment = SanitizeComment(fd.comment, "  //");
  if (comment.length()) {
    os << "  //" << comment << std::endl;
  }
  os << "  " << CFieldTypes[fd.type] << " " << fd.name;

  if (fd.is_array()) {
    for (int i = fd.size.size(); i > 0; --i) {
      auto dim = fd.size[i - 1];
      if (dim.index() == FieldDescriptor::kSizeString) {
        os << "[" << std::get<FieldDescriptor::kSizeString>(dim) << "]";
      } else {
        os << "[" << std::get<FieldDescriptor::kSizeInt>(dim) << "]";
      }
    }
  }

  os << ";\n";
}

void CDerivedTypeFooter(std::ostream &os, std::string const &dtypename) {
  os << "\n} " << dtypename << ";\n\n";
}

void CFileFooter(std::ostream &os, std::string const &modname) {
  os << "} "
     << "\n";
}

void GenerateCInterface(std::string const &fname, std::string const &modname,
                        ParameterFields const &parameters,
                        DerivedTypes const &dtypes) {

  std::ofstream out(fname);

  CFileHeader(out, modname);

  CModuleParameters(out, parameters);

  for (auto const &dt : dtypes) {

    CDerivedTypeHeader(out, dt.first, dt.second.comment);

    for (auto const &fd : dt.second.fields) {

      CDerivedTypeField(out, fd);
    }

    CDerivedTypeFooter(out, dt.first);
  }

  CFileFooter(out, modname);

  out << "\n\n#ifdef __cplusplus" << std::endl << std::endl;
  out << "#include <cstring>" << std::endl;
  out << "#include <algorithm>" << std::endl << std::endl;
  out << "namespace " << modname << " {" << std::endl;

  for (auto const &dt : dtypes) {
    out << "struct " << dt.first << "IFace {" << std::endl;

    for (auto const &fd : dt.second.fields) {

      if (fd.is_string()) {
        int string_capacity = std::get<FieldDescriptor::kSizeInt>(fd.size[0]);

        out << "  // Accessors for " << fd.name << std::endl;
        out << "  static char const * get_" << fd.name << "() {" << std::endl;
        out << "    return " << dt.first << "." << fd.name << ";" << std::endl;
        out << "  }" << std::endl;

        out << "  static int get_" << fd.name << "_capacity() {" << std::endl;
        out << "    return " << string_capacity << ";" << std::endl;
        out << "  }" << std::endl;

        out << "  static void set_" << fd.name << "(" << CFieldTypes[fd.type]
            << " const *x, int n) {" << std::endl;
        out << "    if((n+1) > " << string_capacity << "){" << std::endl;
        out << "      std::cout << \"[WARNING]: When setting " << fd.name
            << ", a cstr of length: \" << n " << std::endl;
        out << "                << \" was passed, but " << fd.name
            << " only has a capacity of: \" " << std::endl;
        out << "                << string_capacity << \". It will be "
               "truncated.\" <<  std::endl;"
            << std::endl;
        out << "    }" << std::endl;
        out << "    std::memset(" << dt.first << "." << fd.name
            << ", x, std::min(n+1," << string_capacity << "));" << std::endl;
        out << "  }" << std::endl << std::endl;

      } else if (fd.is_array()) {

        out << "  // Accessors for " << fd.name << std::endl;
        out << "  static int get_" << fd.name << "_ndims() { return "
            << fd.size.size() << "; } " << std::endl;

        out << "  static int const * get_" << fd.name << "_shape() { "
            << std::endl;
        out << "    static int shape[] = {";

        for (int i = fd.size.size(); i > 0; --i) {
          out << fd.get_dim_size(i - 1, parameters) << ((i == 1) ? "" : ", ");
        }

        out << "};" << std::endl;
        out << "    return shape; " << std::endl;
        out << "  } " << std::endl;

        out << "  static " << CFieldTypes[fd.type] << " const ";

        for (int i = fd.size.size(); i > 0; --i) {
          out << "*";
        }

        out << " get_" << fd.name << "() {" << std::endl;
        out << "    return " << dt.first << "." << fd.name << ";" << std::endl;
        out << "  }" << std::endl << std::endl;

        out << "  static void set_" << fd.name << "(" << CFieldTypes[fd.type]
            << " const ";
        for (int i = fd.size.size(); i > 0; --i) {
          out << "*";
        }
        out << " x) {" << std::endl;

        std::string indent = "    ";
        char varname = 'i';
        for (int i = fd.size.size(); i > 0; --i) {
          int dimsize = fd.get_dim_size(i - 1, parameters);
          out << indent << "for(int " << varname << " = 0; " << varname << " < "
              << dimsize << "; ++" << varname << ") {" << std::endl;

          indent += "  ";
          varname++;
        }

        out << indent << dt.first << "." << fd.name;

        varname = 'i';
        for (int i = fd.size.size(); i > 0; --i) {
          out << "[" << varname << "]";
          varname++;
        }

        out << " = x";
        varname = 'i';
        for (int i = fd.size.size(); i > 0; --i) {
          out << "[" << varname << "]";
          varname++;
        }

        out << ";" << std::endl;

        for (int i = fd.size.size(); i > 0; --i) {
          indent = indent.substr(0, indent.size() - 2);
          out << indent << "}" << std::endl;
        }

        out << "  }" << std::endl << std::endl;

      } else {
        out << "  // Accessors for " << fd.name << std::endl;
        out << "  static " << CFieldTypes[fd.type] << " get_" << fd.name
            << "() {" << std::endl;
        out << "    return " << dt.first << "." << fd.name << ";" << std::endl;
        out << "  }" << std::endl;

        out << "  static void set_" << fd.name << "(" << CFieldTypes[fd.type]
            << " const &x) {" << std::endl;
        out << "    " << dt.first << "." << fd.name << " = x;" << std::endl;
        out << "  }" << std::endl << std::endl;
      }
    }

    out << "};" << std::endl << std::endl;
  }

  out << "}\n#endif\n";
}