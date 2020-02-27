#include "unity.h"
#include "exception_test_helpers.h"
#include "utils/List.h"

static struct list_s *obj                = NULL;
static struct list_callbacks_s callbacks = {NULL, NULL, NULL};
static uint32_t elements[]               = {1, 2, 3};

static uint32_t nbCallsToCompareCb = 0;
static uint32_t nbCallsToReleaseCb = 0;

static uint8_t compareCb(struct list_s *obj, void *elementToCheck, const void *userData)
{
    (void)obj;

    uint32_t elementToRemove = *((uint32_t*)userData);
    nbCallsToCompareCb++;
    return *((uint32_t*)elementToCheck) == elementToRemove;
}

static void releaseCb(struct list_s *obj, void *element)
{
    (void)obj;
    (void)element;

    nbCallsToReleaseCb++;
}

void setUp(void)
{
    nbCallsToCompareCb = 0;
    nbCallsToReleaseCb = 0;

    callbacks.compareCb = compareCb;
    callbacks.releaseCb = releaseCb;
    (void)List_Init(&obj, &callbacks);
}

void tearDown(void)
{
    (void)obj->removeAll(obj);
    (void)List_UnInit(&obj);
}

/* -------------------------------------------------------------------------------------------- */
/*                                        ADD ELEMENTS                                          */
/* -------------------------------------------------------------------------------------------- */

/**
 * Requirement:
 * - add() must "assert" when at least one of its input parameters is NULL
 */
void test_List_Add_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(obj->add(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->add(NULL, &elements[0]));
}

/**
 * Requirement:
 * - add() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Add_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->add(&fakeObj, &elements[0]));
}

/**
 * Requirement:
 * - add() must add "element" without error when called as expected (valid "obj"
 *   and element not NULL)
 */
void test_List_Add_Valid_Input_Parameters(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    for (uint32_t i = 0; i < SIZEOF_ARRAY(elements); ++i) {
        ret = obj->add(obj, &elements[i]);
        TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    }
}

/* -------------------------------------------------------------------------------------------- */
/*                                      REMOVE ELEMENTS                                         */
/* -------------------------------------------------------------------------------------------- */

/**
 * Requirement:
 * - remove() must "assert" when at least one of its input parameters is NULL
 */
void test_List_Remove_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(obj->remove(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->remove(NULL, &elements[0]));
}

/**
 * Requirement:
 * - remove() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Remove_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->remove(&fakeObj, &elements[0]));
}

/**
 * Requirement:
 * - remove() must remove "element" without error if it exists in list
 * - compareCb() must be called at least once when one element has been removed
 * - releaseCb() must be called only once when one element has been removed
 */
void test_List_Remove_Existing_Element(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->remove(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToCompareCb);
    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - remove() must return an error when removing a non existent element
 * - compareCb() must be called at least once when the list is not empty
 * - releaseCb() must not be called since the element does not exist
 */
void test_List_Remove_NonExistent_Element_From_Non_Empty_List(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->remove(obj, &elements[1]);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToCompareCb);
    TEST_ASSERT_EQUAL_UINT32(0, nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - remove() must return an error when removing element from an empty list
 * - compareCb() must not be called since there is no element to compare
 * - releaseCb() must not be called since there is no element to release
 */
void test_List_Remove_Element_From_Empty_List(void)
{
    enum list_error_e ret = obj->remove(obj, &elements[0]);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);

    TEST_ASSERT_EQUAL_UINT32(0, nbCallsToCompareCb);
    TEST_ASSERT_EQUAL_UINT32(0, nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - remove() must return an error when removing an existing element and
 *   no compare callback has been provided
 * - It must be possible to set remove compareCb using updateCallbacks()
 * - releaseCb() must not be called since no compare callback exists
 */
void test_List_Remove_Element_No_Compare_Callback_Provided(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    callbacks.compareCb = NULL;
    ret = obj->updateCallbacks(obj, &callbacks);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->remove(obj, &elements[0]);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(0, nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - remove() must return no error when removing an existing element and
 *   no release callback has been provided
 * - It must be possible to set remove releaseCb using updateCallbacks()
 * - CompareCb() must return TRUE since the element exists
 */
void test_List_Remove_Element_No_Release_Callback_Provided(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    callbacks.releaseCb = NULL;
    ret = obj->updateCallbacks(obj, &callbacks);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->remove(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToCompareCb);
}

/* -------------------------------------------------------------------------------------------- */
/*                                     REMOVE ALL ELEMENTS                                      */
/* -------------------------------------------------------------------------------------------- */

/**
 * Requirement:
 * - removeAll() must "assert" when its input parameter is NULL
 */
void test_List_Remove_All_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(obj->removeAll(NULL));
}

/**
 * Requirement:
 * - removeAll() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Remove_All_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->removeAll(&fakeObj));
}

/**
 * Requirement:
 * - removeAll() must remove all elements without error when the list is not empty
 * - releaseCb() must be called "nbElements" times
 */
void test_List_Remove_All_Existing_Elements(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    for (uint32_t i = 0; i < SIZEOF_ARRAY(elements); ++i) {
        ret = obj->add(obj, &elements[i]);
        TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    }

    ret = obj->removeAll(obj);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(SIZEOF_ARRAY(elements), nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - removeAll() must return an error when called while the list is empty
 * - releaseCb() must not be called since there is no element to remove
 */
void test_List_Remove_All_Elements_From_Empty_List(void)
{
    enum list_error_e ret = obj->removeAll(obj);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);

    TEST_ASSERT_EQUAL_UINT32(0, nbCallsToReleaseCb);
}

/**
 * Requirement:
 * - removeAll() must return no error when the list is not empty
 */
void test_List_Remove_All_Element_No_Release_Callback_Provided(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    callbacks.releaseCb = NULL;
    ret = obj->updateCallbacks(obj, &callbacks);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->removeAll(obj);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
}
