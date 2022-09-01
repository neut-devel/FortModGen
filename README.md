# FortModGen

Generate fortran module files and C/C++ interfaces to the same data structure

# Example Usage

`fortmodgen -i config/example.toml -o testmod`

produces: `testmod.f90` and `testmod.h`.


```
!testmod.f90
module mymod
  use iso_c_binding

  character(kind=C_CHAR),len=*), parameter :: constant_key = "abcde12345"
  integer(kind=C_INT), parameter :: maxlen = 5

  !This is an example type
  !about which we have some commentary
  !
  type, bind(C) :: t_example
    !the string
    character(kind=C_CHAR), dimension(100) :: mystr
    real(kind=C_FLOAT) :: myfloat
    real(kind=C_FLOAT), dimension(3) :: myfloat3
    real(kind=C_DOUBLE), dimension(3, 4) :: mydouble3_4
    real(kind=C_DOUBLE), dimension(3, maxlen) :: mydouble3_maxlen

  end type t_example

  type (t_example), bind(C) :: example

  save
end module mymod

```

```
//testmod.h
#pragma once

extern "C" { 

char* const constant_key = "abcde12345";

int const maxlen = 5;

//This is an example type
//about which we have some commentary
//
extern struct {

  //the string
  char mystr[100];
  float myfloat;
  float myfloat3[3];
  double mydouble3_4[4][3];
  double mydouble3_maxlen[maxlen][3];

} example;

} 


#ifdef __cplusplus

#include <cstring>
#include <algorithm>

namespace mymod {
struct exampleIFace {
  // Accessors for mystr
  static char const * get_mystr() {
    return example.mystr;
  }
  static int get_mystr_capacity() {
    return 100;
  }
  static void set_mystr(char const *x, int n) {
    if((n+1) > 100){
      std::cout << "[WARNING]: When setting mystr, a cstr of length: " << n 
                << " was passed, but mystr only has a capacity of: " 
                << string_capacity << ". It will be truncated." <<  std::endl;
    }
    std::memset(example.mystr, x, std::min(n+1,100));
  }

  // Accessors for myfloat
  static float get_myfloat() {
    return example.myfloat;
  }
  static void set_myfloat(float const &x) {
    example.myfloat = x;
  }

  // Accessors for myfloat3
  static int get_myfloat3_ndims() { return 1; } 
  static int const * get_myfloat3_shape() { 
    static int shape[] = {3};
    return shape; 
  } 
  static float const * get_myfloat3() {
    return example.myfloat3;
  }

  static void set_myfloat3(float const * x) {
    for(int i = 0; i < 3; ++i) {
      example.myfloat3[i] = x[i];
    }
  }

  // Accessors for mydouble3_4
  static int get_mydouble3_4_ndims() { return 2; } 
  static int const * get_mydouble3_4_shape() { 
    static int shape[] = {4, 3};
    return shape; 
  } 
  static double const ** get_mydouble3_4() {
    return example.mydouble3_4;
  }

  static void set_mydouble3_4(double const ** x) {
    for(int i = 0; i < 4; ++i) {
      for(int j = 0; j < 3; ++j) {
        example.mydouble3_4[i][j] = x[i][j];
      }
    }
  }

  // Accessors for mydouble3_maxlen
  static int get_mydouble3_maxlen_ndims() { return 2; } 
  static int const * get_mydouble3_maxlen_shape() { 
    static int shape[] = {5, 3};
    return shape; 
  } 
  static double const ** get_mydouble3_maxlen() {
    return example.mydouble3_maxlen;
  }

  static void set_mydouble3_maxlen(double const ** x) {
    for(int i = 0; i < 5; ++i) {
      for(int j = 0; j < 3; ++j) {
        example.mydouble3_maxlen[i][j] = x[i][j];
      }
    }
  }

};

}
#endif
```