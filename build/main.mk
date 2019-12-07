##
#
# \file main.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create main's objects files
#
##

MODULE_NAME := main

SOURCES := utils/List.c utils/Parser.c utils/Task.c Main.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))

# See
# - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
# - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80454
all: CFLAGS += -Wno-missing-braces -Wno-error=missing-braces

# Default path to Main.xml file
all: CFLAGS += -DMAIN_XML_FILE=\"$(RES_RELATIVE_DIR)/Main.xml\"
