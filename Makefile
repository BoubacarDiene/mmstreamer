##
#
# \file Makefile
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Root makefile to generate mmstreamer binary based on
#         sub-makefiles
#
##

MODULE_NAME := app

#################################################################
#                            Include                            #
#################################################################

include build/common.mk

#################################################################
#                            Variables                          #
#################################################################

# Binary
BIN_SUFFIX := $(if $(subst no,,$(DEBUG)),.dbg,)
BIN_NAME   := $(PROJECT_NAME)-$(PROJECT_VERSION)$(BIN_SUFFIX)

BIN_SDL     := SDL$(subst 1,,$(SDL_BUILD_VERSION))
BIN_LDFLAGS := -L$(OUT_STAGING_LIB) -Wl,-rpath,$(OUT_STAGING_LIB) \
               -l$(BIN_SDL) -l$(BIN_SDL)_image -l$(BIN_SDL)_ttf \
               -lfreetype -lpng -ljpeg -lz -lexpat

# Path
OUT_RELEASE     := $(OUT)/$(PROJECT_NAME)
OUT_RELEASE_BIN := $(OUT_RELEASE)/bin
OUT_RELEASE_LIB := $(OUT_RELEASE)/lib
OUT_RELEASE_RES := $(OUT_RELEASE)/res

WORKDIR := $(dir $(OUT))
export BIN_RELATIVE_DIR := $(subst $(WORKDIR),./,$(OUT_RELEASE_BIN))
export LIB_RELATIVE_DIR := $(subst $(WORKDIR),./,$(OUT_RELEASE_LIB))
export RES_RELATIVE_DIR := $(subst $(WORKDIR),./,$(OUT_RELEASE_RES))

#################################################################
#                             Build                             #
#################################################################

#
# Function to run specified target on all makefiles
# $1: target to run
#
MK_FILES := dependencies.mk main.mk core.mk control.mk \
            video.mk network.mk graphics.mk
define make-target
	@- $(foreach makefile,$(MK_FILES), \
		$(MAKE) -f build/$(makefile) $1 ; \
	)
endef

#
# Rules
#
all: $(BIN_NAME)

prepare:
	$(call make-target,all install)
	$(MKDIR) $(OUT_BUILD_DIR)

$(BIN_NAME): LDFLAGS += $(BIN_LDFLAGS)
$(BIN_NAME): prepare
	$(CC) $(CFLAGS) \
		-o $(OUT_BUILD_DIR)/$@ \
		$(shell find $(OUT_STAGING_OBJS) -name *.o) \
		$(LDFLAGS)

install:
	$(call copy-content,$(OUT_BUILD_DIR),$(OUT_RELEASE_BIN))
	$(call copy-content,$(OUT_STAGING_LIB),$(OUT_RELEASE_LIB))
	$(call copy-content,$(RES),$(OUT_RELEASE_RES))
	
	$(PRINT) "\n---------------------------------------------"
	$(PRINT) " SUCCESS! As root, run $(BIN_NAME) using: "
	$(PRINT) "---------------------------------------------"
	$(PRINT) " export LD_LIBRARY_PATH=$(LIB_RELATIVE_DIR)"
	$(PRINT) " $(BIN_RELATIVE_DIR)/$(BIN_NAME) -f $(RES_RELATIVE_DIR)/Main.xml\n"

clean:
	$(call make-target,clean)
	$(RM) $(OUT_BUILD_DIR) ||:
	$(RM) $(OUT_RELEASE) ||:

.PHONY: mrproper
mrproper:
	$(RM) $(OUT)
