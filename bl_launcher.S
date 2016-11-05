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
@ LaunchUserAppNoNVIC(uint32_t *addr, uint32_t *runtimep) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
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
	ldr r2, [ r0, #0 ] /* Thats the stack pointer */
	mov sp, r2
	ldr r2, [ r0, #4 ] /* The initial PC */
        mov r0, r1         /* Put the RT Link in the right spot */
	cpsie i
	bx  r2

@*************************************************************
@ LaunchUserAppNoSPNoNVIC(uint32_t *addr) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
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
        mov r0, r1  
	bx  r2

@*************************************
.global LaunchUserAppUpdateNVIC
.thumb
.thumb_func

LaunchUserAppUpdateNVIC:
	cpsid i 
	ldr r2, [ r0, #0 ] /* Stack pointer */
        mov sp, r2

	ldr r2, =0xE000ED08 /* VTOR */
	str r0, [ r2, #0 ]
                 
        ldr r2, [ r0, #4 ] /* Initial PC */
        mov r0, r1 /* Runtime link */                  

        cpsie i 
        bx r2
 .end

