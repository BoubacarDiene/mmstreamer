#include "unity.h"
#include "exception_test_helpers.h"
#include "utils/List.h"

static struct list_s *obj          = NULL;
static struct list_params_s params = {0};
static uint32_t elements[]         = {1, 2, 3, 1};

static uint32_t nbCallsToBrowseCb = 0;
static uint32_t receivedElement   = 0;
static char *receivedUserdata     = NULL;

static void browseCb(struct list_s *obj, void *element, void *userData)
{
    (void)obj;

    receivedElement  = *((uint32_t*)element);
	receivedUserdata = (char*)userData;
    nbCallsToBrowseCb++;
}

void setUp(void)
{
    nbCallsToBrowseCb = 0;
	receivedElement   = 0;
	receivedUserdata  = NULL;

    params.browseCb = browseCb;
    (void)List_Init(&obj, &params);
}

void tearDown(void)
{
    (void)List_UnInit(&obj);
}

/* -------------------------------------------------------------------------------------------- */
/*                                       GET NB ELEMENTS                                        */
/* -------------------------------------------------------------------------------------------- */

/**
 Requirement:
 - getNbElements() must "assert" when at least one of its input parameters is NULL
 */
void test_List_Get_Nb_Elements_Null_Parameter(void)
{
    uint32_t nbElements = 0;
    TEST_ASSERT_EXPECTED(obj->getNbElements(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->getNbElements(NULL, &nbElements));
}

/**
 Requirement:
 - getNbElements() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Get_Nb_Elements_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};
    uint32_t nbElements   = 0;

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->getNbElements(&fakeObj, &nbElements));
}

/**
 Requirement:
 - getNbElements() must set "nbElements" to the exact number of elements currently
 - in the list
 */
void test_List_Get_Nb_Elements_Valid_Input_Parameters(void)
{
    uint32_t nbElements   = 0;
    enum list_error_e ret = LIST_ERROR_NONE;

    for (uint32_t i = 0; i < SIZEOF_ARRAY(elements); ++i) {
        ret = obj->add(obj, &elements[i]);
        TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    }

    ret = obj->getNbElements(obj, &nbElements);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(SIZEOF_ARRAY(elements), nbElements);
}

/**
 Requirement:
 - getNbElements() must set "nbElements" to 0 in case the list is empty
 */
void test_List_Get_Nb_Elements_Empty_List(void)
{
    uint32_t nbElements   = 0;
    enum list_error_e ret = LIST_ERROR_NONE;

    ret = obj->getNbElements(obj, &nbElements);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(0, nbElements);
}

/* -------------------------------------------------------------------------------------------- */
/*                                          GET ELEMENT                                         */
/* -------------------------------------------------------------------------------------------- */

/**
 Requirement:
 - getElement() must "assert" when at least one of its input parameters is NULL
 */
void test_List_Get_Element_Null_Parameter(void)
{
    void *element = NULL;
    TEST_ASSERT_EXPECTED(obj->getElement(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->getElement(NULL, &element));
}

/**
 Requirement:
 - getElement() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Get_Element_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};
    void *element         = NULL;

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->getElement(&fakeObj, &element));
}

/**
 Requirement:
 - getElement() must return a valid element from the list without error
 */
void test_List_Get_Element_Valid_Input_Parameters(void)
{
    void *element         = NULL;
    enum list_error_e ret = LIST_ERROR_NONE;
    
    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->getElement(obj, &element);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_EQUAL_UINT32(elements[0], *((uint32_t*)element));
}

/**
 Requirement:
 - getElement() must keep "element" unchanged (=> NULL) when getting an eleent
   from an empty list
 */
void test_List_Get_Element_Empty_List(void)
{
    void *element         = NULL;
    enum list_error_e ret = LIST_ERROR_NONE;
        
    ret = obj->getElement(obj, &element);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);
    TEST_ASSERT_NULL(element);
}

/* -------------------------------------------------------------------------------------------- */
/*                                       BROWSE ELEMENTS                                        */
/* -------------------------------------------------------------------------------------------- */

/**
 Requirement:
 - browseElements() must "assert" when "obj" is NULL
 - browseElements() must not "assert" when "userData" is NULL
 */
void test_List_Browse_Elements_Null_Parameter(void)
{
	char userdata[] = "myuserdata";

    TEST_ASSERT_NOT_EXPECTED(obj->browseElements(obj, NULL));
    TEST_ASSERT_EXPECTED(obj->browseElements(NULL, (void**)&userdata));
}

/**
 Requirement:
 - browseElements() must "crash" when "obj" has not been obtained using List_Init()
 */
void test_List_Browse_Elements_Bad_Memory_Access(void)
{
    struct list_s fakeObj = {0};

    TEST_BAD_MEMORY_ACCESS_EXPECTED(obj->browseElements(&fakeObj, NULL));
}

/**
 Requirement:
 - browseElements() must return an error when no browse callback has been provided
 */
void test_List_Browse_Elements_No_Browse_Callback_Provided(void)
{
    struct list_s *objNoBrowseCallback = NULL;
    enum list_error_e ret              = LIST_ERROR_NONE;

	params.browseCb = NULL;
    ret = List_Init(&objNoBrowseCallback, &params);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = objNoBrowseCallback->add(objNoBrowseCallback, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

	ret = objNoBrowseCallback->browseElements(objNoBrowseCallback, NULL);
    TEST_ASSERT_NOT_EQUAL(ret, LIST_ERROR_NONE);

    ret = List_UnInit(&objNoBrowseCallback);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
}

/**
 Requirement:
 - browseElements() must return no error when browsing a list containing at least one element
 - browseCb() must be called as many times as there are elements in the list
 - elements returned through browseCb() must those previously added in the list
 */
void test_List_Browse_Elements_Valid_Input_Parameters(void)
{
    enum list_error_e ret = LIST_ERROR_NONE;

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->browseElements(obj, NULL);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
	TEST_ASSERT_EQUAL_UINT32(1, nbCallsToBrowseCb);
    TEST_ASSERT_EQUAL_UINT32(elements[0], receivedElement);
}

/**
 Requirement:
 - browseElements() must return no error when browsing a list containing at least one element
 - The provided userdata must be returned through browseCb()
 */
void test_List_Browse_Elements_Same_Userdata(void)
{
    char userdata[]       = "myuserdata";
    enum list_error_e ret = LIST_ERROR_NONE;

    ret = obj->add(obj, &elements[0]);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);

    ret = obj->browseElements(obj, (void**)&userdata);
    TEST_ASSERT_EQUAL(ret, LIST_ERROR_NONE);
	TEST_ASSERT_EQUAL_UINT32(1, nbCallsToBrowseCb);
    TEST_ASSERT_EQUAL_UINT32(elements[0], receivedElement);
	TEST_ASSERT_EQUAL_STRING(userdata, receivedUserdata);
}

/**
 Requirement:
 - browseElements() must return an error when used with an empty list
 */
void test_List_Browse_Elements_Empty_List(void)
{
    TEST_ASSERT_NOT_EQUAL(obj->browseElements(obj, NULL), LIST_ERROR_NONE);
}
