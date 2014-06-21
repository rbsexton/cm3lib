/*
 *  Simple example of a CUnit unit test.
 *
 *  This program (crudely) demonstrates a very simple "black box"
 *  test of the standard library functions fprintf() and fread().
 *  It uses suite initialization and cleanup functions to open
 *  and close a common temporary file used by the test functions.
 *  The test functions then write to and read from the temporary
 *  file in the course of testing the library functions.
 *
 *  The 2 test functions are added to a single CUnit suite, and
 *  then run using the CUnit Basic interface.  The output of the
 *  program (on CUnit version 2.0-2) is:
 *
 *           CUnit : A Unit testing framework for C.
 *           http://cunit.sourceforge.net/
 *
 *       Suite: Suite_1
 *         Test: test of fprintf() ... passed
 *         Test: test of fread() ... passed
 *
 *       --Run Summary: Type      Total     Ran  Passed  Failed
 *                      suites        1       1     n/a       0
 *                      tests         2       2       2       0
 *                      asserts       5       5       5       0
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CUnit/Basic.h"

#include <stdlib.h>
#include <stdint.h>
#include "ringbuffer.h"

#define BUFFERSIZE 16384
#define RINGSIZE 64

const uint8_t randchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
uint8_t bufcontents[RINGSIZE];
uint8_t* source;
uint8_t* dest;
RINGBUF ring;


uint8_t* makerandbuffer() {
        uint8_t* buf;
	int i;

	buf = malloc(BUFFERSIZE * sizeof(uint8_t));

	for (i = 0; i < BUFFERSIZE; i++) {
		buf[i] = randchars[random() & 0x1f];
		}
	return(buf);
	}


/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void) {
	source = makerandbuffer();
	dest = malloc(BUFFERSIZE * sizeof(uint8_t));
	ringbuffer_init(&ring,bufcontents,RINGSIZE);

	return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void)
{
      return 0;
}

/* Start adding tests. */
void testNEW(void) {
	CU_ASSERT(ringbuffer_free(&ring) == (RINGSIZE-1) );
	CU_ASSERT(ringbuffer_used(&ring) == 0);
	CU_ASSERT(ring.iWrite == 0 );
	CU_ASSERT(ring.iRead == 0 );
	CU_ASSERT( ring.Dropped == 0 );
}

// Push a lot of data into the ring buffer, and see of it comes
// back out.   We should start with an empty buffer
void testSINGLES(void) {

	int i;
	uint8_t cin, cout;


	// Push a lot of bytes through.  Check lots of stuff
	for (i=0; i < 1000; i++) {
		cin = source[i];

		CU_ASSERT(ring.iWrite == ring.iRead );

		// Make sure we return the right thing.
		CU_ASSERT( ringbuffer_addchar(&ring, cin) == RINGSIZE-2 )
		CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-2);

		// Nothing should be getting Dropped
		CU_ASSERT( ring.Dropped == 0 );
		CU_ASSERT(ringbuffer_used(&ring) == 1);

		cout = ringbuffer_getchar(&ring);
		
		CU_ASSERT(ring.iWrite == ring.iRead );
		CU_ASSERT(ringbuffer_used(&ring) == 0);
		CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-1);
	
		CU_ASSERT(cin == cout);
		}

	}


void testUNDERFLOW(void) {

	int i;
	uint8_t cin, cout;

	// Push a lot of bytes through.  Check lots of stuff
	for (i=0; i < 10; i++) {
		cin = source[i];
		ringbuffer_addchar(&ring, cin);
		}

	for (i=0; i < 10; i++) {
		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(cout == source[i]);
		}

	for (i=0; i < 10; i++) {
		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(cout == 0);
		}

	}


void testPREPUSHkern(int divide) {
	int i;
	uint8_t cin, cout;

	CU_ASSERT(ringbuffer_used(&ring) == 0);
	CU_ASSERT(ring.iRead == ring.iWrite);

	// Pre-Push 10 chars
	for (i=0; i < divide; i++) {
		cin = source[i];

		CU_ASSERT(ringbuffer_used(&ring) == i);
		CU_ASSERT( ringbuffer_addchar(&ring, cin) == ( (RINGSIZE - 2) - i) );
		CU_ASSERT(ringbuffer_used(&ring) == i+1);

		// CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		// CU_ASSERT(ring.iRead >= 0 && ring.iRead < RINGSIZE);
		// CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(i+1));
		}

	for (i=divide; i < 1123; i++) {
		cin = source[i];

		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(ringbuffer_used(&ring) == (RINGSIZE));
		
		
		//CU_ASSERT(ring.iRead >= 0 && ring.iRead < RINGSIZE);
		//CU_ASSERT(ringbuffer_used(&ring) == (divide-1));
		//CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(divide-1));
		// CU_ASSERT(cout == source[i-divide]);

		ringbuffer_addchar(&ring, cin);
		
		// CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		// CU_ASSERT(ring.iRead >= 0 && ring.iRead < RINGSIZE);
		// CU_ASSERT(ringbuffer_used(&ring) == divide);
		// CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(divide));

		}

	// Drain it.
	for (i=divide; i > 0 ; i--) {
		// CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		// CU_ASSERT(ring.iRead >= 0 && ring.iRead < RINGSIZE);
		// CU_ASSERT(ringbuffer_used(&ring) == i);
		// CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(i));
		cout = ringbuffer_getchar(&ring);
		// CU_ASSERT(ringbuffer_used(&ring) == i-1);
		// CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(i-1));
		}
	}


void testPREPUSH() {
	int i;
	for (i = 1; i < RINGSIZE; i++) {
		testPREPUSHkern(i);
		}
	}


void testWRAP() {

	int i;
	uint8_t cin, cout;
	
	printf("\n");
	// Fill it up and force a wrap.  For a 32-byte buffer,
	// over-write should start at when you write the 32nd byte.
	for (i=0; i < RINGSIZE-1; i++) {
		cin = source[i];
		CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		CU_ASSERT(ring.iRead  >= 0 && ring.iRead < RINGSIZE);
		CU_ASSERT(ringbuffer_used(&ring) == i );
		CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(i) );

		ringbuffer_addchar(&ring, cin);
		putchar(cin);

		CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		CU_ASSERT(ring.iRead  >= 0 && ring.iRead < RINGSIZE);
		CU_ASSERT(ringbuffer_used(&ring) == i+1);
		CU_ASSERT(ringbuffer_free(&ring) == RINGSIZE-(i+1));
		}

	for (i=RINGSIZE-1; i < RINGSIZE*2; i++) {
		cin = source[i];
		CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		CU_ASSERT(ring.iRead  >= 0 && ring.iRead < RINGSIZE);
		CU_ASSERT(ringbuffer_used(&ring) == RINGSIZE-1 );
		CU_ASSERT(ringbuffer_free(&ring) == 1 );

		ringbuffer_addchar(&ring, cin);
		putchar(cin);
		}

	printf("\n");
	// Drain it and check as we go.
	// Wrap starts happening at RINGSIZE-1
	for (i=0; i < RINGSIZE-1; i++) {
		CU_ASSERT(ring.iWrite >= 0 && ring.iWrite < RINGSIZE);
		CU_ASSERT(ring.iRead  >= 0 && ring.iRead < RINGSIZE);
		CU_ASSERT(ringbuffer_used(&ring) == (RINGSIZE-1)-i);
		CU_ASSERT(ringbuffer_free(&ring) == i+1);

		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(cout == source[i+RINGSIZE+1]);

		putchar(cout);

		}
	printf("\n");
	}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main() {
	
	int rcode;
	int i;
	CU_pSuite pSuite = NULL;

	srandom(time(0));

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */

	rcode = (NULL == CU_add_test(pSuite, "Test of fresh structure", testNEW)) ||
        	(NULL == CU_add_test(pSuite, "push/get 1000 chars", testSINGLES)) ||
        	(NULL == CU_add_test(pSuite, "underflow ", testUNDERFLOW)) ||
        	(NULL == CU_add_test(pSuite, "push/get 1000 chars with pre-push", testPREPUSH) ||
        	(NULL == CU_add_test(pSuite, "Wrap-Around test", testWRAP))
		);

   if ( rcode ) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

