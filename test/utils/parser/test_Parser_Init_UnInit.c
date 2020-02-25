#include "unity.h"
#include "exception_test_helpers.h"
#include "Parser.h"

void setUp(void) {}

void tearDown(void) {}

/**
 * Requirement:
 * - Parser_Init() must "assert" when its input parameter is NULL
 */
void test_Parser_Init_Invalid_Parameter(void)
{
    TEST_ASSERT_EXPECTED(Parser_Init(NULL));
}

/**
 * Requirement:
 * - Parser_UnInit() must "assert" when its input parameter is NULL
 */
void test_Parser_UnInit_Invalid_Parameter(void)
{
    struct parser_s *obj = NULL;

    TEST_ASSERT_EXPECTED(Parser_UnInit(NULL));
    TEST_ASSERT_EXPECTED(Parser_UnInit(&obj));
}

/*
 * Requirement:
 * - Parser_UnInit() must "crash" when its input parameter has not been obtained
 *   using Parser_Init()
 */
void test_Parser_UnInit_Bad_Memory_Access(void)
{
    struct parser_s _obj = {0};
    struct parser_s *obj = &_obj;

    TEST_BAD_MEMORY_ACCESS_EXPECTED(Parser_UnInit(&obj));
}

/*
 * Requirement:
 * - Parser_Init() must initialize "obj" without error when called as expected (valid
 *   address of uninitialized "obj"
 * - Parser_UnInit() must release resources allocated by Parser_Init() without error
 */
void test_Parser_Init_UnInit_Valid_Input_Parameters(void)
{
    struct parser_s *obj    = NULL;
    enum parser_error_e ret = PARSER_ERROR_NONE;

    ret = Parser_Init(&obj);
    TEST_ASSERT_EQUAL(ret, PARSER_ERROR_NONE);
    TEST_ASSERT_NOT_NULL(obj);

    ret = Parser_UnInit(&obj);
    TEST_ASSERT_EQUAL(ret, PARSER_ERROR_NONE);
    TEST_ASSERT_NULL(obj);
}
