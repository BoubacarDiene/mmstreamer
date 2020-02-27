#include "unity.h"
#include "exception_test_helpers.h"
#include "utils/List.h"

void setUp(void) {}

void tearDown(void) {}

/**
 Requirement:
 - List_Init() must "assert" when at least one of its input parameters is NULL
 */
void test_List_Init_Null_Parameter(void)
{
    struct list_s *obj                = NULL;
    struct list_callbacks_s callbacks = {NULL, NULL, NULL};

    TEST_ASSERT_EXPECTED(List_Init(&obj, NULL));
    TEST_ASSERT_EXPECTED(List_Init(NULL, &callbacks));
}

/**
 Requirement:
 - List_UnInit() must "assert" when its input parameter is NULL
 */
void test_List_UnInit_Null_Parameter(void)
{
    TEST_ASSERT_EXPECTED(List_UnInit(NULL));
}

/**
 Requirement:
 - List_UnInit() must "crash" when its input parameter has not been obtained
   using List_Init()
 */
void test_List_UnInit_Bad_Memory_Access(void)
{
    struct list_s _obj = {0};
    struct list_s *obj = &_obj;

    TEST_BAD_MEMORY_ACCESS_EXPECTED(List_UnInit(&obj));
}

/**
 Requirement:
 - List_Init() must initialize "obj" without error when called as expected (valid
   address of unitialized "obj")
 - List_UnInit() must release resources allocated by List_Init() without error
 */
void test_List_Init_UnInit_Valid_Input_Parameters(void)
{
    struct list_s *obj                = NULL;
    struct list_callbacks_s callbacks = {0};
    enum list_error_e ret             = LIST_ERROR_NONE;

    ret = List_Init(&obj, &callbacks);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_NOT_NULL(obj);

    ret = List_UnInit(&obj);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_NULL(obj);
}
