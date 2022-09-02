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
    character(kind=C_CHAR), dimension(100) :: fstr
    real(kind=C_FLOAT) :: ffloat
    real(kind=C_DOUBLE) :: fdouble
    real(kind=C_FLOAT), dimension(5) :: ffloata
    real(kind=C_FLOAT), dimension(intpar) :: ffloatapar
    real(kind=C_FLOAT), dimension(3, 5) :: ffloat2a
    real(kind=C_FLOAT), dimension(intpar, 5) :: ffloat2apar

  end type t_testtype

  type (t_testtype), bind(C) :: testtype

  save
end module testmod

```

```c++
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define floatpar 1.234000

#define stringpar "abcde12345"

#define intpar 2

extern struct testtype_t {

  char fstr[100];
  float ffloat;
  double fdouble;
  float ffloata[5];
  float ffloatapar[intpar];
  float ffloat2a[5][3];
  float ffloat2apar[5][intpar];

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