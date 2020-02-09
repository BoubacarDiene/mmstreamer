//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              //
//              Copyright © 2016, 2018 Boubacar DIENE                                           //
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
* \file Handlers.c
* \brief TODO
* \author Boubacar DIENE
*/

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// HEADERS ///////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#include "control/Handlers.h"

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////////////////// MACROS ////////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

#undef  TAG
#define TAG "Handlers"

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////// PUBLIC FUNCTIONS PROTOTYPES //////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

static enum handlers_error_e getCommandHandler_f(struct handlers_s *obj, const char *handlerName,
                                                 handlers_command_f *out);
static enum handlers_error_e getElementIndex_f(struct handlers_s *obj, char *elementName,
                                               uint32_t *index);
static enum handlers_error_e getSubstring_f(struct handlers_s *obj, const char *haystack,
                                            const char *needle, char *out, uint32_t *offset);

/* -------------------------------------------------------------------------------------------- */
/* ///////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

extern struct handlers_commands_s gSingleInputHandlers[];
extern uint32_t gNbSingleInputHandlers;

extern struct handlers_commands_s gMultiInputsHandlers[];
extern uint32_t gNbMultiInputsHandlers;

/* -------------------------------------------------------------------------------------------- */
/* /////////////////////////////////////// INITIALIZER //////////////////////////////////////// */
/* -------------------------------------------------------------------------------------------- */

/*!
 *
 */
enum handlers_error_e Handlers_Init(struct handlers_s **obj,
                                    struct handlers_params_s *handlersParams)
{
    ASSERT(obj && handlersParams && (*obj = calloc(1, sizeof(struct handlers_s))));

    struct handlers_private_data_s *pData;
    ASSERT((pData = calloc(1, sizeof(struct handlers_private_data_s))));

    (*obj)->getCommandHandler = getCommandHandler_f;
    (*obj)->getElementIndex   = getElementIndex_f;
    (*obj)->getSubstring      = getSubstring_f;

    pData->handlersParams        = *handlersParams;

    pData->nbSingleInputHandlers = gNbSingleInputHandlers;
    pData->singleInputHandlers   = gSingleInputHandlers;

    pData->nbMultiInputsHandlers = gNbMultiInputsHandlers;
    pData->multiInputsHandlers   = gMultiInputsHandlers;

    (*obj)->pData = (void*)pData;

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
enum handlers_error_e Handlers_UnInit(struct handlers_s **obj)
{
    ASSERT(obj && *obj);

    free((*obj)->pData);
    free(*obj);
    *obj = NULL;

    return HANDLERS_ERROR_NONE;
}

/* -------------------------------------------------------------------------------------------- */
/* ////////////////////////////// PUBLIC FUNCTIONS IMPLEMENTATION ///////////////////////////// */
/* -------------------------------------------------------------------------------------------- */


/*!
 *
 */
static enum handlers_error_e getCommandHandler_f(struct handlers_s *obj, const char *handlerName,
                                                 handlers_command_f *out)
{
    ASSERT(obj && obj->pData && handlerName && out);

    struct handlers_private_data_s *pData = (struct handlers_private_data_s*)(obj->pData);

    uint32_t i = 0;
    while ((i < pData->nbSingleInputHandlers)
            && pData->singleInputHandlers[i].name
            && (strcmp(pData->singleInputHandlers[i].name, handlerName) != 0)) {
        ++i;
    }

    if (pData->singleInputHandlers[i].name) {
        *out = pData->singleInputHandlers[i].fct;
        goto exit;
    }

    i = 0;
    while ((i < pData->nbMultiInputsHandlers)
            && pData->multiInputsHandlers[i].name
            && (strcmp(pData->multiInputsHandlers[i].name, handlerName) != 0)) {
        ++i;
    }

    if (pData->multiInputsHandlers[i].name) {
        *out = pData->multiInputsHandlers[i].fct;
        goto exit;
    }

    Loge("\"%s\" not found", handlerName);

    return HANDLERS_ERROR_PARAMS;

exit:
    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
static enum handlers_error_e getElementIndex_f(struct handlers_s *obj, char *elementName,
                                                                       uint32_t *index)
{
    ASSERT(obj && obj->pData && elementName && index);

    struct handlers_private_data_s *pData  = (struct handlers_private_data_s*)(obj->pData);
    struct graphics_infos_s *graphicsInfos = &pData->handlersParams.ctx->params.graphicsInfos;
    uint32_t nbGfxElements                 = graphicsInfos->nbGfxElements;
    struct gfx_element_s **gfxElements     = graphicsInfos->gfxElements;

    for (*index = 0; *index < nbGfxElements; (*index)++) {
        if (strcmp(gfxElements[*index]->name, elementName) == 0) {
            break;
        }
    }

    if (*index >= nbGfxElements) {
        Loge("Element \"%s\" not found", elementName);
        return HANDLERS_ERROR_PARAMS;
    }

    return HANDLERS_ERROR_NONE;
}

/*!
 *
 */
enum handlers_error_e getSubstring_f(struct handlers_s *obj, const char * const haystack,
                                     const char * const needle, char *out, uint32_t *offset)
{
    ASSERT(obj && obj->pData);

    if (!haystack || !needle || !out || !offset) {
        Loge("Bad params");
        return HANDLERS_ERROR_PARAMS;
    }

    char *result = strstr(haystack + *offset, needle);
    if (!result) {
        return HANDLERS_ERROR_PARAMS;
    }

    size_t len = strlen(haystack + *offset) - strlen(result);
    if (len > 0) {
        snprintf(out, len + 1, "%s", haystack + *offset);
    }

    *offset += (uint32_t)(len + 1);

    return HANDLERS_ERROR_NONE;
}
