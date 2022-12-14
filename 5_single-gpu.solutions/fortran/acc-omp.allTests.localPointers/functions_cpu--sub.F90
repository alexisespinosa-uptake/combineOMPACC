      submodule (functions_cpu) sub
      use globals
      implicit none
      contains
! ----------------------------------
         module procedure init
         integer i,j
!           initilising:
            do j=1,GRIDY+2
               do i=1,GRIDX+2
                  U(i,j)=0.0
               end do
            end do
!           these boundary conditions never change throughout run

!           set left side to 0 and right to a linear increase
            do i=1,GRIDX+2
               U(i,1)=0.0
               U(i,GRIDY+2)=(128.0/GRIDX)*(i-1)
            end do

!           set top to 0 and bottom to linear increase
            do j=1,GRIDY+2
               U(1,j)=0.0
               U(GRIDY+2,j)=(128.0/GRIDY)*(j-1)
            end do
         end procedure init
! ----------------------------------

      end submodule sub
