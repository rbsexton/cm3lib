@*************************************************************
@ Load up the designated user app and give it control of the
@ system.  The user app is a Cortex-M executable that will
@ take complete ownership of the system after the startup code
@ does the hardware initialization.
@
@ In other contexts, this could be considered a normal part of 
@ operating a bootloader.
@
@ These routines have to be used with care. 

@*************************************************************
@ LaunchUserApp(uint32_t *appaddr, uint32_t *runtimep) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@ Loads up the stack pointer and the initial PC from memory
@ and starts things off.   Passes in the runtime pointer.
@	No return 
.syntax unified
.section .text 

@*************************************
.global LaunchUserApp
.thumb
.thumb_func

LaunchUserApp:
	cpsid i 
	ldr r2, [ r0, #0 ] /* Thats the stack pointer */
	mov sp, r2
	ldr r2, [ r0, #4 ] /* The initial PC */
        mov r0, r1         /* Put the RT Link in the right spot */
	cpsie i
	bx  r2

@*************************************************************
@ LaunchUserAppThread(uint32_t *appaddr, uint32_t *runtimep) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@ Loads up the stack pointer and the initial PC from memory
@ and starts things off.   Passes in the runtime pointer.
@	No return 

@*************************************
.global LaunchUserAppThread
LaunchUserAppThread:
	cpsid i 
	ldr r2, [ r0, #0 ] /* Thats the stack pointer */
	msr psp, r2
	movs r3, # 2
	msr control, r3
	isb					// Per the Arm Docs.

	ldr r2, [ r0, #4 ] /* The initial PC */
	mov r0, r1         /* Put the RT Link in the right spot */
	cpsie i
	bx  r2

@*************************************************************
@ LaunchUserAppNoSP(uint32_t *appaddr, uint32_t *runtimep) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@ Loads up the initial PC from memory and starts things off.
@ Don't alter the stack pointer.
@	No return 
.syntax unified
.section .text 

@*************************************
.global LaunchUserAppNoSP
.thumb
.thumb_func

LaunchUserAppNoSP:
        ldr r2, [ r0, #4 ] /* The initial PC */
        mov r0, r1  
	bx  r2

@*************************************************************
@ LaunchUserAppUpdateNVIC(uint32_t *appaddr, uint32_t *runtimep) 
@	R0: Starting address of the user app in memory.
@	R1: Pointer to runtime data to share with forth.
@ Loads up the initial PC from memory and starts things off.
@ Adjust the VTOR register in the NVIC so that the app
@ can use the interrupt vectors.
@ Doesnt return


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

