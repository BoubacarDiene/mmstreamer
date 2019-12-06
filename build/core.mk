##
#
# \file core.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create core module's objects files
#
##

MODULE_NAME := core

SOURCES := $(MODULE_NAME)/Core.c \
           $(MODULE_NAME)/configs/*.c \
           $(MODULE_NAME)/listeners/*.c \
           $(MODULE_NAME)/loaders/*.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))

# See
# - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
# - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80454
all: CFLAGS += -Wno-missing-braces -Wno-error=missing-braces
