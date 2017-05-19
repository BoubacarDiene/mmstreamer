##
#
# \file Makefile.inc
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   September 04, 2016
#
# \brief  Common settings
#
##

#################################################################
#                            Variables                          #
#################################################################

# Project
PROJECT_NAME    := mmstreamer
PROJECT_VERSION := 0.7

# Path
LOCAL := ${shell pwd}

DEPS := ${LOCAL}/deps
INC  := ${LOCAL}/inc
RES  := ${LOCAL}/res
SRC  := ${LOCAL}/src

OUT             := ${LOCAL}/out
OUT_BUILD       := ${OUT}/build
OUT_RELEASE     := ${OUT}/${PROJECT_NAME}
OUT_RELEASE_BIN := ${OUT_RELEASE}/bin
OUT_RELEASE_INC := ${OUT_RELEASE}/inc
OUT_RELEASE_LIB := ${OUT_RELEASE}/lib
OUT_RELEASE_RES := ${OUT_RELEASE}/res

# Utils
COMMON := utils/Common
LIST   := utils/List
LOG    := utils/Log
PARSER := utils/Parser
TASK   := utils/Task

# Build
DEBUG     := no
LOG_LEVEL := 1
OPTIONS   := -Wall -Wextra -Wno-missing-braces -Wno-unused-parameter -Wno-unused-variable

ifeq (${DEBUG},no)
	OPTIONS += -O3 -s
else
	OPTIONS += -g
endif

CC      := gcc
CFLAGS  := ${OPTIONS} -DLOG_LEVEL=${LOG_LEVEL} -DPROJECT_NAME=\"${PROJECT_NAME}\" -DPROJECT_VERSION=\"${PROJECT_VERSION}\"
LDFLAGS := -pthread -lm -ldl

# Shell commands
CP    := cp -rf
RM    := rm -rf
MKDIR := mkdir -p
PRINT := @echo

# Versions
VIDEO_LIB_VERSION  := 1.0
CLIENT_LIB_VERSION := 1.0
SERVER_LIB_VERSION := 1.0

EXPAT_LIB_VERSION    := 2.2.0
FREETYPE_LIB_VERSION := 2.4.0
JPEG_LIB_VERSION     := 8d

SDL_BUILD_VERSION    := 2
ifeq (${SDL_BUILD_VERSION},2)
    SDL_MAIN_VERSION  := 2.0.5
    SDL_IMAGE_VERSION := 2.0.1
    SDL_TTF_VERSION   := 2.0.14
else
    SDL_MAIN_VERSION  := 1.2.15
    SDL_IMAGE_VERSION := 1.2.12
    SDL_TTF_VERSION   := 2.0.11
endif

#################################################################
#                             Prepare                           #
#################################################################

prepare-release:
	${PRINT} ***** Preparing release *****
	if [ ! -d ${OUT_RELEASE_BIN} ]; then \
	    ${MKDIR} ${OUT_RELEASE_BIN};     \
	fi
	if [ ! -d ${OUT_RELEASE_INC} ]; then \
	    ${MKDIR} ${OUT_RELEASE_INC};     \
	fi
	if [ ! -d ${OUT_RELEASE_LIB} ]; then \
	    ${MKDIR} ${OUT_RELEASE_LIB};     \
	fi
	if [ ! -d ${OUT_RELEASE_RES} ]; then \
	    ${MKDIR} ${OUT_RELEASE_RES};     \
	fi
