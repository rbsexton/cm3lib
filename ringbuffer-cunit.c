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
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <time.h>

#include "ringbuffer.h"

#include "CUnit/Basic.h"


#define BUFFERSIZE 16384
#define RINGSIZE 16

#define FULLSIZE (RINGSIZE)


// This is twice as long so we can wrap off the end.
const uint8_t patternchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
#define PATTERNLEN 32

uint8_t bufcontents[RINGSIZE];
uint8_t* source;
uint8_t* dest;
RINGBUF ring;

// --------------------------------------------------
// Utility Functions
// --------------------------------------------------

// A Dump that assumes printable data.
void dump_ring() {
    for ( int i = 0; i < RINGSIZE; i++) printf("%c", bufcontents[i]);

    printf(" ");
    }

uint8_t* makerandbuffer() {
    uint8_t* buf;
    int i;

    buf = malloc(BUFFERSIZE * sizeof(uint8_t));

    for (i = 0; i < BUFFERSIZE; i++) {
        buf[i] = patternchars[random() & 0x1f];
        }

    return(buf);
    }

// Drain a buffer and tell us how many operations it took.
int drain() {
    int count = 0;
    int ret;

    do {
        count++;
        ret = ringbuffer_getchar(&ring);
        }
    while ( count < 2 * RINGSIZE && ret != -1 );

    return(count);
    }

// Add a random number of characters, and then
// pull them all out.   Make sure that things match.
void doPush(int count) {

    CU_ASSERT( ringbuffer_free(&ring) == FULLSIZE);

    for ( int i = 0; i < count; i++) {
        ringbuffer_addchar(&ring, patternchars[i]);
        }

    // printf("Pre = %d, Post=%d\n", pre_free,ringbuffer_free(&ring));
    CU_ASSERT( ringbuffer_free(&ring) == (FULLSIZE - count));
    CU_ASSERT( ringbuffer_used(&ring) == count);

    // Now pull them out and make sure that they match.
    for ( int i = 0; i < count; i++) {
        int c_out = ringbuffer_getchar(&ring);
        CU_ASSERT( c_out = patternchars[i]);
        }
    }

// Add a random number of characters, and then
// pull them all out with bulk remove functions.
// Make sure that things match.
void doPushBulkRemove(int count) {

    CU_ASSERT( ringbuffer_free(&ring) == FULLSIZE);

    for ( int i = 0; i < count; i++) {
        ringbuffer_addchar(&ring, patternchars[i]);
        }

    CU_ASSERT( ringbuffer_free(&ring) == (FULLSIZE - count));
    CU_ASSERT( ringbuffer_used(&ring) == count);

    uint8_t const *checkchars = patternchars;

    // This will take up to two passes.
    for (int i = 0; i < 2; i++ ) {
        int before   = ringbuffer_used(&ring);
        int howmuch = ringbuffer_getbulkcount(&ring);

        if ( howmuch == 0 ) break;

        printf("bulkc=%02d ", howmuch);
        dump_ring();

        CU_ASSERT( howmuch <= FULLSIZE );
        uint8_t *start = ringbuffer_getbulkpointer(&ring);

        // It had better match!
        int ret = memcmp(start, checkchars, howmuch);
        checkchars += howmuch; // Sliding window...
        memset(start, '*', howmuch); // Scrub for debug.
        CU_ASSERT( ret == 0);

        if ( ret != 0 ) {
            printf("Compare Fail!");
            return;
            }

        ringbuffer_bulkremove(&ring, howmuch);
        CU_ASSERT( ringbuffer_used(&ring) == ( before - howmuch ) );
        }

    CU_ASSERT( ring.iWrite == ring.iRead );
    CU_ASSERT( ring.Dropped == 0);
    }

// Find a random length of the right size > 0 && <= max
static int next_rand(int max) {
    int val;

    do {
        val = random() & (RINGSIZE - 1);
        }
    while ( (val == 0 ) || ( val > max ) );

    return(val);
    }

// ---------------------------------------------------
// ---------------------------------------------------
/* The suite initialization function.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1(void) {
    source = makerandbuffer();
    dest = malloc(BUFFERSIZE * sizeof(uint8_t));
    ringbuffer_init(&ring, bufcontents, RINGSIZE);

    return(0);
    }

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void) {
    return 0;
    }

// ------------------------------------------------------
// ------------------------------------------------------
// Tests
// ------------------------------------------------------
// ------------------------------------------------------

// ----------------------------------------
// ----------------------------------------
void testNEW(void) {
    CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE );
    CU_ASSERT(ringbuffer_used(&ring) == 0);
    CU_ASSERT(ring.iWrite == 0 );
    CU_ASSERT(ring.iRead == 0 );
    CU_ASSERT( ring.Dropped == 0 );
    }

// ----------------------------------------
// ----------------------------------------
// Push a lot of data into the ring buffer, and see of it comes
// back out.   We should start with an empty buffer
void testSINGLES(void) {

    int i;
    uint8_t cin, cout;


    // Push a lot of bytes through.  Check lots of stuff
    for (i = 0; i < 1000; i++) {
        cin = source[i];

        CU_ASSERT(ring.iWrite == ring.iRead );

        // Make sure we return the right thing.
        CU_ASSERT( ringbuffer_addchar(&ring, cin) == FULLSIZE - 1 )
        CU_ASSERT( ring.iWrite > ring.iRead )

        CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE - 1);

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

// ----------------------------------------
// ----------------------------------------
void testUnderFlow(void) {

    int i;
    uint8_t cin;
    int cout;

    // Push a lot of bytes through.  Check lots of stuff
    for (i = 0; i < 10; i++) {
        cin = source[i];
        ringbuffer_addchar(&ring, cin);
        }

    CU_ASSERT( ring.iWrite > ring.iRead )

    for (i = 0; i < 10; i++) {
        cout = ringbuffer_getchar(&ring);
        CU_ASSERT(cout == source[i]);
        }

    CU_ASSERT(ring.iWrite == ring.iRead );


    for (i = 0; i < 10; i++) {
        cout = ringbuffer_getchar(&ring);
        CU_ASSERT(cout == -1);
        }

    }

// ----------------------------------------
// Overflow test.    This one is sensitive
// to the nature of the buffer.
// We should be able to add RINGSIZE + characters
// and see exactly one get dropped.
// ----------------------------------------
void testOverflow() {

    int count;
    int ret = 0;

    int dropped_before = ring.Dropped;

    // Drain the buffer, make sure it takes a reasonable number of characters.
    count = drain();
    CU_ASSERT(count <= RINGSIZE);
    ret = ringbuffer_free(&ring);
    CU_ASSERT(ret == RINGSIZE);


    // Now fill it up, and verify that we can insert the correct number
    // of characters.

    for (int i = 0; i < RINGSIZE; i++ ) {
        uint8_t cin = i & 0xFF;
        ret = ringbuffer_addchar(&ring, cin);
        }

    // The return code should be -1 - its full.
    CU_ASSERT(ret == 0);

    // Add one more.
    ret = ringbuffer_addchar(&ring, 0xAA);
    CU_ASSERT(ret == -1);

    // Make sure that it got recorded as dropped.
    CU_ASSERT(ring.Dropped = (dropped_before + 1) );
    ring.Dropped = 0; // Put it back!

    // Clean up.
    drain();
    CU_ASSERT(ringbuffer_free(&ring) == FULLSIZE);
    }

// ----------------------------------------
// ----------------------------------------
// Push a random number of characters and
// pull them out.
void testRandAdd() {
    printf("Chars ");
    srandom(0); // Start with a known seed value.
    drain(); // Known state

    // Fill it with known data.
    for (int i = 0; i < RINGSIZE; i++) bufcontents[i] = '_';

    printf("before: iWrite=%02d, iRead=%02d ", ring.iWrite, ring.iRead);

    for (int i = 0; i < 50; i++) {
        int max = ringbuffer_free(&ring);
        int size = next_rand(max);
        // printf("%02d/%02d ",size,max);
        // dump_ring();

        CU_ASSERT( max > 0 );

        doPush(size);
        CU_ASSERT( ring.iWrite == ring.iRead );
        }
    }


void testRandAddBulkRemove() {

    printf("Bulk Remove");
    printf("\n  before: iWrite=%02d, iRead=%02d\n", ring.iWrite, ring.iRead);

    // Do it again, with the bulk remove operator.
    srandom(0); // Start with a known seed value.
    drain(); // Known state
    CU_ASSERT( ringbuffer_used(&ring) == 0 );

    for (int i = 0; i < RINGSIZE; i++) bufcontents[i] = '_';

    for (int i = 0; i < 50; i++) {
        int max = ringbuffer_free(&ring);
        int size = next_rand(max);
        printf("%03d %02d/%02d ", i, size, max);
        dump_ring();

        CU_ASSERT( max > 0 );

        doPushBulkRemove(size);
        CU_ASSERT( ring.iWrite == ring.iRead );
        printf("\n");
        }

    fflush(stdout);
    }

// ----------------------------------------
// ----------------------------------------

// Heres the full cycle.
// Pre-load a random number of characters
// Init the counters.
// Repeat N times:
//  fill the buffer all the way up.
//  take a maximal bite and check the results.

static void PrConOp(int offset, int loops, int drainmax) {
    // Pre-load, and mark empty.
    ring.iRead += offset;
    ring.iWrite = ring.iRead;
    printf("Pre-load %d ", offset);
    dump_ring();
    printf("\n");

    // Keep an index so that we can do our checks.
    int wr_index = 0;
    int rd_index = 0;

    for ( int i = 0; i < loops; i++ ) {
        // Fill it up.
        int count = 0;

        while ( ringbuffer_free(&ring) ) {
            ringbuffer_addchar(&ring, patternchars[wr_index & (PATTERNLEN - 1)]);
            wr_index++;
            count++;
            }

        printf("Pre-filled %d ", count);

        dump_ring();

        // Now for the main loop that tests the aggregation.
        // if we alternate between filling and bulk removal, it should
        // result in full - sized operations.

        int drainpasses = drainmax;

        while (drainpasses && ringbuffer_getbulkcount(&ring) ) {
            uint8_t *start = ringbuffer_getbulkpointer(&ring);
            int howmuch = ringbuffer_getbulkcount(&ring);

            const uint8_t *ref = &patternchars[rd_index & (PATTERNLEN - 1)];
            int ret = memcmp(start, ref, howmuch);
            CU_ASSERT(ret == 0);

            if ( ret != 0 )
                return;

            ringbuffer_bulkremove(&ring, howmuch);
            rd_index += howmuch;
            printf(" Removed %d", howmuch);
            drainpasses--;
            }

        // This test only works for full drain scenarios.
        if ( drainmax > 1 ) {
            CU_ASSERT(rd_index == wr_index);
            }

        printf("\n");
        }
    }

// --------------------------------------------------
// Producer/Consumer Test.
// Add a random number of characters to the RB,
// then fill it all the way up.
// Drain it using bulk operations.
// Use two indices into the pattern space to verify that
// the data is being handled correctly.

void testProducerConsumer1() {
    printf("Produce/Consume Fill/Drain\n");
    srandom(0); // Start with a known seed value.

    // Random # of characters between 0-4
    for ( int i = 0; i < 5; i++ ) {
        int preload = rand() & 0x3;
        PrConOp(preload, 16, 5);
        }
    }

void testProducerConsumer2() {
    printf("Produce/Consume Fill/Partial Drain\n");
    srandom(0); // Start with a known seed value.

    // Random # of characters between 0-4
    for ( int i = 0; i < 5; i++ ) {
        int preload = rand() & 0x3;
        PrConOp(preload, 4, 1);
        }
    }



/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main() {

    int rcode;
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
            (NULL == CU_add_test(pSuite, "Random Length Adds", testRandAdd)) ||
            (NULL == CU_add_test(pSuite, "Random Length Adds", testRandAddBulkRemove)) ||
            (NULL == CU_add_test(pSuite, "Producer/Consumer test 1", testProducerConsumer1)) ||
            (NULL == CU_add_test(pSuite, "Producer/Consumer test 2", testProducerConsumer2))
            ;

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
