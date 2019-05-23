//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2019 Boubacar DIENE                                           //
//                                                                                              //
//              This file is part of mmstreamer project.                                        //
//                                                                                              //
//              mmstreamer is free software: you can redistribute it and/or modify              //
//              it under the terms of the GNU General Public License as published by            //
//              the Free Software Foundation, either version 2 of the License, or               //
//              (at your option) any later version.                                             //
//                                                                                              //
//              mmstreamer is distributed in the hope that it will be useful,                   //
//              but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                   //
//              GNU General Public License for more details.                                    //
//                                                                                              //
//              You should have received a copy of the GNU General Public License               //
//              along with mmstreamer. If not, see <http://www.gnu.org/licenses/>               //
//              or write to the Free Software Foundation, Inc., 51 Franklin Street,             //
//              51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

/*!
* \file assert_test_helpers.h
* \author Boubacar DIENE
*/
#ifndef __ASSERT_TEST_HELPERS_H__
#define __ASSERT_TEST_HELPERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 * code_under_test is expected to assert
 * If not, the test is considered as not successful
 */
#define TEST_ASSERT_EXPECTED(code_under_test)                \
{                                                            \
    CEXCEPTION_T e;                                          \
    Try {                                                    \
        code_under_test;                                     \
        TEST_FAIL_MESSAGE("Code under test did not assert"); \
    } Catch(e) {}                                            \
}


/*!
 * code_under_test is not expected to assert
 * If not, the test is considered as not successful
 */
#define TEST_ASSERT_NOT_EXPECTED(code_under_test)                 \
{                                                                 \
    CEXCEPTION_T e;                                               \
    Try {                                                         \
        code_under_test;                                          \
    } Catch(e) {                                                  \
        TEST_FAIL_MESSAGE("Code under test failed an assertion"); \
    }                                                             \
}

#ifdef __cplusplus
}
#endif

#endif //__ASSERT_TEST_HELPERS_H__
