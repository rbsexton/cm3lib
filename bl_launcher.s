@*************************************************************
@ Load up the designated user app and give it control of the
@ system.  The user app is a Cortex-M executable that will
@ take complete ownership of the system after the startup code
@ does the hardware initialization.
@
@ In other contexts, this could be considered a normal part of 
@ operating a bootloader.
@ 
@*************************************************************
@ launchuserapp(uint32_t *addr) 
@	R0: Starting address of the user app in memory.
@ Returns
@	No return 
.syntax unified
.section .text 

@*************************************
.global LaunchUserAppNoNVIC
.thumb
.thumb_func

LaunchUserAppNoNVIC:
	ldr r1, [ r0, #0 ] /* Thats the stack pointer */
	mov sp, r1
	ldr r2, [ r0, #4 ] /* The initial PC */
	bx  r2
.end

@*************************************
.global LaunchUserAppUpdateNVIC
.thumb
.thumb_func

LaunchUserAppUpdateNVIC:
	ldr r1, [ r0, #0 ] /* Stack pointer */
	mov sp, r1
	ldr r2, [ r0, #4 ] /* Initial PC */
	push { r2 }
	ldr r2, =0xE000ED08 /* VTOR */
	str r0, [ r2, #0 ]
	pop { pc }
.end

