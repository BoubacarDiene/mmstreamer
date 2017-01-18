##
#
# \file Makefile.spec
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   September 04, 2016
#
# \brief  Create .o files for specific code
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

OUT_BUILD_SPECIFIC_H           := ${OUT_BUILD_INC}/specific
OUT_BUILD_SPECIFIC_C           := ${OUT_BUILD_SRC}/specific
OUT_BUILD_SPECIFIC_CONFIGS_C   := ${OUT_BUILD_SPECIFIC_C}/configs
OUT_BUILD_SPECIFIC_LISTENERS_C := ${OUT_BUILD_SPECIFIC_C}/listeners
OUT_BUILD_SPECIFIC_LOADERS_C   := ${OUT_BUILD_SPECIFIC_C}/loaders

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
CONFIG          := specific/Config
LISTENERS       := specific/Listeners
LOADERS         := specific/Loaders
SPECIFIC        := specific/Specific
SPECIFIC_COMMON := specific/Common
XML_DEFINES     := specific/XmlDefines

VIDEO_CONFIG := specific/configs/VideoConfig

CLIENTS_LISTENERS  := specific/listeners/ClientsListeners
GRAPHICS_LISTENERS := specific/listeners/GraphicsListeners
SERVERS_LISTENERS  := specific/listeners/ServersListeners
VIDEO_LISTENERS    := specific/listeners/VideoListeners

CLIENTS_LOADER  := specific/loaders/ClientsLoader
GRAPHICS_LOADER := specific/loaders/GraphicsLoader
SERVERS_LOADER  := specific/loaders/ServersLoader
VIDEO_LOADER    := specific/loaders/VideoLoader

# Objects
OBJS := ${OUT_BUILD_SRC}/${VIDEO_CONFIG}.o       \
		${OUT_BUILD_SRC}/${CLIENTS_LISTENERS}.o  \
		${OUT_BUILD_SRC}/${GRAPHICS_LISTENERS}.o \
		${OUT_BUILD_SRC}/${SERVERS_LISTENERS}.o  \
		${OUT_BUILD_SRC}/${VIDEO_LISTENERS}.o    \
		${OUT_BUILD_SRC}/${CLIENTS_LOADER}.o     \
		${OUT_BUILD_SRC}/${GRAPHICS_LOADER}.o    \
		${OUT_BUILD_SRC}/${SERVERS_LOADER}.o     \
		${OUT_BUILD_SRC}/${VIDEO_LOADER}.o       \
		${OUT_BUILD_SRC}/${LISTENERS}.o          \
		${OUT_BUILD_SRC}/${LOADERS}.o            \
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
	if [ ! -d ${OUT_BUILD_SPECIFIC_H} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_H};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_C};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_CONFIGS_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_CONFIGS_C};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_LISTENERS_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_LISTENERS_C};     \
	fi
	if [ ! -d ${OUT_BUILD_SPECIFIC_LOADERS_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_SPECIFIC_LOADERS_C};     \
	fi

	${PRINT} ***** Copying specific sources *****
	${CP} ${INC}/${COMMON}.h ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${PARSER}.h ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${LOG}.h    ${OUT_BUILD_UTILS_H}/.
	
	${CP} ${INC}/${CONFIG}.h          ${OUT_BUILD_SPECIFIC_H}/.
	${CP} ${INC}/${LISTENERS}.h       ${OUT_BUILD_SPECIFIC_H}/.
	${CP} ${INC}/${LOADERS}.h         ${OUT_BUILD_SPECIFIC_H}/.
	${CP} ${INC}/${SPECIFIC}.h        ${OUT_BUILD_SPECIFIC_H}/.
	${CP} ${INC}/${SPECIFIC_COMMON}.h ${OUT_BUILD_SPECIFIC_H}/.
	${CP} ${INC}/${XML_DEFINES}.h     ${OUT_BUILD_SPECIFIC_H}/.
	
	${CP} ${SRC}/${VIDEO_CONFIG}.c ${OUT_BUILD_SPECIFIC_CONFIGS_C}/.
	
	${CP} ${SRC}/${CLIENTS_LISTENERS}.c  ${OUT_BUILD_SPECIFIC_LISTENERS_C}/.
	${CP} ${SRC}/${GRAPHICS_LISTENERS}.c ${OUT_BUILD_SPECIFIC_LISTENERS_C}/.
	${CP} ${SRC}/${SERVERS_LISTENERS}.c  ${OUT_BUILD_SPECIFIC_LISTENERS_C}/.
	${CP} ${SRC}/${VIDEO_LISTENERS}.c    ${OUT_BUILD_SPECIFIC_LISTENERS_C}/.
	
	${CP} ${SRC}/${CLIENTS_LOADER}.c  ${OUT_BUILD_SPECIFIC_LOADERS_C}/.
	${CP} ${SRC}/${GRAPHICS_LOADER}.c ${OUT_BUILD_SPECIFIC_LOADERS_C}/.
	${CP} ${SRC}/${SERVERS_LOADER}.c  ${OUT_BUILD_SPECIFIC_LOADERS_C}/.
	${CP} ${SRC}/${VIDEO_LOADER}.c    ${OUT_BUILD_SPECIFIC_LOADERS_C}/.
	
	${CP} ${SRC}/${LISTENERS}.c ${OUT_BUILD_SPECIFIC_C}/.
	${CP} ${SRC}/${LOADERS}.c   ${OUT_BUILD_SPECIFIC_C}/.
	${CP} ${SRC}/${SPECIFIC}.c  ${OUT_BUILD_SPECIFIC_C}/.

#################################################################
#                              Clean                            #
#################################################################

clean-specific:
	${PRINT} ***** Removing generated specific files *****
	${RM} ${OUT_BUILD_OBJS} ||:

mrproper-specific: clean-specific
	${PRINT} ***** Cleaning specific output *****
	${RM} ${OUT_BUILD} ||: