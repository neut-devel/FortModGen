#include "FortranModuleGenerator.h"
#include "utils.h"

#include "fmt/os.h"

#include <map>

std::map<FieldType, std::string> FortranFieldTypes = {
    {FieldType::kInteger, "integer"},     {FieldType::kString, "character"},
    {FieldType::kCharacter, "character"}, {FieldType::kFloat, "real"},
    {FieldType::kDouble, "real"},         {FieldType::kBool, "logical"},
};

std::map<FieldType, std::string> FortranFieldKinds = {
    {FieldType::kInteger, "C_INT"},    {FieldType::kString, "C_CHAR"},
    {FieldType::kCharacter, "C_CHAR"}, {FieldType::kFloat, "C_FLOAT"},
    {FieldType::kDouble, "C_DOUBLE"},  {FieldType::kBool, "C_BOOL"},
};

std::map<FieldType, std::string> FortranPrintFormatSpecifier = {
    {FieldType::kInteger, "I3X"},      {FieldType::kString, "999A"},
    {FieldType::kCharacter, "A"},     {FieldType::kFloat, "ES10.3E1X"},
    {FieldType::kDouble, "ES10.3E1X"}, {FieldType::kBool, "LX"},
};

void FortranFileHeader(fmt::ostream &os, std::string const &modname,
                       std::vector<std::string> const &Uses) {
  os.print("module {}\n  use iso_c_binding\n", modname);
  for (auto const &u : Uses) {
    os.print("  use {}\n", u);
  }
  os.print("\n");
}

void FortranModuleParameters(fmt::ostream &os,
                             ParameterFields const &ParameterFieldDescriptors) {
  for (auto const &p : ParameterFieldDescriptors) {
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
                                FieldDescriptor::data_element_type const &d) {

  if (ft == FieldType::kInteger) {
    if (d.index() == 0) { // int
      return fmt::format("{}", int(std::get<0>(d)));
    } else if (d.index() == 1) { // double
      return fmt::format("{}", int(std::get<1>(d)));
    } else if (d.index() == 2) { // string
      return fmt::format("{}", std::get<2>(d));
    }

  } else if (ft == FieldType::kFloat) {
    if (d.index() == 0) { // int
      return fmt::format("{}", std::get<0>(d));
    } else if (d.index() == 1) { // double
      return fmt::format("{:g}", float(std::get<1>(d)));
    } else if (d.index() == 2) { // string
      return fmt::format("{}", std::get<2>(d));
    }
  } else if (ft == FieldType::kDouble) {
    if (d.index() == 0) { // int
      return fmt::format("{}", std::get<0>(d));
    } else if (d.index() == 1) { // double
      return fmt::format("{:g}", std::get<1>(d));
    } else if (d.index() == 2) { // string
      return fmt::format("{}", std::get<2>(d));
    }
  } else if (ft == FieldType::kBool) {
    if (d.index() == 0) { // int
      return std::get<0>(d) ? ".true." : ".false.";
    } else {
      std::cout << "[ERROR]: Invalid data type variant index: " << d.index()
                << ", expected 0 == int for Field of type bool." << std::endl;
      abort();
    }
  }
  std::cout << "[ERROR]: Cannot transcribe FieldType: " << ft << " to data."
            << std::endl;
  abort();
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

  os.print(R"-(
    function get_{0}_{1}() result(out_str)
      use iso_c_binding
      implicit none

      character(len=:), allocatable :: out_str
      integer :: loop_end = 0, i

      do i = {2}, 1, -1 
          if (.not.(({0}%{1}(i).eq.' ').or.({0}%{1}(i).eq.C_NULL_CHAR))) then
            loop_end = i
            exit
          end if
      end do

      if (.not.allocated(out_str)) then
        allocate(character(len=loop_end) :: out_str)
      end if

      do i = 1, loop_end
          out_str(i:i) = {0}%{1}(i)
      end do

    end function get_{0}_{1}

    subroutine set_{0}_{1}(in_str)
      use iso_c_binding
      implicit none

      character(kind=C_CHAR,len=*), intent(in) :: in_str
      integer :: loop_end = 0, i

      ! blank out the string (but don't flatten the secret C_NULL_CHAR backstop)
      {0}%{1}(1:{2}) = ' '

      do i = len(in_str), 1, -1
        if (.not.((in_str(i:i).eq.' ').or.(in_str(i:i).eq.C_NULL_CHAR))) then
          loop_end = i
          exit
        end if
      end do

      if (loop_end.gt.{2}) loop_end = {2}

      ! copy the relevant characters
      do i = 1, loop_end
          {0}%{1}(i) = in_str(i:i)
      end do

      ! put a C_NULL_CHAR after the last copied character
      {0}%{1}(loop_end+1) = C_NULL_CHAR
    end subroutine set_{0}_{1}
)-",
           dtypename, fd.name, fd.get_size(parameters));
}

void FortranDerivedTypeFooter(fmt::ostream &os, std::string const &dtypename) {
  os.print("\n  end type t_{0}\n\n  type (t_{0}), save, bind(C) :: {0}\n\n",
           dtypename);
}

void FortranPrintArrayRecursiveHelper(
    fmt::ostream &os, std::string const &dtypename,
    ParameterFields const &parameters,
    decltype(DerivedTypes::mapped_type::fields)::value_type const &fd, int d,
    std::string index_string, std::string indent) {

  if (d == 0) { // we actually print a row

    os.print(R"-(
{0}do {2} = 1, {3}
{0}  write (*, "({7})", advance='no') {4}%{5}({6})
{0}end do)-",
             indent, indent.substr(0, indent.size() - 2), char('i' + d),
             fd.get_dim_size(d, parameters), dtypename, fd.name, index_string,
             FortranPrintFormatSpecifier[fd.type]);

  } else {
    os.print(R"-(
{0}do {2} = 1, {3}
{0}  write (*,"(A,I3X,A)"{4}) "{1}", {2}, ": ["
)-",
             indent, indent.substr(0, indent.size() - 2), char('i' + d),
             fd.get_dim_size(d, parameters), (d == 1) ? ",advance='no'" : "");
    FortranPrintArrayRecursiveHelper(os, dtypename, parameters, fd, d - 1,
                                     index_string, indent + "  ");

    os.print(R"-(
{0}  write (*,"(A)") "{1}  ],"
{0}end do
)-",
             indent, indent.substr(0, indent.size() - 2));
  }
}

void FortranDerivedTypeInstancePrint(fmt::ostream &os,
                                     std::string const &dtypename,
                                     ParameterFields const &parameters,
                                     decltype(DerivedTypes::mapped_type::fields)
                                         const &fields) {

  os.print(R"(
    subroutine print_{0}()  bind(C, name='print_{0}')
      implicit integer(i-z)

      write (*,*) "{0}:"
)",
           dtypename);

  for (auto const &fd : fields) {

    if (fd.is_array() && !fd.is_string()) {
      os.print(R"-(      write (*,"(A)"{3}) "  {1} :: {0}({2})")-", fd.name,
               to_string(fd.type), fd.get_fort_shape_str(parameters),
               (fd.size.size() > 1) ? "" : ",advance='no'");

      std::stringstream index_string("");
      for (int i = 0; i < fd.size.size(); ++i) {
        index_string << char('i' + i) << ((i + 1) == fd.size.size() ? "" : ",");
      }

      if (fd.size.size() > 1) {
        os.print(R"-(
      write (*,"(A)") "  ["
)-");
      } else {
        os.print(R"-(
      write (*,"(A)", advance='no') "  ["
)-");
      }

      FortranPrintArrayRecursiveHelper(os, dtypename, parameters, fd,
                                       (fd.size.size() - 1), index_string.str(),
                                       "      ");
      os.print(R"-(
      write (*,"(A)") "  ]"

)-");

    } else {
      os.print(R"-(      write (*,"(A,{3})") "  {1}({2}): ", {0}%{1}

)-",
               dtypename, fd.name, to_string(fd.type),
               FortranPrintFormatSpecifier[fd.type]);
    }
  }

  os.print(R"(
        write (*,*) "end {0}"
    end subroutine print_{0}
)",
           dtypename);
}

void FortranDerivedTypeInstanceAccessors(fmt::ostream &os,
                                         std::string const &dtypename) {

  os.print(R"(
    subroutine copy_{0}(cinst) bind(C, name='copy_{0}')
      type (c_ptr), value :: cinst
      type (t_{0}), pointer :: finst

      call C_F_POINTER(cinst,finst)

      finst = {0}
    end subroutine copy_{0}

    subroutine update_{0}(cinst) bind(C, name='update_{0}')
      type (c_ptr), value :: cinst
      type (t_{0}), pointer :: finst

      call C_F_POINTER(cinst,finst)

      {0} = finst
    end subroutine update_{0}
    )",
           dtypename);
}

void FortranFileFooter(fmt::ostream &os, std::string const &modname) {
  os.print("\nend module {}\n", modname);
}

void GenerateFortranModule(std::string const &fname, std::string const &modname,
                           ParameterFields const &parameters,
                           DerivedTypes const &dtypes,
                           std::vector<std::string> const &Uses) {

  auto out = fmt::output_file(fname);

  FortranFileHeader(out, modname, Uses);

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

  out.print("\n  contains\n");
  for (auto const &dt : dtypes) {

    // instance data initialization must come after the instance declaration
    for (auto const &fd : dt.second.fields) {
      if (!fd.is_string()) {
        continue;
      }
      FortranStringAccessor(out, dt.first, fd, parameters);
    }

    FortranDerivedTypeInstancePrint(out, dt.first, parameters,
                                    dt.second.fields);
    FortranDerivedTypeInstanceAccessors(out, dt.first);
  }

  FortranFileFooter(out, modname);
}
