#include "unity.h"
#include "exception_test_helpers.h"
#include "utils/List.h"

static struct list_s *obj          = NULL;
static struct list_params_s params = {0};

void setUp(void)
{
    (void)List_Init(&obj, &params);
}

void tearDown(void)
{
    (void)List_UnInit(&obj);
}

/* -------------------------------------------------------------------------------------------- */
/*                                              LOCK                                            */
/* -------------------------------------------------------------------------------------------- */

/**
 Requirement:
 - lock() must "assert" when its input parameter is NULL
 */
void test_List_Lock_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(obj->lock(NULL));
}

/**
 Requirement:
 - lock() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Lock_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->lock(&fakeObj));
}

/**
 Requirement:
 - lock() must return no error when "obj" has been obtained using List_Init()
 */
void test_List_Lock_Valid_Input_Parameter(void)
{
    TEST_ASSERT_EQUAL(obj->lock(obj), LIST_ERROR_NONE);
}

/* -------------------------------------------------------------------------------------------- */
/*                                             UNLOCK                                           */
/* -------------------------------------------------------------------------------------------- */

/**
 Requirement:
 - unlock() must "assert" when its input parameter is NULL
 */
void test_List_Unlock_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(obj->unlock(NULL));
}

/**
 Requirement:
 - unlock() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Unlock_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->unlock(&fakeObj));
}

/**
 Requirement:
 - unlock() must return no error when "obj" has been obtained using List_Init()
 */
void test_List_Unlock_Valid_Input_Parameter(void)
{
    TEST_ASSERT_EQUAL(obj->unlock(obj), LIST_ERROR_NONE);
}

/**
 Requirement:
 - unlock() must return no error when "obj" has been obtained using List_Init()
 */
void test_List_Unlock_After_Lock(void)
{
    TEST_ASSERT_EQUAL(obj->unlock(obj), LIST_ERROR_NONE);
}
