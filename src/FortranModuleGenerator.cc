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

void FortranDerivedTypeField(fmt::ostream &os, FieldDescriptor const &fd,
                             ParameterFields const &parameters) {

  std::string comment = SanitizeComment(fd.comment, "    !");
  if (comment.length()) {
    os.print("    !{}\n", comment);
  }
  os.print("    {}(kind={})", FortranFieldTypes[fd.type],
           FortranFieldKinds[fd.type]);
  if (fd.is_array()) {
    os.print(", dimension(");
    for (int i = 0; i < fd.size.size(); ++i) {
      int dim_size = fd.get_dim_size(i, parameters);
      if (fd.is_string()) { // keep an extra character around that the interface
                            // functions don't use and put a C_NULL_CHAR in it.
        dim_size++;
      }
      os.print("{}{}", dim_size, ((i + 1 == fd.size.size()) ? "" : ", "));
    }
    os.print(")");
  }
  os.print(" :: {}\n", fd.name);
}

std::string DataElementToString(FieldType ft,
                                std::variant<int, double> const &d) {

  if (ft == FieldType::kInteger) {
    return std::to_string(std::get<0>(d));
  } else if (ft == FieldType::kFloat) {
    if (d.index() == 0) { // int
      return std::to_string(std::get<0>(d));
    } else if (d.index() == 1) { // double
      return std::to_string(float(std::get<1>(d)));
    }
  } else if (ft == FieldType::kFloat) {
    if (d.index() == 0) { // int
      return std::to_string(std::get<0>(d));
    } else if (d.index() == 1) { // double
      return std::to_string(std::get<1>(d));
    }
  }
  return "";
}

void FortranDerivedTypeFieldData(fmt::ostream &os, std::string const &dtypename,
                                 FieldDescriptor const &fd,
                                 ParameterFields const &parameters) {

  if (fd.is_string()) { // put a C_NULL_CHAR at the end of any string array
    os.print("  data {0}%{1}({2}:{2})/C_NULL_CHAR/\n", dtypename, fd.name,
             fd.get_size(parameters) + 1);
    return;
  }

  std::string data_fmt = "  data {}%{}/";
  auto indent_size = fmt::formatted_size(data_fmt, dtypename, fd.name);

  std::string indent_str = "";
  for (int i = 0; i < indent_size - 1; ++i) {
    indent_str += " ";
  }

  auto line_length = indent_size;

  os.print(data_fmt, dtypename, fd.name);
  if (fd.is_array()) {
    int ents = std::min(int(fd.data.size()), fd.get_size(parameters));
    for (int i = 0; i < ents; ++i) {

      auto data_el = DataElementToString(fd.type, fd.data[i]);

      if ((line_length + data_el.length() + 1) > 78) {
        line_length = fmt::formatted_size("&\n&{}", indent_str);
        os.print("&\n&{}", indent_str);
      }

      line_length += fmt::formatted_size("{}{}", data_el,
                                         (((i + 1) == ents) ? "/\n" : ", "));
      os.print("{}{}", data_el, (((i + 1) == ents) ? "/\n" : ", "));
    }
  } else {
    os.print("{}/\n", DataElementToString(fd.type, fd.data.front()));
  }
}

void FortranStringAccessor(fmt::ostream &os, std::string const &dtypename,
                           FieldDescriptor const &fd,
                           ParameterFields const &parameters) {

  os.print(R"(
    function get_{0}_{1}() result(out_str)
      character(len={2}) :: out_str
      do i = 1, {2}
          out_str(i:i) = {0}%{1}(i)
      end do
    end function

    subroutine set_{0}_{1}(in_str)
      use iso_c_binding
      character(kind=C_CHAR,len=*), intent(in) :: in_str
      integer :: loop_end = {2}

      if(len(in_str).lt.{2}) loop_end = len(in_str)

      do i = 1, loop_end
          {0}%{1}(i) = in_str(i:i)
      end do
      {0}%{1}({2}) = C_NULL_CHAR
    end subroutine
)",
           dtypename, fd.name, fd.get_size(parameters));
}

void FortranDerivedTypeFooter(fmt::ostream &os, std::string const &dtypename) {
  os.print("\n  end type t_{0}\n\n  type (t_{0}), bind(C) :: {0}\n\n",
           dtypename);
}

void FortranFileFooter(fmt::ostream &os, std::string const &modname) {
  os.print("\nend module {}\n", modname);
}

void GenerateFortranModule(std::string const &fname, std::string const &modname,
                           ParameterFields const &parameters,
                           DerivedTypes const &dtypes) {

  auto out = fmt::output_file(fname);

  FortranFileHeader(out, modname);

  FortranModuleParameters(out, parameters);

  for (auto const &dt : dtypes) {

    FortranDerivedTypeHeader(out, dt.first, dt.second.comment);

    for (auto const &fd : dt.second.fields) {

      FortranDerivedTypeField(out, fd, parameters);
    }

    FortranDerivedTypeFooter(out, dt.first);

    // instance data initialization must come after the instance declaration
    for (auto const &fd : dt.second.fields) {

      if (!fd.data.size() && !fd.is_string()) {
        continue;
      }

      FortranDerivedTypeFieldData(out, dt.first, fd, parameters);
    }
  }

  out.print("\n  save\n  contains\n");
  for (auto const &dt : dtypes) {

    // instance data initialization must come after the instance declaration
    for (auto const &fd : dt.second.fields) {
      if (!fd.is_string()) {
        continue;
      }
      FortranStringAccessor(out, dt.first, fd, parameters);
    }
  }

  FortranFileFooter(out, modname);
}
