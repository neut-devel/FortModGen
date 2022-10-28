#include "testmod.h"

#include "fmt/core.h"

#include <cmath>

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

int main() {
  CPPAssert_float(floatpar,1.2345678);
}