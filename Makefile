##
#
# \file Makefile
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   September 04, 2016
#
# \brief  Build libraries:
#         - libvideo-x.y.so
#         - libnet-client-x.y.so
#         - libnet-server-x.y.so
#
#         - libexpat
#         - libfreetype
#         - libjpeg
#         - libSDL
#         - libSDL_image
#         - lSDL_ttf
#
#         and binary:
#         - mmstreamer
#
##

#################################################################
#               Implicit rules / Particular targets             #
#################################################################

# Disable implicit rules
.SUFFIXES:

# Define particular targets
.PHONY: prepare-sources, build-submodules, clean-all

#################################################################
#                             Include                           #
#################################################################

include build/Makefile.inc

#################################################################
#                            Variables                          #
#################################################################

# Binary name
ifeq (${DEBUG},no)
    BIN_NAME       := ${PROJECT_NAME}-${PROJECT_VERSION}
else
    BIN_NAME       := ${PROJECT_NAME}-${PROJECT_VERSION}.dbg
endif

# Path
OUT_BUILD_DIR      := ${OUT_BUILD}/main
OUT_BUILD_INC      := ${OUT_BUILD_DIR}/include
OUT_BUILD_SRC      := ${OUT_BUILD_DIR}/src
OUT_BUILD_BIN      := ${OUT_BUILD_DIR}/bin
OUT_BUILD_OBJS     := ${OUT_BUILD_DIR}/objs

OUT_BUILD_UTILS_H  := ${OUT_BUILD_INC}/utils
OUT_BUILD_UTILS_C  := ${OUT_BUILD_SRC}/utils

OUT_BUILD_GRAPHICS      := ${OUT_BUILD}/graphics
OUT_BUILD_INC_GRAPHICS  := ${OUT_BUILD_GRAPHICS}/include
OUT_BUILD_OBJS_GRAPHICS := ${OUT_BUILD_GRAPHICS}/objs

OUT_BUILD_NETWORK       := ${OUT_BUILD}/net
OUT_BUILD_INC_NETWORK   := ${OUT_BUILD_NETWORK}/include
OUT_BUILD_OBJS_NETWORK  := ${OUT_BUILD_NETWORK}/objs

OUT_BUILD_SPECIFIC      := ${OUT_BUILD}/specific
OUT_BUILD_INC_SPECIFIC  := ${OUT_BUILD_SPECIFIC}/include
OUT_BUILD_OBJS_SPECIFIC := ${OUT_BUILD_SPECIFIC}/objs

OUT_BUILD_CORE          := ${OUT_BUILD}/core
OUT_BUILD_INC_CORE      := ${OUT_BUILD_CORE}/include
OUT_BUILD_OBJS_CORE     := ${OUT_BUILD_CORE}/objs

OUT_BUILD_VIDEO         := ${OUT_BUILD}/video
OUT_BUILD_INC_VIDEO     := ${OUT_BUILD_VIDEO}/include
OUT_BUILD_OBJS_VIDEO    := ${OUT_BUILD_VIDEO}/objs

OUT_BUILD_DEPS          := ${OUT_BUILD}/deps
OUT_BUILD_INC_DEPS      := ${OUT_BUILD_DEPS}/include
OUT_BUILD_LIB_DEPS      := ${OUT_BUILD_DEPS}/lib

# Build options
HEADERS            := -I${OUT_BUILD_INC}           \
                       -I${OUT_BUILD_INC_GRAPHICS} \
                       -I${OUT_BUILD_INC_NETWORK}  \
                       -I${OUT_BUILD_INC_SPECIFIC} \
                       -I${OUT_BUILD_INC_CORE}     \
                       -I${OUT_BUILD_INC_VIDEO}    \
                       -I${OUT_BUILD_INC_DEPS}
CFLAGS             += ${HEADERS} -DMAIN_CONFIG_FILE=\"${OUT_RELEASE}/res/Main.xml\"

DEPS_LDFLAGS       := -L${OUT_BUILD_LIB_DEPS} -Wl,-rpath,${OUT_BUILD_LIB_DEPS}
DEPS_LDFLAGS       += -lSDL -lSDL_image -lSDL_ttf -lfreetype -ljpeg -lexpat
LDFLAGS            += ${DEPS_LDFLAGS}

# Files
MAIN               := Main

# Objects
OBJS := ${OUT_BUILD_SRC}/${LIST}.o   \
        ${OUT_BUILD_SRC}/${PARSER}.o \
		${OUT_BUILD_SRC}/${TASK}.o   \
		${OUT_BUILD_SRC}/${MAIN}.o
	
#################################################################
#                               Build                           #
#################################################################

# all
all: prepare-sources build-submodules ${BIN_NAME}

# Binary
${BIN_NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${OUT_BUILD_BIN}/$@ ${OUT_BUILD_OBJS}/*.o ${OUT_BUILD_OBJS_GRAPHICS}/*.o ${OUT_BUILD_OBJS_SPECIFIC}/*.o \
            ${OUT_BUILD_OBJS_CORE}/*.o \
            ${shell find ${OUT_BUILD_OBJS_VIDEO}/*.o ! -name $(notdir ${LIST}.o) ! -name $(notdir ${TASK}.o)} \
            ${shell find ${OUT_BUILD_OBJS_NETWORK}/*.o ! -name $(notdir ${LIST}.o) ! -name $(notdir ${TASK}.o)} \
            ${LDFLAGS}

# objects
%.o: %.c
	${CC} ${CFLAGS} -c $< -o ${OUT_BUILD_OBJS}/$(notdir $@)
	
#################################################################
#                              Install                          #
#################################################################

install: prepare-release
	make -f build/Makefile.vid install
	make -f build/Makefile.net install
	
	${CP} ${OUT_BUILD_BIN}/${BIN_NAME} ${OUT_RELEASE_BIN}/.
	${CP} ${RES}/*                     ${OUT_RELEASE_RES}/.
	
	cd ${shell dirname ${OUT_RELEASE}} && \
	tar czvf ${shell basename ${OUT_RELEASE}}.tar.gz ${shell basename ${OUT_RELEASE}} ||: && \
	cd -

#################################################################
#                             Prepare                           #
#################################################################

prepare-sources:
	${PRINT} ***** Preparing build directories *****
	if [ ! -d ${OUT_BUILD_INC} ]; then   \
	    ${MKDIR} ${OUT_BUILD_INC};       \
	fi
	if [ ! -d ${OUT_BUILD_SRC} ]; then   \
	    ${MKDIR} ${OUT_BUILD_SRC};       \
	fi
	if [ ! -d ${OUT_BUILD_BIN} ]; then   \
	    ${MKDIR} ${OUT_BUILD_BIN};       \
	fi
	if [ ! -d ${OUT_BUILD_OBJS} ]; then   \
	    ${MKDIR} ${OUT_BUILD_OBJS};       \
	fi
	if [ ! -d ${OUT_BUILD_UTILS_H} ]; then \
	    ${MKDIR} ${OUT_BUILD_UTILS_H};     \
	fi
	if [ ! -d ${OUT_BUILD_UTILS_C} ]; then \
	    ${MKDIR} ${OUT_BUILD_UTILS_C};     \
	fi

	${PRINT} ***** Copying network sources *****
	${CP} ${INC}/${COMMON}.h     ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${LIST}.h       ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${LOG}.h        ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${PARSER}.h     ${OUT_BUILD_UTILS_H}/.
	${CP} ${INC}/${TASK}.h       ${OUT_BUILD_UTILS_H}/.
	
	${CP} ${SRC}/${LIST}.c       ${OUT_BUILD_UTILS_C}/.
	${CP} ${SRC}/${PARSER}.c     ${OUT_BUILD_UTILS_C}/.
	${CP} ${SRC}/${TASK}.c       ${OUT_BUILD_UTILS_C}/.
	
	${CP} ${SRC}/${MAIN}.c       ${OUT_BUILD_SRC}/.

#################################################################
#                            Submodules                         #
#################################################################

build-submodules:
	${PRINT} ***** Building all submodules *****
	make -f build/Makefile.vid  all
	make -f build/Makefile.net  all
	make -f build/Makefile.deps all
	make -f build/Makefile.gfx  all
	make -f build/Makefile.spec all
	make -f build/Makefile.core all

#################################################################
#                              Clean                            #
#################################################################

clean-all:
	${PRINT} ***** Removing all generated files *****
	make -f build/Makefile.vid  clean-video
	make -f build/Makefile.net  clean-network
	make -f build/Makefile.deps clean-dependencies
	make -f build/Makefile.gfx  clean-graphics
	make -f build/Makefile.spec clean-spec
	make -f build/Makefile.core clean-core
	${RM} ${OUT}/${PROJECT_NAME}-${PROJECT_VERSION}* ||:

mrproper-all: clean-all
	${PRINT} ***** Cleaning all outputs *****
	make -f build/Makefile.vid  mrproper-video
	make -f build/Makefile.net  mrproper-network
	make -f build/Makefile.deps mrproper-dependencies
	make -f build/Makefile.gfx  mrproper-graphics
	make -f build/Makefile.spec mrproper-spec
	make -f build/Makefile.core mrproper-core
	${RM} ${OUT} ||:
