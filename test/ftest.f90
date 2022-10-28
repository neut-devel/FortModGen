subroutine assert_str(msg, a, b)
  use iso_c_binding
  character(len=*) :: msg
  character(kind=C_CHAR,len=*) :: a, b

  if(len(a).lt.len(b)) then
    print *, msg, " len(a): ", len(a), " .lt. ", len(b)
    call exit(1)
  end if

  do i = 1, len(b)

    if(a(i:i).ne.b(i:i)) then
      print *, msg, " at character: ", i, " ", a(i:i), " .ne. ", b(i:i)
      call exit(1)
    end if

  end do
end subroutine

subroutine assert_float(msg, a, b)
  use iso_c_binding
  character(len=*) :: msg
  real(kind=C_FLOAT) :: a, b

  if(a.ne.b) then
    print *, msg, a, ".ne.", b
    call exit(1)
  end if
end subroutine

subroutine assert_int(msg, a, b)
  use iso_c_binding
  character(len=*) :: msg
  integer(kind=C_INT) :: a, b

  if(a.ne.b) then
    print *, msg, a, ".ne.", b
    call exit(1)
  end if
end subroutine

subroutine assert_double(msg, a, b)
  use iso_c_binding
  character(len=*) :: msg
  real(kind=C_DOUBLE) :: a, b

  if(a.ne.b) then
    print *, msg, a, ".ne.", b
    call exit(1)
  end if
end subroutine

subroutine assert_logical(msg, a, b)
  use iso_c_binding
  character(len=*) :: msg
  logical(kind=C_BOOL) :: a, b

  if(a.neqv.b) then
    print *, msg, a, ".ne.", b
    call exit(1)
  end if
end subroutine

subroutine fortassert_fort()
  use testmod
  use iso_c_binding
  integer :: i, j, k, ctr
  real(kind=C_FLOAT) :: helper_float
  logical(kind=C_BOOL) :: helper_bool

  helper_bool = .true.
  call assert_logical("ASSERT[FAILED] testtype1%fbool", testtype1%fbool, helper_bool)
  
  call assert_float("ASSERT[FAILED] testtype1%ffloat", testtype1%ffloat, 1.2345678e0)
  call assert_double("ASSERT[FAILED] testtype1%fdouble", testtype1%fdouble, 1.234567891123456d0)
  call assert_str("ASSERT[FAILED] testtype1%fstr", get_testtype1_fstr(), "string from fortran")

  ctr = 1
  do i = 1, 5
    helper_float = ctr
    call assert_float("ASSERT[FAILED] testtype2%ffloata(i)",testtype2%ffloata(i),helper_float)
    ctr = ctr + 1
  end do

  ctr = 10
  do i = 1, intpar
    helper_float = ctr
    call assert_float("ASSERT[FAILED] testtype2%ffloatapar(i)",testtype2%ffloatapar(i),helper_float)
    ctr = ctr + 1
  end do
  
  ctr = 100
  do j = 1, 5
    do i = 1, 3
      helper_float = ctr
      call assert_float("ASSERT[FAILED] testtype2%ffloat2a(i,j)",testtype2%ffloat2a(i,j),helper_float)
      ctr = ctr + 1
    end do
  end do
  
  ctr = 1000
  do j = 1, 5
    do i = 1, intpar
      helper_float = ctr
      call assert_float("ASSERT[FAILED] testtype2%ffloat2apar(i,j)",testtype2%ffloat2apar(i,j),helper_float)
      ctr = ctr + 1
    end do
  end do

  ctr = 10000
  do k = 1, 4
    do j = 1, 3
      do i = 1, 2
        call assert_int("ASSERT[FAILED] testtype2%fint3dim(i,j,k)",testtype2%fint3dim(i,j,k),ctr)
        ctr = ctr + 1
      end do
    end do
  end do
end subroutine

subroutine fortassert_cpp()
  use testmod
  use iso_c_binding
  integer :: i, j, k, ctr
  real(kind=C_FLOAT) :: helper_float
  logical(kind=C_BOOL) :: helper_bool

  helper_bool = .false.
  call assert_logical("ASSERT[FAILED] testtype1%fbool", testtype1%fbool, helper_bool)
  
  call assert_float("ASSERT[FAILED] testtype1%ffloat", testtype1%ffloat, 8.7654321e0)
  call assert_double("ASSERT[FAILED] testtype1%fdouble", testtype1%fdouble, 6.543210987654321d0)
  call assert_str("ASSERT[FAILED] testtype1%fstr", get_testtype1_fstr(), "A slightly longer string from C++")

  ctr = 1
  do i = 1, 5
    helper_float = ctr
    call assert_float("ASSERT[FAILED] testtype2%ffloata(i)",testtype2%ffloata(i),helper_float)
    ctr = ctr - 1
  end do

  ctr = 10
  do i = 1, intpar
    helper_float = ctr
    call assert_float("ASSERT[FAILED] testtype2%ffloatapar(i)",testtype2%ffloatapar(i),helper_float)
    ctr = ctr - 1
  end do
  
  ctr = 100
  do j = 1, 5
    do i = 1, 3
      helper_float = ctr
      call assert_float("ASSERT[FAILED] testtype2%ffloat2a(i,j)",testtype2%ffloat2a(i,j),helper_float)
      ctr = ctr - 1
    end do
  end do
  
  ctr = 1000
  do j = 1, 5
    do i = 1, intpar
      helper_float = ctr
      call assert_float("ASSERT[FAILED] testtype2%ffloat2apar(i,j)",testtype2%ffloat2apar(i,j),helper_float)
      ctr = ctr - 1
    end do
  end do

  ctr = 10000
  do k = 1, 4
    do j = 1, 3
      do i = 1, 2
        call assert_int("ASSERT[FAILED] testtype2%fint3dim(i,j,k)",testtype2%fint3dim(i,j,k),ctr)
        ctr = ctr - 1
      end do
    end do
  end do
end subroutine

program ftest
  use testmod
  use fwrite_mod

  interface
    subroutine cppwrite() bind(c,name='cppwrite')
    end subroutine
  end interface

  call fortwrite()
  call fortassert_fort()

  call cppwrite()
  call fortassert_cpp()

end program