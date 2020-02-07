##
#
# \file common.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Common settings
#
##

#################################################################
#                             Project                           #
#################################################################

# Infos
PROJECT_NAME    := mmstreamer
PROJECT_VERSION := 1.3

# Build options
#   - DEBUG = ", separated from gdb, asan, secu" (default: release)
#   - LOG_LEVEL = <1 to 4>
#
#   Examples:
#   - make all install
#   - make all install DEBUG=gdb LOG_LEVEL=3
#   - make all install DEBUG=gdb,asan,secu
DEBUG     ?= no
LOG_LEVEL ?= 1

# https://linux.die.net/man/1/gcc
# https://security.stackexchange.com/questions/24444/what-is-the-most-hardened-set-of-options-for-gcc-compiling-c-c
LDFLAGS_OPTIONS := -pthread -lm -ldl
CFLAGS_OPTIONS := -Wall -Wextra -Werror -Wconversion -Wsign-conversion \
                   -Wuninitialized -Wparentheses -Winit-self -Wcomment

CFLAGS_OPTIONS += $(if $(findstring no,$(DEBUG)),-O3 -s -DNDEBUG,)
CFLAGS_OPTIONS += $(if $(findstring gdb,$(DEBUG)),-ggdb,)
CFLAGS_OPTIONS += $(if $(findstring asan,$(DEBUG)),-fsanitize=address -fno-omit-frame-pointer,)

ifneq (,$(findstring secu,$(DEBUG)))
LDFLAGS_OPTIONS += -Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack
CFLAGS_OPTIONS  += -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security \
                   -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4 \
                   -pie -fPIE -ftrapv
endif

CC      := gcc
LDFLAGS := $(LDFLAGS_OPTIONS)
CFLAGS   = $(CFLAGS_OPTIONS) \
          -DLOG_LEVEL=$(LOG_LEVEL) \
          -DPROJECT_NAME=\"$(PROJECT_NAME)\" \
          -DPROJECT_VERSION=\"$(PROJECT_VERSION)\" \
          -I$(OUT_STAGING_INC)

# Shell commands
CP    := cp -rf
RM    := rm -rf
MKDIR := mkdir -p
PRINT := @echo

#################################################################
#                             Sources                           #
#################################################################

LOCAL := $(shell pwd)

# Versions
SDL_BUILD_VERSION ?= 2
RES_VERSION       := drawer$(SDL_BUILD_VERSION)

# Sources
DEPS := $(LOCAL)/deps
INC  := $(LOCAL)/inc
RES  := $(LOCAL)/res/$(RES_VERSION)
SRC  := $(LOCAL)/src

# Files needed to build the module
# - SOURCES: relative to src/ directory
SOURCES +=
# - OBJECTS: relative to src/ directory
#   Example: SOURCES = a/aa.c b/*.c
#            Step 1:   $(SRC)/a/aa.c $(SRC)/b/*.c
#            Step 2:   $(SRC)/a/aa.c $(SRC)/b/bb1.c $(SRC)/b/bb2.c
#            Step 3:   a/aa.c b/bb1.c b/bb2.c
#            Step 4:   a/aa b/bb1 b/bb2
#            Step 5:   a/aa.o b/bb1.o b/bb2.o
OBJECTS := $(addsuffix .o,$(basename $(subst $(SRC)/,,\
               $(wildcard $(addprefix $(SRC)/,$(SOURCES))))))

#################################################################
#                             Output                            #
#################################################################

OUT := $(LOCAL)/out

# Staging
OUT_STAGING      := $(OUT)/staging
OUT_STAGING_INC  := $(OUT_STAGING)/inc
OUT_STAGING_LIB  := $(OUT_STAGING)/lib
OUT_STAGING_OBJS := $(OUT_STAGING)/objs

# Build
# MODULE_NAME: To be specified before including common.mk
OUT_BUILD_DIR = $(OUT)/build/$(MODULE_NAME)

#################################################################
#                              Rules                            #
#################################################################

.PHONY: prepare all install clean
.DEFAULT_GOAL := all
.NOTPARALLEL:

#
## Helpers
#

# $1: Directories whose contents have to be copied
# $2: Path to directory where to copy contents
define copy-content
	@- $(foreach item,$1, \
		$(eval src := $(item)/*) \
		$(eval dst := $2/.) \
		\
		$(MKDIR) $(dst) && $(CP) $(src) $(dst) ; \
	)
endef

# $1: Files to copy
# $2: Path to directory containing given files
# $3: Path to directory where to copy files
define copy-files
	@- $(foreach item,$1, \
		$(eval src := $2/$(item)) \
		$(eval dst := $3/$(dir $(item))) \
		\
		$(MKDIR) $(dst) && $(CP) $(src) $(dst) ; \
	)
endef

# $1: Files to remove
# $2: Path to directory containing given files
define remove-files
	@- $(foreach item,$1, \
		$(eval files := $2/$(basename $(item)).*) \
		\
		$(RM) $(files) ; \
	)
endef

#
## Rules
#

define declare-common-rules
prepare:
	$(call copy-content,$(INC),$(OUT_STAGING_INC))
	$(call copy-files,$(SOURCES),$(SRC),$(OUT_BUILD_DIR))

all: prepare $(addprefix $(OUT_BUILD_DIR)/,$(OBJECTS))

install:
	$(call copy-files,$(OBJECTS),$(OUT_BUILD_DIR),$(OUT_STAGING_OBJS))

clean:
	$(call remove-files,$(SOURCES),$(OUT_BUILD_DIR))
	$(call remove-files,$(OBJECTS),$(OUT_STAGING_OBJS))
endef

