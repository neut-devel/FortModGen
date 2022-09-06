#include "testmod.h"

#include <cstring>
#include <iomanip>
#include <iostream>

extern "C" {

void cppwrite() {

  auto myinst1 = FortMod::testtype1IF::copy();
  auto myinst2 = FortMod::testtype2IF::copy();

  myinst1.fbool = false;
  myinst1.ffloat = 9.8765;
  myinst1.fdouble = 9.876543210;
  myinst1.set_fstr("string from C++");

  int ctr = 1;
  for (int i = 0; i < 5; ++i) {
    myinst2.ffloata[i] = ctr--;
  }

  ctr = 10;
  for (int i = 0; i < intpar; ++i) {
    myinst2.ffloatapar[i] = ctr--;
  }

  ctr = 100;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      myinst2.ffloat2a[i][j] = ctr--;
    }
  }

  ctr = 1000;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < intpar; ++j) {
      myinst2.ffloat2apar[i][j] = ctr--;
    }
  }

  ctr = 10000;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 2; ++k) {
        myinst2.fint3dim[i][j][k] = ctr--;
      }
    }
  }
  FortMod::testtype1IF::update(myinst1);
  FortMod::testtype2IF::update(myinst2);
}
}
