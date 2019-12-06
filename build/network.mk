##
#
# \file network.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create network module's objects files
#
##

MODULE_NAME := network

SOURCES := utils/List.c utils/Task.c $(MODULE_NAME)/*.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))
