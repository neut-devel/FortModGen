#include "testmod.h"

#include "fmt/core.h"

#include <cmath>
#include <iomanip>

extern "C" {
void cppwrite();
void fortwrite();
}

#define CPPAssert(field, Expected)                                             \
  if (field != Expected) {                                                     \
    std::cout << fmt::format("ASSERT[FAILED]: {}:{}\n\t{}, Read: {} != {}.",   \
                             __FILE__, __LINE__, #field, field, Expected)      \
              << std::endl;                                                    \
    abort();                                                                   \
  }

#define CPPAssert_float(field, Expected)                                       \
  if (std::fabs(field - float(Expected)) > 1E-7) {                             \
    std::cout << fmt::format(                                                  \
                     "ASSERT[FAILED]: {}:{}\n\tfloat {}, Read: {:.8E} != "     \
                     "{:.8E}. Difference = {:.8E}",                            \
                     __FILE__, __LINE__, #field, field, float(Expected),       \
                     std::fabs(field - Expected))                              \
              << std::endl;                                                    \
    abort();                                                                   \
  }

#define CPPAssert_double(field, Expected)                                      \
  if (std::fabs(field - double(Expected)) > 1E-15) {                           \
    std::cout << fmt::format(                                                  \
                     "ASSERT[FAILED]: {}:{}\n\tdouble {}, Read: {:.16E} != "   \
                     "{:.16E}. Difference = {:.8E}",                           \
                     __FILE__, __LINE__, #field, field, double(Expected),      \
                     std::fabs(field - Expected))                              \
              << std::endl;                                                    \
    abort();                                                                   \
  }

void cppassert_cpp() {

  auto myinst1 = FortMod::testtype1IF::copy();
  auto myinst2 = FortMod::testtype2IF::copy();

  CPPAssert(myinst1.fbool, false);
  CPPAssert_float(myinst1.ffloat, 9.8765);
  CPPAssert_double(myinst1.fdouble, 9.876543210);
  CPPAssert(myinst1.fstr, std::string("string from C++"));

  int ctr = 1;
  for (int i = 0; i < 5; ++i) {
    CPPAssert_float(myinst2.ffloata[i], ctr--);
  }

  ctr = 10;
  for (int i = 0; i < intpar; ++i) {
    CPPAssert_float(myinst2.ffloatapar[i], ctr--);
  }

  ctr = 100;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      CPPAssert_float(myinst2.ffloat2a[i][j], ctr--);
    }
  }

  ctr = 1000;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < intpar; ++j) {
      CPPAssert_float(myinst2.ffloat2apar[i][j], ctr--);
    }
  }

  ctr = 10000;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 2; ++k) {
        CPPAssert(myinst2.fint3dim[i][j][k], ctr--);
      }
    }
  }
}

void cppassert_fort() {

  auto myinst1 = FortMod::testtype1IF::copy();
  auto myinst2 = FortMod::testtype2IF::copy();

  CPPAssert(myinst1.fbool, true);
  CPPAssert_float(myinst1.ffloat, 1.2345);
  CPPAssert_double(myinst1.fdouble, 1.234567891);
  CPPAssert(myinst1.fstr, std::string("string from fortran"));

  int ctr = 1;
  for (int i = 0; i < 5; ++i) {
    CPPAssert_float(myinst2.ffloata[i], ctr++);
  }

  ctr = 10;
  for (int i = 0; i < intpar; ++i) {
    CPPAssert_float(myinst2.ffloatapar[i], ctr++);
  }

  ctr = 100;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      CPPAssert_float(myinst2.ffloat2a[i][j], ctr++);
    }
  }

  ctr = 1000;
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < intpar; ++j) {
      CPPAssert_float(myinst2.ffloat2apar[i][j], ctr++);
    }
  }

  ctr = 10000;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 3; ++j) {
      for (int k = 0; k < 2; ++k) {
        CPPAssert(myinst2.fint3dim[i][j][k], ctr++);
      }
    }
  }
}

int main() {
  cppwrite();
  cppassert_cpp();

  fortwrite();
  cppassert_fort();
}