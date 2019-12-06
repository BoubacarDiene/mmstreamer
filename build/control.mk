##
#
# \file control.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create control module's objects files
#
##

MODULE_NAME := control

SOURCES := utils/Parser.c \
           $(MODULE_NAME)/Control.c \
           $(MODULE_NAME)/handlers/*.c \
           $(MODULE_NAME)/controllers/*.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))
