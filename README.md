# FortModGen

Can be used to generate consistent Fortran and C/C++ data structures, and a consistent C/C++ API from a toml configuration file for global object interop. 

## Example

Turn this

```toml
[module]

name = "testmod"

uses = ["ISO_FORTRAN_ENV", ]

parameters = [
  { name = "intpar", type = "integer", value = 2 },
  { name = "floatpar", type = "float", value = 1.234 },
  { name = "floatparexp", type = "float", value = 1E-8 },
  { name = "floatparsq", type = "float", value = "floatpar*floatpar" },
  { name = "stringpar", type = "string", value = "abcde12345" },
]

derivedtypes = [ "testtype1", "testtype2"  ]

[module.testtype1]
fields = [
  { name = "fbool",  type = "bool", data = true },
  { name = "ffloat",  type = "float" },
  { name = "fdouble",  type = "double"},
  { name = "fstr",  type = "string", size = 100 },
]

[module.testtype2]
fields = [
  { name = "ffloata",  type = "float", size = 5, data = [ 1, 2.0, 3.456, 4, 5 ] },
  { name = "ffloatapar",  type = "float", size = "intpar" },
  { name = "ffloat2a",  type = "float", size = [3,5], data = [ 
    1,2,3,4,5,
    6,7,8.12345,"floatpar",10E-10,
    11,12,13,14,15
   ]},
  { name = "ffloat2apar",  type = "float", size = ["intpar", 5] },
  { name = "fint3dim",  type = "integer", size = [2,3,4] },
]

```

into these:

```Fortran
module testmod
  use iso_c_binding
  use ISO_FORTRAN_ENV

  integer(kind=C_INT), parameter :: intpar = 2
  real(kind=C_FLOAT), parameter :: floatpar = 1.234
  real(kind=C_FLOAT), parameter :: floatparexp = 1e-08
  real(kind=C_FLOAT), parameter :: floatparsq = floatpar*floatpar
  character(kind=C_CHAR,len=*), parameter :: stringpar = "abcde12345"

  type, bind(C) :: t_testtype2
    real(kind=C_FLOAT), dimension(5) :: ffloata
    real(kind=C_FLOAT), dimension(2) :: ffloatapar
    real(kind=C_FLOAT), dimension(3, 5) :: ffloat2a
    real(kind=C_FLOAT), dimension(2, 5) :: ffloat2apar
    integer(kind=C_INT), dimension(2, 3, 4) :: fint3dim

  end type t_testtype2

  type (t_testtype2), save, bind(C) :: testtype2

  data testtype2%ffloata/1, 2, 3.456, 4, 5/
  data testtype2%ffloat2a/1, 2, 3, 4, 5, 6, 7, 8.12345, floatpar, 1e-09, 11, &
&                         12, 13, 14, 15/
  type, bind(C) :: t_testtype1
    logical(kind=C_BOOL) :: fbool
    real(kind=C_FLOAT) :: ffloat
    real(kind=C_DOUBLE) :: fdouble
    character(kind=C_CHAR), dimension(101) :: fstr

  end type t_testtype1

  type (t_testtype1), save, bind(C) :: testtype1

  data testtype1%fbool/.true./
  data testtype1%fstr(101:101)/C_NULL_CHAR/

  contains

    subroutine print_testtype2()  bind(C, name='print_testtype2')
      implicit integer(i-z)

      write (*,*) "testtype2:"
      write (*,"(A)",advance='no') "  float :: ffloata(5)"
      write (*,"(A)", advance='no') "  ["

      do i = 1, 5
        write (*, "(ES10.3E1X)", advance='no') testtype2%ffloata(i)
      end do
      write (*,"(A)") "  ]"

      write (*,"(A)",advance='no') "  float :: ffloatapar(2)"
      write (*,"(A)", advance='no') "  ["

      do i = 1, 2
        write (*, "(ES10.3E1X)", advance='no') testtype2%ffloatapar(i)
      end do
      write (*,"(A)") "  ]"

      write (*,"(A)") "  float :: ffloat2a(3:5)"
      write (*,"(A)") "  ["

      do j = 1, 5
        write (*,"(A,I3X,A)",advance='no') "    ", j, ": ["

        do i = 1, 3
          write (*, "(ES10.3E1X)", advance='no') testtype2%ffloat2a(i,j)
        end do
        write (*,"(A)") "      ],"
      end do

      write (*,"(A)") "  ]"

      write (*,"(A)") "  float :: ffloat2apar(2:5)"
      write (*,"(A)") "  ["

      do j = 1, 5
        write (*,"(A,I3X,A)",advance='no') "    ", j, ": ["

        do i = 1, 2
          write (*, "(ES10.3E1X)", advance='no') testtype2%ffloat2apar(i,j)
        end do
        write (*,"(A)") "      ],"
      end do

      write (*,"(A)") "  ]"

      write (*,"(A)") "  integer :: fint3dim(2:3:4)"
      write (*,"(A)") "  ["

      do k = 1, 4
        write (*,"(A,I3X,A)") "    ", k, ": ["

        do j = 1, 3
          write (*,"(A,I3X,A)",advance='no') "      ", j, ": ["

          do i = 1, 2
            write (*, "(I3X)", advance='no') testtype2%fint3dim(i,j,k)
          end do
          write (*,"(A)") "        ],"
        end do

        write (*,"(A)") "      ],"
      end do

      write (*,"(A)") "  ]"


        write (*,*) "end testtype2"
    end subroutine print_testtype2

    subroutine copy_testtype2(cinst) bind(C, name='copy_testtype2')
      type (c_ptr), value :: cinst
      type (t_testtype2), pointer :: finst

      call C_F_POINTER(cinst,finst)

      finst = testtype2
    end subroutine copy_testtype2

    subroutine update_testtype2(cinst) bind(C, name='update_testtype2')
      type (c_ptr), value :: cinst
      type (t_testtype2), pointer :: finst

      call C_F_POINTER(cinst,finst)

      testtype2 = finst
    end subroutine update_testtype2
    
    function get_testtype1_fstr() result(out_str)
      character(len=100) :: out_str
      do i = 1, 100
          out_str(i:i) = testtype1%fstr(i)
      end do
    end function get_testtype1_fstr

    subroutine set_testtype1_fstr(in_str)
      use iso_c_binding
      character(kind=C_CHAR,len=*), intent(in) :: in_str
      integer :: loop_end = 100

      if(len(in_str).lt.100) loop_end = len(in_str)

      do i = 1, loop_end
          testtype1%fstr(i) = in_str(i:i)
      end do
      testtype1%fstr(100) = C_NULL_CHAR
    end subroutine set_testtype1_fstr

    subroutine print_testtype1()  bind(C, name='print_testtype1')
      implicit integer(i-z)

      write (*,*) "testtype1:"
      write (*,"(A,LX)") "  fbool(bool): ", testtype1%fbool

      write (*,"(A,ES10.3E1X)") "  ffloat(float): ", testtype1%ffloat

      write (*,"(A,ES10.3E1X)") "  fdouble(double): ", testtype1%fdouble

      write (*,"(A,999A)") "  fstr(string): ", testtype1%fstr


        write (*,*) "end testtype1"
    end subroutine print_testtype1

    subroutine copy_testtype1(cinst) bind(C, name='copy_testtype1')
      type (c_ptr), value :: cinst
      type (t_testtype1), pointer :: finst

      call C_F_POINTER(cinst,finst)

      finst = testtype1
    end subroutine copy_testtype1

    subroutine update_testtype1(cinst) bind(C, name='update_testtype1')
      type (c_ptr), value :: cinst
      type (t_testtype1), pointer :: finst

      call C_F_POINTER(cinst,finst)

      testtype1 = finst
    end subroutine update_testtype1
    
end module testmod

```

```C++
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
#include <iostream>
#include <string>
#include <cstring>

extern "C" {

#endif

static int const intpar = 2;

static float const floatpar = 1.234;

static float const floatparexp = 1e-08;

static float const floatparsq = floatpar*floatpar;

static char const * stringpar = "abcde12345";


#ifdef FORTMODGEN_EXPOSE_GLOBAL_INSTANCE
extern
#endif
struct testtype2_t {

  float ffloata[5];
  float ffloatapar[2];
  float ffloat2a[5][3];
  float ffloat2apar[5][2];
  int fint3dim[4][3][2];

}
#ifdef FORTMODGEN_EXPOSE_GLOBAL_INSTANCE
testtype2
#endif
;

#ifdef FORTMODGEN_EXPOSE_GLOBAL_INSTANCE
extern
#endif
struct testtype1_t {

  _Bool fbool;
  float ffloat;
  double fdouble;
  char fstr[101];

#ifdef __cplusplus
  std::string get_fstr() const { return std::string(fstr, 100); }
  void set_fstr(std::string in_str) {
    if (in_str.size() > 100) {
      std::cout
          << "[WARN]: String: \"" << in_str
          << "\", is too large to fit in testtype1::fstr, truncated to 100 characters."
          << std::endl;
    }
    std::memcpy(fstr, in_str.c_str(), std::min(size_t(100), in_str.size()));
  }
#endif

}
#ifdef FORTMODGEN_EXPOSE_GLOBAL_INSTANCE
testtype1
#endif
;
#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
#include <stdlib.h>
#include <string.h>


//Fortran function declarations for struct interface for testtype2
void copy_testtype2(void *);
void update_testtype2(void *);
void print_testtype2();

//C memory management helpers for testtype2
inline struct testtype2_t *alloc_testtype2(){
  return malloc(sizeof(struct testtype2_t));
}

inline void free_testtype2(struct testtype2_t *inst){
  if(inst != NULL){
    free(inst);
  }
}


//Fortran function declarations for struct interface for testtype1
void copy_testtype1(void *);
void update_testtype1(void *);
void print_testtype1();

//C memory management helpers for testtype1
inline struct testtype1_t *alloc_testtype1(){
  return malloc(sizeof(struct testtype1_t));
}

inline void free_testtype1(struct testtype1_t *inst){
  if(inst != NULL){
    free(inst);
  }
}

#endif

#ifdef __cplusplus
namespace FortMod {

//C++ Interface for testtype2

extern "C" {
  //Fortran function declarations for struct interface for testtype2
  void copy_testtype2(void *);
  void update_testtype2(void *);
  void print_testtype2();
}

namespace testtype2IF {

inline testtype2_t copy(){
  testtype2_t inst;
  copy_testtype2(&inst);
  return inst;
}

inline void update(testtype2_t inst){
  update_testtype2(&inst);
}

}


//C++ Interface for testtype1

extern "C" {
  //Fortran function declarations for struct interface for testtype1
  void copy_testtype1(void *);
  void update_testtype1(void *);
  void print_testtype1();
}

namespace testtype1IF {

inline testtype1_t copy(){
  testtype1_t inst;
  copy_testtype1(&inst);
  return inst;
}

inline void update(testtype1_t inst){
  update_testtype1(&inst);
}

}

}
#endif

#ifndef __cplusplus
#include <stdio.h>
#else
#include <cstdio>
#define printf std::printf
#endif

#ifdef __cplusplus
extern "C" {
#endif

inline void cprint_testtype2(){

#ifndef __cplusplus
  struct testtype2_t testtype2_local_inst;
  copy_testtype2(&testtype2_local_inst);
#else
  auto testtype2_local_inst = FortMod::testtype2IF::copy();
#endif
  
  printf("testtype2:\n");
  printf("  float ffloata[5]: ");
  printf("  [ ");

  for(int i = 0; i < 5; ++i) {
    printf("%.3E%s",testtype2_local_inst.ffloata[i], ((i+1) == 5) ? " " : ", " );
  }
  printf("  ]\n");

  printf("  float ffloatapar[2]: ");
  printf("  [ ");

  for(int i = 0; i < 2; ++i) {
    printf("%.3E%s",testtype2_local_inst.ffloatapar[i], ((i+1) == 2) ? " " : ", " );
  }
  printf("  ]\n");

  printf("  float ffloat2a[5][3]: \n");
  printf("  [ \n");

  for(int j = 0; j < 5; ++j) {
    printf("   %d: [ ", j);

      for(int i = 0; i < 3; ++i) {
        printf("%.3E%s",testtype2_local_inst.ffloat2a[j][i], ((i+1) == 3) ? " " : ", " );
      }
    printf("      ],\n");
  }

  printf("  ]\n");

  printf("  float ffloat2apar[5][2]: \n");
  printf("  [ \n");

  for(int j = 0; j < 5; ++j) {
    printf("   %d: [ ", j);

      for(int i = 0; i < 2; ++i) {
        printf("%.3E%s",testtype2_local_inst.ffloat2apar[j][i], ((i+1) == 2) ? " " : ", " );
      }
    printf("      ],\n");
  }

  printf("  ]\n");

  printf("  integer fint3dim[4][3][2]: \n");
  printf("  [ \n");

  for(int k = 0; k < 4; ++k) {
    printf("   %d: [ \n", k);

      for(int j = 0; j < 3; ++j) {
        printf("       %d: [ ", j);

          for(int i = 0; i < 2; ++i) {
            printf("%d%s",testtype2_local_inst.fint3dim[k][j][i], ((i+1) == 2) ? " " : ", " );
          }
        printf("          ],\n");
      }

    printf("      ],\n");
  }

  printf("  ]\n");


  printf("end testtype2\n");
}

#ifdef __cplusplus
}
#undef printf
#endif

#ifndef __cplusplus
#include <stdio.h>
#else
#include <cstdio>
#define printf std::printf
#endif

#ifdef __cplusplus
extern "C" {
#endif

inline void cprint_testtype1(){

#ifndef __cplusplus
  struct testtype1_t testtype1_local_inst;
  copy_testtype1(&testtype1_local_inst);
#else
  auto testtype1_local_inst = FortMod::testtype1IF::copy();
#endif
  
  printf("testtype1:\n");
  printf("  bool fbool: %d\n", testtype1_local_inst.fbool);

  printf("  float ffloat: %.3E\n", testtype1_local_inst.ffloat);

  printf("  double fdouble: %.3E\n", testtype1_local_inst.fdouble);

  printf("  string fstr: %s\n", testtype1_local_inst.fstr);


  printf("end testtype1\n");
}

#ifdef __cplusplus
}
#undef printf
#endif

```

Note the auto-generated copy/update functions so that you can work with copies of the global Fortran instance in C/C++ and control when an update happens. Additionally, helper functions for string get/set and for semi-pretty-printing the global instance are provided in both the Fortran and C/C++ APIs for each generated type/instance.

## Build

Requires a C++17-capable compiler.

```
git clone https://github.com/neut-devel/FortModGen.git
mkdir build && cd build
cmake ../FortModGen
make install -j4
```

### Testing

Run standard tests like:

```
git clone https://github.com/neut-devel/FortModGen.git
mkdir build && cd build
cmake ../FortModGen -DFORTMODGEN_TEST_ENABLED=On
make && make test
```

These test module and C/C++ API code generation of an example module descriptor, [`test/testmod.toml`](test/testmod.toml) against two hand-written test applications [`test/ftest.f90`](test/ftest.f90) and [`test/cppwrite.cc`](test/cppwrite.cc) setting all the fields of a test structure from a fortran subroutine and a C++ function can be asserted to be the expected values in both direction (fortran:set -> fortran:assert, fortran:set -> cpp:assert, cpp:set -> fortran:assert, and cpp:set -> cpp:assert).

## Incorporating in your Project

If you use a CMake build system, then `include(CPM)` and the below to your CMakeLists.txt:

```
CPMFindPackage(
    NAME FortModGen
    GIT_TAG v1.0.0
    GITHUB_REPOSITORY neut-devel/FortModGen
)
include(CompileFortModGenTypes)
```

This will also include the `CompileFortModGenTypes` CMake function, which is used like:

```
FORTMODGEN(
      MOD_DESCRIPTOR_FILE my_descriptor.toml
      MOD_OUTPUT_STUB my_generated_source_stub)
```

This will set up correct dependencies on the input toml file and the generated `my_generated_source_stub.f90` and `my_generated_source_stub.h` API files.

## Limitations

* No string arrays. You can use multi-dimensional character arrays which are represented by the same data-structures, but they don't come with convenience C/Fortran functions for string getting/setting.