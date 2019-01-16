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

#define FULLSIZE (RINGSIZE-1)


const uint8_t randchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
uint8_t bufcontents[RINGSIZE];
uint8_t* source;
uint8_t* dest;
RINGBUF ring;

// A Dump that assumes printable data.
void dump_ring() {
  for ( int i; i<RINGSIZE; i++) printf("%c",bufcontents[i]);
  printf(" ");
}

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
	CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE );
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
		CU_ASSERT( ringbuffer_addchar(&ring, cin) == FULLSIZE-1 )
    CU_ASSERT( ring.iWrite > ring.iRead )

		CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE-1);

		// Nothing should be getting Dropped
		CU_ASSERT( ring.Dropped == 0 );
		CU_ASSERT(ringbuffer_used(&ring) == 1);

		cout = ringbuffer_getchar(&ring);

		CU_ASSERT(ring.iWrite == ring.iRead );
		CU_ASSERT(ringbuffer_used(&ring) == 0);
		CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE);

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

  int dropped_before = ring.Dropped;

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
  CU_ASSERT(count == FULLSIZE);
  // Make sure that it got recorded as dropped.
  CU_ASSERT(ring.Dropped = (dropped_before+1) );

  // Clean up.
  drain();
  CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE);
	}


// Add a random number of characters, and then
// pull them all out.   Make sure that things match.

void doPush(int count) {

  CU_ASSERT( ringbuffer_free(&ring) == FULLSIZE);

  for ( int i = 0; i < count; i++) {
    ringbuffer_addchar(&ring,randchars[i]);
    }

  // printf("Pre = %d, Post=%d\n", pre_free,ringbuffer_free(&ring));
  CU_ASSERT( ringbuffer_free(&ring) == (FULLSIZE - count));
  CU_ASSERT( ringbuffer_used(&ring) == count);

  // Now pull them out and make sure that they match.
  for ( int i = 0; i < count; i++) {
    int cout = ringbuffer_getchar(&ring);
    CU_ASSERT( cout = randchars[i]);
    }
  }

void doPushBulkRemove(int count) {

  CU_ASSERT( ringbuffer_free(&ring) == FULLSIZE);

  for ( int i = 0; i < count; i++) {
    ringbuffer_addchar(&ring,randchars[i]);
    }

  CU_ASSERT( ringbuffer_free(&ring) == (FULLSIZE - count));
  CU_ASSERT( ringbuffer_used(&ring) == count);

  uint8_t const *checkchars = randchars;

  // This will take up to two passes.
  for (int i=0; i < 2; i++ ) {
    int before   = ringbuffer_used(&ring);
    int howmuch = ringbuffer_getbulkcount(&ring);

    if ( howmuch == 0 ) break;

    printf("bulkc=%02d ",howmuch);
    dump_ring();

    CU_ASSERT( howmuch <= FULLSIZE );
    uint8_t *start = ringbuffer_getbulkpointer(&ring);

    // It had better match!
    int ret = memcmp(start,checkchars,howmuch);
    checkchars+= howmuch; // Sliding window...
    memset(start, '*', howmuch); // Scrub for debug.
    CU_ASSERT( ret == 0);
    if ( ret != 0 ) {
        printf("Compare Fail!");
        return;
        }
    ringbuffer_bulkremove(&ring,howmuch);
    CU_ASSERT( ringbuffer_used(&ring) == ( before - howmuch ) );
    }

  CU_ASSERT( ring.iWrite == ring.iRead );
  }

// Find a random length of the right size > 0 && <= max
static int next_rand(int max) {
  int val;
  do {
    val = random() & (RINGSIZE-1);
  } while ( (val == 0 ) || ( val > max ) );
  return(val);
}

// Push a random number of characters.
void testRandAdd() {
  printf("Chars ");
  srandom(0); // Start with a known seed value.
  drain(); // Known state
  // Fill it with known data.
  for (int i=0; i<RINGSIZE;i++) bufcontents[i] = '_';

  // printf("before: iWrite=%02d, iRead=%02d ",ring.iWrite,ring.iRead);

  for (int i=0; i < 50; i++) {
    int max = ringbuffer_free(&ring);
    int size = next_rand(max);
    // printf("%02d/%02d ",size,max);
    // dump_ring();

    CU_ASSERT( max > 0 );

    doPush(size);
    CU_ASSERT( ring.iWrite == ring.iRead );
    }

  printf("Bulk ");
  printf("\n  before: iWrite=%02d, iRead=%02d\n",ring.iWrite,ring.iRead);

  // Do it again, with the bulk remove operator.
  srandom(0); // Start with a known seed value.
  drain(); // Known state
  CU_ASSERT( ringbuffer_used(&ring) == 0 );
  for (int i=0; i<RINGSIZE;i++) bufcontents[i] = '_';

  for (int i=0; i < 50; i++) {
    int max = ringbuffer_free(&ring);
    int size = next_rand(max);
    printf("%03d %02d/%02d ",i,size,max);
    dump_ring();

    CU_ASSERT( max > 0 );

    doPushBulkRemove(size);
    CU_ASSERT( ring.iWrite == ring.iRead );
    printf("\n");
    }

  }





/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main() {

	int rcode;
	int i;
	CU_pSuite pSuite = NULL;

	srandom(0);

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
        	(NULL == CU_add_test(pSuite, "Overflow test", testOverflow)) ||
          (NULL == CU_add_test(pSuite, "Random Length Adds", testRandAdd)) ;

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
