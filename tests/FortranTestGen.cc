#include "testgen.h"
#include "types.h"

#include <fstream>

void GenFortranTest(std::string const &fname, std::string const &modname,
                    DerivedTypes const &dtypes) {

  std::ofstream fortran_test_out(fname);
  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;
    fortran_test_out << "subroutine fread_" << dtypename << std::endl;
    fortran_test_out << "  use " << modname << std::endl << std::endl;
    fortran_test_out << "  print *, \"from Fortran\"" << std::endl;
    fortran_test_out << "  print *, \"--" << dtypename << ":{\" " << std::endl;
    for (auto const &fieldpair : dtype.second.fields) {
      auto const &field = fieldpair.second;
      fortran_test_out << "  print *, \"  " << field.name << ": \", "
                       << dtypename << "%" << field.name << std::endl;
    }
    fortran_test_out << "  print *, \"}\" " << std::endl << std::endl;
    fortran_test_out << "end subroutine fread_" << dtypename << std::endl
                     << std::endl;
  }

  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;
    fortran_test_out << "subroutine fwrite_" << dtypename << std::endl;
    fortran_test_out << "  use " << modname << std::endl << std::endl;
    fortran_test_out << "  integer :: i " << std::endl;

    for (auto const &fieldpair : dtype.second.fields) {
      auto const &field = fieldpair.second;
      if (field.is_string()) {
        std::string test_string = "string from fortran";

        fortran_test_out << "  character(*), parameter :: " << field.name
                         << "_tmp = \"" << test_string << "\"" << std::endl;
      }
    }

    fortran_test_out << "  print *, \"fwrite_" << dtypename << "()\""
                     << std::endl
                     << std::endl;

    for (auto const &fieldpair : dtype.second.fields) {
      auto const &field = fieldpair.second;
      if (field.is_string()) {
        fortran_test_out << "  do i = 1, size(" << dtypename << "%"
                         << field.name << ")" << std::endl;
        fortran_test_out << "    if ( i > len(" << field.name << "_tmp) ) then"
                         << std::endl;
        fortran_test_out << "      " << dtypename << "%" << field.name
                         << "(i) = C_NULL_CHAR" << std::endl;
        fortran_test_out << "    else" << std::endl;
        fortran_test_out << "      " << dtypename << "%" << field.name
                         << "(i) = " << field.name << "_tmp(i:i)" << std::endl;
        fortran_test_out << "    end if" << std::endl;
        fortran_test_out << "  end do" << std::endl;

      } else if (field.is_array()) {
        fortran_test_out << "  do i = 1, size(" << dtypename << "%"
                         << field.name << ")" << std::endl;
        fortran_test_out << "    " << dtypename << "%" << field.name
                         << "(i) = (size(" << dtypename << "%" << field.name
                         << ") - i)" << std::endl;
        fortran_test_out << "  end do" << std::endl;
      } else {
        fortran_test_out << "  " << dtypename << "%" << field.name << " = 321; "
                         << std::endl;
      }
    }

    fortran_test_out << "end subroutine fwrite_" << dtypename << std::endl
                     << std::endl;
  }

  fortran_test_out << "program test_" << modname << std::endl;
  fortran_test_out << "  use iso_c_binding" << std::endl << std::endl;
  fortran_test_out << "  interface" << std::endl << std::endl;
  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;
    fortran_test_out << "    subroutine cread_" << dtypename
                     << " () bind (C, name='cread_" << dtypename << "')"
                     << std::endl;
    fortran_test_out << "    end subroutine cread_" << dtypename << std::endl;
    fortran_test_out << "    subroutine cwrite_" << dtypename
                     << " () bind (C, name='cwrite_" << dtypename << "')"
                     << std::endl;
    fortran_test_out << "    end subroutine cwrite_" << dtypename << std::endl
                     << std::endl;
  }
  fortran_test_out << "  end interface" << std::endl << std::endl;

  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;
    fortran_test_out << "  call fread_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call cwrite_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call fread_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call cread_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call fwrite_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call fread_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
    fortran_test_out << "  call cread_" << dtypename << std::endl;
    fortran_test_out << "  print *, \"\"" << std::endl;
  }

  fortran_test_out << "end program test_" << modname << std::endl;
}

void GenCTest(std::string const &fname, std::string const &modname,
              DerivedTypes const &dtypes) {

  std::ofstream c_test_out(fname);

  c_test_out << "#include \"" << modname << ".h\"" << std::endl;

  c_test_out << R"(
#include <cstring>
#include <iostream>

template<typename T, size_t N>
int arrlen(T (&arr)[N]){
  return N;
}

extern "C" {
)";

  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;

    c_test_out << "void fread_" << dtypename << "();" << std::endl;
    c_test_out << "void fwrite_" << dtypename << "();\n" << std::endl;

    c_test_out << "void cread_" << dtypename << "(){" << std::endl;
    c_test_out << "  std::cout << \"from C++\" << std::endl;" << std::endl;
    c_test_out << "  std::cout << \"--" << dtypename << ":{\" << std::endl;"
               << std::endl;

    for (auto const &fieldpair : dtype.second.fields) {
      auto const &field = fieldpair.second;
      c_test_out << "  std::cout << \"  " << field.name << "\" << \": \" ";

      if ((field.is_array()) && (field.type != FieldType::kString)) {
        c_test_out << " << \"[ \";" << std::endl;

        c_test_out << "  for (int i = 0; i < arrlen(" << dtypename << "."
                   << field.name << "); ++i) {" << std::endl;
        c_test_out << "    std::cout << " << dtypename << "." << field.name
                   << "[i] << \", \";" << std::endl;
        c_test_out << "  }\n  std::cout << \"]\" << std::endl;" << std::endl;
      } else {
        c_test_out << " << " << dtypename << "." << field.name
                   << " << std::endl;" << std::endl;
      }
    }
    c_test_out << "  std::cout << \"}\" << std::endl; " << std::endl;
    c_test_out << "  }\n" << std::endl;

    c_test_out << "void cwrite_" << dtypename << "(){" << std::endl;
    c_test_out << "  std::cout << \"cwrite_" << dtypename
               << "()\" << std::endl; " << std::endl;
    for (auto const &fieldpair : dtype.second.fields) {
      auto const &field = fieldpair.second;
      if (field.is_string()) {
        std::string test_string = "testing string string with spaces";
        c_test_out << "  std::memcpy(" << dtypename << "." << field.name
                   << ", \"" << test_string << "\", "
                   << "std::min(" << test_string.size() + 1 << ", arrlen("
                   << dtypename << "." << field.name << ")));" << std::endl;

      } else if (field.is_array()) {
        c_test_out << "  for (int i = 0; i < arrlen(" << dtypename << "."
                   << field.name << "); ++i) {" << std::endl;
        c_test_out << "    " << dtypename << "." << field.name << "[i] = i;"
                   << std::endl;
        c_test_out << "  }" << std::endl;
      } else {
        c_test_out << "  " << dtypename << "." << field.name << " = 123; "
                   << std::endl;
      }
    }
    c_test_out << "}\n" << std::endl;
  }
  c_test_out << "}\n" << std::endl;

  c_test_out << "#ifdef INC_MAIN\n" << std::endl;

  c_test_out << "int main(){" << std::endl;
  for (auto const &dtype : dtypes) {
    std::string dtypename = dtype.first;
    c_test_out << "cread_" << dtypename << "();" << std::endl;
    c_test_out << "cwrite_" << dtypename << "();" << std::endl;
    c_test_out << "cread_" << dtypename << "();" << std::endl << std::endl;
  }
  c_test_out << "}" << std::endl;

  c_test_out << "#endif\n" << std::endl;
}

void GenSteeringScript(std::string const &fname, std::string const &modname) {
  std::ofstream testsh_out(fname);

  testsh_out << "#!/bin/bash" << std::endl;
  testsh_out << "set -x" << std::endl;
  testsh_out << "set -e" << std::endl;
  testsh_out << "gfortran -Werror -Wall -c " << modname << ".f90" << std::endl;
  testsh_out << "g++ -Werror -Wall -I. -c " << modname << "_test.cc"
             << std::endl;

  testsh_out << "g++ -Werror -Wall -I. -DINC_MAIN -o ctest_" << modname << " "
             << modname << "_test.cc " << modname << ".o" << std::endl;
  testsh_out << "./ctest_" << modname << std::endl;

  testsh_out << "gfortran -Werror -Wall -I. -o fortrantest_" << modname << " "
             << modname << "_test.f90 " << modname << ".o " << modname
             << "_test.o -lstdc++" << std::endl;
  testsh_out << "./fortrantest_" << modname << std::endl;
}