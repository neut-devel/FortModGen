# FortModGen

Can be used to generate consistent Fortran and C/C++ data structures from a toml configuration file for global memory-based interop (not recommended, but we're stuck with it in NEUT). Also generates a simple C/C++ object-ified interface for each data structure.

## Build

Requires a C++17-capable compiler.

```
git clone https://github.com/neut-devel/FortModGen.git
mkdir build && cd build
cmake ../FortModGen
make install -j4
```

## Example Usage

From a `toml` descriptor file, `config.toml` as below:

```toml
[module]

name = "testmod"

parameters = [
  { name = "intpar", type = "integer", value = 2 },
  { name = "stringpar", type = "string", value = "abcde12345" },
  { name = "floatpar", type = "float", value = 1.234 },
]

derivedtypes = [ "testtype",  ]

[module.testtype]
fields = [
  { name = "fstr",  type = "string", size = 100 },
  { name = "ffloat",  type = "float" },
  { name = "fdouble",  type = "double"},
  { name = "ffloata",  type = "float", size = 5 },
  { name = "ffloatapar",  type = "float", size = "intpar" },
  { name = "ffloat2a",  type = "float", size = [3,5] },
  { name = "ffloat2apar",  type = "float", size = ["intpar", 5] },
]

```

Executing `fortmodgen -i config.toml -o testmod` would produce: `testmod.f90` and `testmod.h`:

```fortran
module testmod
  use iso_c_binding

  real(kind=C_FLOAT), parameter :: floatpar = 1.234000
  character(kind=C_CHAR,len=*), parameter :: stringpar = "abcde12345"
  integer(kind=C_INT), parameter :: intpar = 2

  type, bind(C) :: t_testtype
    character(kind=C_CHAR), dimension(101) :: fstr
    real(kind=C_FLOAT) :: ffloat
    real(kind=C_DOUBLE) :: fdouble
    real(kind=C_FLOAT), dimension(5) :: ffloata
    real(kind=C_FLOAT), dimension(2) :: ffloatapar
    real(kind=C_FLOAT), dimension(3, 5) :: ffloat2a
    real(kind=C_FLOAT), dimension(2, 5) :: ffloat2apar

  end type t_testtype

  type (t_testtype), bind(C) :: testtype

  data testtype%fstr(101:101)/C_NULL_CHAR/
  data testtype%ffloata/1, 2.000000, 3.456000, 4, 5/
  data testtype%ffloat2a/1, 2, 3, 4, 5, 6, 7, 8.123450, 9, 10, 11, 12, 13, 14, &
&                        15/

  save
  contains
    function get_testtype_fstr() result(out_str)
      character(len=100) :: out_str
      do i = 1, 100
          out_str(i:i) = testtype%fstr(i)
      end do
    end function

    subroutine set_testtype_fstr(in_str)
      use iso_c_binding
      character(kind=C_CHAR,len=*), intent(in) :: in_str
      integer :: loop_end = 100

      if(len(in_str).lt.100) loop_end = len(in_str)

      do i = 1, loop_end
          testtype%fstr(i) = in_str(i:i)
      end do
      testtype%fstr(100) = C_NULL_CHAR
    end subroutine

end module testmod


```

```c++
#pragma once
#ifdef __cplusplus
#include<iostream>
#include<string>
#include<cstring>

extern "C" {

#endif
#define floatpar 1.234000

#define stringpar "abcde12345"

#define intpar 2

extern struct testtype_t {

  char fstr[101];

#ifdef __cplusplus
  std::string get_fstr() const { return std::string(fstr, 100); }
  void set_fstr(std::string in_str) {
    if (in_str.size() > 100) {
      std::cout
          << "[WARN]: String: \"" << in_str
          << "\", is too large to fit in testtype::fstr, truncated to 100 characters."
          << std::endl;
    }
    std::memcpy(fstr, in_str.c_str(), std::min(size_t(100), in_str.size()));
  }
#endif
    float ffloat;
  double fdouble;
  float ffloata[5];
  float ffloatapar[2];
  float ffloat2a[5][3];
  float ffloat2apar[5][2];

} testtype ;

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
#include <stdlib.h>
#include <string.h>

//C Interface for testtype
inline struct testtype_t *alloc_testtype(){
  return malloc(sizeof(struct testtype_t));
}

inline void free_testtype(struct testtype_t *inst){
  if(inst != NULL){
    free(inst);
  }
}
inline void copy_testtype(struct testtype_t *inst){
  memcpy(inst->fstr,testtype.fstr,sizeof(char)*100);
  inst->ffloat = testtype.ffloat;
  inst->fdouble = testtype.fdouble;
  memcpy(inst->ffloata,testtype.ffloata,sizeof(float)*5);
  memcpy(inst->ffloatapar,testtype.ffloatapar,sizeof(float)*2);
  memcpy(inst->ffloat2a,testtype.ffloat2a,sizeof(float)*15);
  memcpy(inst->ffloat2apar,testtype.ffloat2apar,sizeof(float)*10);
}

inline void update_testtype(struct testtype_t const *inst){
  memcpy(testtype.fstr, inst->fstr,sizeof(char)*100);
  testtype.ffloat = inst->ffloat;
  testtype.fdouble = inst->fdouble;
  memcpy(testtype.ffloata, inst->ffloata,sizeof(float)*5);
  memcpy(testtype.ffloatapar, inst->ffloatapar,sizeof(float)*2);
  memcpy(testtype.ffloat2a, inst->ffloat2a,sizeof(float)*15);
  memcpy(testtype.ffloat2apar, inst->ffloat2apar,sizeof(float)*10);
}


#else
namespace FortMod {

//C++ Interface for testtype
namespace testtypeIF {

inline testtype_t copy(){
  return testtype;
}

inline void update(testtype_t const &inst){
  testtype = inst;
}

}

}
#endif


```

## Limitations

* No string arrays. You can use multi-dimensional character arrays which are represented by the same data-structures, but they don't come with convenience C/Fortran functions for string getting/setting.

## To Do Before V1

* Add other types defined by iso_c_binding
* Try and get rid of the globally accessible instance in the C/C++ interface and get/set a C_PTR version from a Fortran interface