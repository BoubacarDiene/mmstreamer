#include "unity.h"
#include "exception_test_helpers.h"
#include "Parser.h"

#define PATH_TO_RESOURCES "/workdir/test/utils/parser"
#define VALID_XML_FILE    PATH_TO_RESOURCES"/test-valid.xml"
#define INVALID_XML_FILE  PATH_TO_RESOURCES"/test-invalid.xml"

static struct parser_s *obj = NULL;

static uint32_t nbCallsToTestStartCb = 0;
static uint32_t nbCallsToTestEndCb   = 0;
static uint32_t nbCallsToElementCb   = 0;
static uint32_t nbCallsToErrorCb     = 0;

static const void *receivedUserData  = NULL;

// Called when <Test> tag is found
static void onTestStartCb(const void *userData, const char **attrs)
{
    (void)userData;
    (void)attrs;
    ++nbCallsToTestStartCb;
}

// Called when </Tag> tag is found
static void onTestEndCb(const void *userData)
{
    (void)userData;
    ++nbCallsToTestEndCb;
}

// Called when <Element> tag is found
static void onElementCb(const void *userData, const char **attrs)
{
    (void)userData;
    (void)attrs;
    ++nbCallsToElementCb;
}

static void onErrorCb(const void *userData, int32_t errorCode, const char *errorStr)
{
    (void)errorCode;
    (void)errorStr;

    receivedUserData = userData;
    ++nbCallsToErrorCb;
}

void setUp(void)
{
    nbCallsToTestStartCb = 0;
    nbCallsToTestEndCb   = 0;
    nbCallsToElementCb   = 0;
    nbCallsToErrorCb     = 0;
    receivedUserData     = NULL;

    (void)Parser_Init(&obj);
}

void tearDown(void)
{
    (void)Parser_UnInit(&obj);
}

/**
 * Requirement:
 * - parse() must "assert" when at least one of its input parameter is NULL
 */
void test_Parser_Parse_Invalid_Parameter(void)
{
    struct parser_params_s params = {0};

    TEST_ASSERT_EXPECTED(obj->parse(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->parse(NULL, &params));
}

/*
 * Requirement:
 * - parse() must return an error when an invalid path to the xml file
 *   to parse is provided
 */
void test_Parser_Parse_Invalid_Path(void)
{
    struct parser_params_s params = {.path = "invalid"};
    enum parser_error_e ret       = PARSER_ERROR_NONE;

    ret = obj->parse(obj, &params);
    TEST_ASSERT_NOT_EQUAL(ret, PARSER_ERROR_NONE);
}

/*
 * Requirement:
 * - parse() must not "crash" when no callback is provided
 */
void test_Parser_Parse_No_Callback_Provided(void)
{
    struct parser_tags_handler_s tagsHandlers[] = {
        { "Test",  NULL,  NULL,  NULL },
        { NULL,    NULL,  NULL,  NULL }
    };

    struct parser_params_s params = {
        .path         = VALID_XML_FILE,
        .encoding     = PARSER_ENCODING_UTF_8,
        .tagsHandlers = tagsHandlers,
        .onErrorCb    = NULL,
        .userData     = NULL
    };

    enum parser_error_e ret = obj->parse(obj, &params);
    TEST_ASSERT_EQUAL(ret, PARSER_ERROR_NONE);
}

/*
 * Requirement:
 * - parse() must call provided callbacks when related tags has been
 *   encountered
 */
void test_Parser_Parse_Only_Call_Callback_When_Tag_Is_Found(void)
{
    struct parser_tags_handler_s tagsHandlers[] = {
        { "Test",        onTestStartCb,  onTestEndCb,  NULL },
        { "Element",     onElementCb,    NULL,         NULL },
        { "Inexistent",  onTestStartCb,  NULL,         NULL },
        { NULL,          NULL,           NULL,         NULL }
    };

    struct parser_params_s params = {
        .path         = VALID_XML_FILE,
        .encoding     = PARSER_ENCODING_UTF_8,
        .tagsHandlers = tagsHandlers,
        .onErrorCb    = NULL,
        .userData     = NULL
    };

    enum parser_error_e ret = obj->parse(obj, &params);
    TEST_ASSERT_EQUAL(ret, PARSER_ERROR_NONE);

    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToTestStartCb);

    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToTestEndCb);
    TEST_ASSERT_EQUAL_UINT32(2, nbCallsToElementCb);
}

/*
 * Requirement:
 * - parse() must return an error when the xml file is not well-formed
 * - Then the provided onErrorCb must be called
 */
void test_Parser_Parse_Xml_Not_Well_Formed(void)
{
    struct parser_tags_handler_s tagsHandlers[] = {
        { NULL,  NULL,  NULL,  NULL }
    };

    struct parser_params_s params = {
        .path         = INVALID_XML_FILE,
        .encoding     = PARSER_ENCODING_UTF_8,
        .tagsHandlers = tagsHandlers,
        .onErrorCb    = onErrorCb,
        .userData     = NULL
    };

    enum parser_error_e ret = obj->parse(obj, &params);
    TEST_ASSERT_NOT_EQUAL(ret, PARSER_ERROR_NONE);

    TEST_ASSERT_EQUAL_UINT32(1, nbCallsToErrorCb);
}

/*
 * Requirement:
 * - Provided userData must always be given back when callbacks
 *   are called
 */
void test_Parser_Parse_UserData_Integrity(void)
{
    struct parser_tags_handler_s tagsHandlers[] = {
        { NULL,  NULL,  NULL,  NULL }
    };

    const char *str = "userData";
    struct parser_params_s params = {
        .path         = INVALID_XML_FILE,
        .encoding     = PARSER_ENCODING_UTF_8,
        .tagsHandlers = tagsHandlers,
        .onErrorCb    = onErrorCb,
        .userData     = str
    };

    (void)obj->parse(obj, &params);
    TEST_ASSERT_EQUAL_MEMORY(str, receivedUserData, strlen(str));
    TEST_ASSERT_EQUAL_STRING(str, receivedUserData);
}
