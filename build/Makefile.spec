##
#
# \file Makefile.spec
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   September 04, 2016
#
# \brief  Create specific .o files
#
##

#################################################################
#               Implicit rules / Particular targets             #
#################################################################

# Disable implicit rules
.SUFFIXES:

# Define particular targets
.PHONY: prepare-specific-sources, clean-specific

#################################################################
#                             Include                           #
#################################################################

include build/Makefile.inc

#################################################################
#                            Variables                          #
#################################################################

# Path
OUT_BUILD_DIR  := ${OUT_BUILD}/specific
OUT_BUILD_INC  := ${OUT_BUILD_DIR}/include
OUT_BUILD_SRC  := ${OUT_BUILD_DIR}/src
OUT_BUILD_OBJS := ${OUT_BUILD_DIR}/objs

OUT_BUILD_UTILS_H := ${OUT_BUILD_INC}/utils
OUT_BUILD_CORE_H  := ${OUT_BUILD_INC}/core

OUT_BUILD_SPECIFIC_H := ${OUT_BUILD_INC}/specific
OUT_BUILD_SPECIFIC_C := ${OUT_BUILD_SRC}/specific

OUT_BUILD_GRAPHICS     := ${OUT_BUILD}/graphics
OUT_BUILD_INC_GRAPHICS := ${OUT_BUILD_GRAPHICS}/include

OUT_BUILD_NETWORK     := ${OUT_BUILD}/net
OUT_BUILD_INC_NETWORK := ${OUT_BUILD_NETWORK}/include

OUT_BUILD_VIDEO     := ${OUT_BUILD}/video
OUT_BUILD_INC_VIDEO := ${OUT_BUILD_VIDEO}/include

# Build options
HEADERS := -I${OUT_BUILD_INC}          \
           -I${OUT_BUILD_INC_GRAPHICS} \
           -I${OUT_BUILD_INC_NETWORK}  \
           -I${OUT_BUILD_INC_VIDEO}
CFLAGS  += ${HEADERS}

# Files
CONFIG   := specific/Config
HANDLERS := specific/Handlers
SPECIFIC := specific/Specific

CORE_COMMON := core/Common

# Objects
OBJS := ${OUT_BUILD_SRC}/${CONFIG}.o \
		${OUT_BUILD_SRC}/${HANDLERS}.o   \
		${OUT_BUILD_SRC}/${SPECIFIC}.o
	
#################################################################
#                               Build                           #
#################################################################

# all
all: prepare-specific-sources ${OBJS}

# objects
%.o: %.c
	${CC} ${CFLAGS} -c $< -o ${OUT_BUILD_OBJS}/$(notdir $@)

#################################################################
#                             Prepare                           #
#################################################################

prepare-specific-sources:
	${PRINT} ***** Preparing specific build directories *****
	if [ ! -d ${OUT_BUILD_INC} ]; then   \
	    ${MKDIR} ${OUT_BUILD_INC};       \
	fi
	if [ ! -d ${OUT_BUILD_SRC} ]; then   \
	    ${MKDIR} ${OUT_BUILD_SRC};       \
	fi
	if [ ! -d ${OUT_BUILD_OBJS} ]; then   \
	    ${MKDIR} ${OUT_BUILD_OBJS};       \
	fi
	if [ ! -d ${OUT_BUILD_UTILS_H} ]; then \
	    ${MKDIR} ${OUT_BUILD_UTILS_H};     \
	fi
	if [ ! -d ${OUT_BUILD_CORE_H} ]; then \
	    ${MKDIR} ${OUT_BUILD_CORE_H};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_H} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_H};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_C};     \
	fi

	${PRINT} ***** Copying specific sources *****
	${CP} ${INC}/${COMMON}.h      ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${LOG}.h         ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${PARSER}.h ${OUT_BUILD_UTILS_H}/.

	${CP} ${INC}/${CORE_COMMON}.h ${OUT_BUILD_CORE_H}/.

	${CP} ${INC}/${SPECIFIC}.h    ${OUT_BUILD_SPECIFIC_H}/.
	
	${CP} ${SRC}/${CONFIG}.c   ${OUT_BUILD_SPECIFIC_C}/.
	${CP} ${SRC}/${HANDLERS}.c ${OUT_BUILD_SPECIFIC_C}/.
	${CP} ${SRC}/${SPECIFIC}.c ${OUT_BUILD_SPECIFIC_C}/.

#################################################################
#                              Clean                            #
#################################################################

clean-specific:
	${PRINT} ***** Removing generated specific files *****
	${RM} ${OUT_BUILD_OBJS} ||:

mrproper-specific: clean-specific
	${PRINT} ***** Cleaning specific output *****
	${RM} ${OUT_BUILD} ||:
