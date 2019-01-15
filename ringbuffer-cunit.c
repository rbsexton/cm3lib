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

 // Update for new and improved ringbuffer tests.
 // The semantics have changed since the original design.
 // Now, for starters, wrap is not supported.   The routines will
 // not overflow data thats already in there.    The other internal
 // change is that rather than two modulo pointers, we have 32-bit
 // indices that get masked down, so that you can always compare
 // the read and write indices.

 // Secondly, there is support for bulk operations.   Those
 // are useful for aggregating small writes into network buffers.

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CUnit/Basic.h"

#include <stdlib.h>
#include <stdint.h>
#include "ringbuffer.h"

#define BUFFERSIZE 16384
#define RINGSIZE 16

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
    CU_ASSERT( ring.iWrite > ring.iRead )

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

void testUnderFlow(void) {

	int i;
	uint8_t cin;
  int cout;

	// Push a lot of bytes through.  Check lots of stuff
	for (i=0; i < 10; i++) {
		cin = source[i];
		ringbuffer_addchar(&ring, cin);
		}
  CU_ASSERT( ring.iWrite > ring.iRead )

	for (i=0; i < 10; i++) {
		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(cout == source[i]);
		}

  CU_ASSERT(ring.iWrite == ring.iRead );


	for (i=0; i < 10; i++) {
		cout = ringbuffer_getchar(&ring);
		CU_ASSERT(cout == -1);
		}

	}

// Drain a buffer and tell us how many operations it took.
int drain() {
  int count = 0;
  int ret;
  do {
    count++;
    ret = ringbuffer_getchar(&ring);
    } while ( count < 2*RINGSIZE && ret != -1 );
  return(count);
  }

void testOverflow() {

	int i, count, ret;
	uint8_t cin, cout;

  // Drain the buffer, make sure it takes a reasonable number of characters.
  count = drain();
  CU_ASSERT(count <= RINGSIZE);

  // Now fill it up, and verify that we can insert the correct number
  // of characters.

  count = 0;
  do {
    uint8_t cin = count & 0xFF;
    ret = ringbuffer_addchar(&ring, cin);
    if ( ret >= 0 ) count++;

  } while(ret && count < 2*RINGSIZE);
  CU_ASSERT(count == RINGSIZE -1);
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
        	(NULL == CU_add_test(pSuite, "underflow ", testUnderFlow)) ||
        	(NULL == CU_add_test(pSuite, "Overflow test", testOverflow)) ;

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
