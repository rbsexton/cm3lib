@*************************************************************
@ Load up the designated user app and give it control of the
@ system.  The user app is a Cortex-M executable that will
@ take complete ownership of the system after the startup code
@ does the hardware initialization.
@*************************************************************
@ launchuserapp(uint32_t *addr) 
@	R0: Starting address of the user app in memory.
@ Returns
@	No return 
.syntax unified
.section .text 

.global launchuserapp
.thumb
.thumb_func

launchuserapp:

	ldr r1, [ r0, #0 ] /* Thats the stack pointer */
	mov sp, r1
	 
	ldr r2, [ r0, #4 ] /* The initial PC */
	push { r2 }
	
	ldr r2, =0xE000ED08 /* VTOR */
	str r0, [ r2, #0 ]
	
	pop { pc }
		
.end
	
