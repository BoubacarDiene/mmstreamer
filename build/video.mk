##
#
# \file video.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create video module's objects files
#
##

MODULE_NAME := video

SOURCES := utils/List.c utils/Task.c $(MODULE_NAME)/*.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))
