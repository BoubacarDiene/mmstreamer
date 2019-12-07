##
#
# \file graphics.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Create graphics module's objects files
#
##

MODULE_NAME := graphics

SOURCES = $(MODULE_NAME)/Graphics.c \
          $(MODULE_NAME)/drawers/Drawer$(SDL_BUILD_VERSION).c \
          $(MODULE_NAME)/fbdev/*.c

#################################################################
#                             Include                           #
#################################################################

include build/common.mk
$(eval $(call declare-common-rules))
