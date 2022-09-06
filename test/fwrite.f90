module fwrite_mod

  contains 
    subroutine fortwrite() bind(C, name="fortwrite")
      use iso_c_binding
      use iso_fortran_env
      use testmod

      integer :: i, j, k, ctr

      testtype1%fbool = .true.
      testtype1%ffloat = 1.2345e0
      testtype1%fdouble = 1.234567891d0
      call set_testtype1_fstr("string from fortran")

      ctr = 1
      do i = 1, 5
        testtype2%ffloata(i) = ctr
        ctr = ctr + 1
      end do

      ctr = 10
      do i = 1, intpar
        testtype2%ffloatapar(i) = ctr
        ctr = ctr + 1
      end do
      
      ctr = 100
      do j = 1, 5
        do i = 1, 3
          testtype2%ffloat2a(i,j) = ctr
          ctr = ctr + 1
        end do
      end do
      
      ctr = 1000
      do j = 1, 5
        do i = 1, intpar
          testtype2%ffloat2apar(i,j) = ctr
          ctr = ctr + 1
        end do
      end do

      ctr = 10000
      do k = 1, 4
        do j = 1, 3
          do i = 1, 2
            testtype2%fint3dim(i,j,k) = ctr
            ctr = ctr + 1
          end do
        end do
      end do

    end subroutine
end module fwrite_mod