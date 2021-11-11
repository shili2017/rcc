# interrupts and exceptions in supervisor mode

.altmacro
.macro SAVE_GP n
    sd x\n, \n*8(sp)
.endm
.macro LOAD_GP n
    ld x\n, \n*8(sp)
.endm

  .globl trap_from_kernel
  .globl kernelvec
  .align 4
kernelvec:
  addi sp, sp, -256
  # save the registers
  sd ra, 1*8(sp)
  sd sp, 2*8(sp)
  sd gp, 3*8(sp)
  # sd tp, 4*8(sp)
  .set n, 5
  .rept 27
    SAVE_GP %n
    .set n, n+1
  .endr

	// call the C trap handler in trap.c
  call trap_from_kernel

kernelret:
  # restore registers
  ld ra, 1*8(sp)
  ld sp, 2*8(sp)
  ld gp, 3*8(sp)
  # not this, in case we moved CPUs: ld tp, 24(sp)
  .set n, 5
  .rept 27
    LOAD_GP %n
    .set n, n+1
  .endr

  addi sp, sp, 256

  # return to whatever we were doing in the kernel
  sret
