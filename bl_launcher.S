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
@ LaunchUserAppNoNVIC(uint32_t *addr) 
@	R0: Starting address of the user app in memory.
@ Loads up the stack pointer and the initial PC from memory
@ and starts things off.
@ Returns
@	No return 
.syntax unified
.section .text 

@*************************************
.global LaunchUserAppNoNVIC
.thumb
.thumb_func

LaunchUserAppNoNVIC:
	cpsid i 
	ldr r1, [ r0, #0 ] /* Thats the stack pointer */
	mov sp, r1
	ldr r2, [ r0, #4 ] /* The initial PC */
	cpsie i
	bx  r2

@*************************************************************
@ LaunchUserAppNoSPNoNVIC(uint32_t *addr) 
@	R0: Starting address of the user app in memory.
@ Loads up the initial PC from memory and starts things off.
@ This could be replaced with a bit of inline assembly, but 
@ its nicer to have a compatible thing.
@ but 
@ Returns
@	No return 
.syntax unified
.section .text 

@*************************************
.global LaunchUserAppNoSPNoNVIC
.thumb
.thumb_func

LaunchUserAppNoSPNoNVIC:
	ldr r2, [ r0, #4 ] /* The initial PC */
	bx  r2

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

