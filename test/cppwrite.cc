#include "testmod.h"

#include <cstring>
#include <iomanip>
#include <iostream>

extern "C" {

void cppwrite() {

  auto myinst = FortMod::testtypeIF::copy();

  std::cout << ">>>>>>>>>>>>>>>>" << std::endl;
  std::cout << "Writing from C++" << std::endl;
  std::cout << "<<<<<<<<<<<<<<<<" << std::endl;

  std::string tmpstr = "string from C++";
  std::memset(myinst.fstr, '\0', 100);
  std::memcpy(myinst.fstr, tmpstr.c_str(), tmpstr.size() + 1);
  myinst.ffloat = 9.87654321;
  myinst.fdouble = 9.876543210123456789;

  for (int i = 0; i < 5; ++i) {
    myinst.ffloata[i] = 5 - i;
  }

  int ctr = 10;
  for (int i = 0; i < intpar; ++i) {
    myinst.ffloatapar[i] = ctr;
    ctr--;
  }

  ctr = 100;
  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 5; ++i) {
      myinst.ffloat2a[i][j] = ctr;
      ctr--;
    }
  }

  ctr = 200;
  for (int j = 0; j < intpar; ++j) {
    for (int i = 0; i < 5; ++i) {
      myinst.ffloat2apar[i][j] = ctr;
      ctr--;
    }
  }

  FortMod::testtypeIF::update(myinst);
}

void cppsay() {

  std::cout << ">>>>>>>>>>>>>>>" << std::endl;
  std::cout << "Saying from C++" << std::endl;
  std::cout << "---------------" << std::endl;

  auto myinst = FortMod::testtypeIF::copy();

  std::cout << "myinst.fstr: " << myinst.fstr << std::endl;
  std::cout << "myinst.ffloat: " << myinst.ffloat << std::endl;
  std::cout << "myinst.fdouble: " << myinst.fdouble << std::endl;

  std::cout << "myinst.ffloata: [";
  for (int i = 0; i < 5; ++i) {
    std::cout << std::setw(2) << myinst.ffloata[i] << (i != 4 ? ", " : " ");
  }
  std::cout << "]" << std::endl;

  std::cout << "myinst.ffloatapar: [";
  for (int i = 0; i < intpar; ++i) {
    std::cout << std::setw(2) << myinst.ffloatapar[i]
              << (i != (intpar - 1) ? ", " : " ");
  }
  std::cout << "]" << std::endl;

  std::cout << "myinst.ffloat2a: [" << std::endl;
  for (int j = 0; j < 3; ++j) {
    std::cout << "    [ ";
    for (int i = 0; i < 5; ++i) {
      std::cout << std::setw(2) << myinst.ffloat2a[i][j]
                << (i != (5 - 1) ? ", " : " ");
    }
    std::cout << "]" << std::endl;
  }
  std::cout << "]" << std::endl;

  std::cout << "myinst.ffloat2apar: [" << std::endl;
  for (int j = 0; j < intpar; ++j) {
    std::cout << "    [ ";
    for (int i = 0; i < 5; ++i) {
      std::cout << std::setw(2) << myinst.ffloat2apar[i][j]
                << (i != (5 - 1) ? ", " : " ");
    }
    std::cout << "]" << std::endl;
  }
  std::cout << "]" << std::endl;

  std::cout << "<<<<<<<<<<<<<<<" << std::endl;
}
}