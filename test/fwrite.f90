module fwrite_mod

  contains 
    subroutine fortwrite() bind(C, name="fortwrite")
      use iso_c_binding
      use iso_fortran_env
      use testmod

      integer :: i, j, ctr

      print *, ">>>>>>>>>>>>>>>>>>>>"
      print *, "Writing from fortran"
      print *, "<<<<<<<<<<<<<<<<<<<<"

      call set_testtype_fstr("hello from fortran")

      testtype%ffloat = 1.234567
      testtype%fdouble = 1.2345678910

      do i = 1, 5
        testtype%ffloata(i) = i
      end do

      do i = 1, intpar
        testtype%ffloatapar(i) = i
      end do
      
      ctr = 10
      do i = 1, 3
        do j = 1, 5
          testtype%ffloat2a(i,j) = ctr
          ctr = ctr + 1
        end do
      end do
      
      ctr = 100
      do i = 1, intpar
        do j = 1, 5
          testtype%ffloat2apar(i,j) = ctr
          ctr = ctr + 1
        end do
      end do
    flush(output_unit)
    end subroutine

    subroutine fortsay() bind(C, name="fortsay")
      use iso_c_binding
      use iso_fortran_env
      use testmod

      integer :: i, j

      print *, ">>>>>>>>>>>>>>>>>>>"
      print *, "Saying from fortran"
      print *, "-------------------"

      call print_testtype()

      print *, "<<<<<<<<<<<<<<<<<<<"

    flush(output_unit)
    end subroutine
end module fwrite_mod