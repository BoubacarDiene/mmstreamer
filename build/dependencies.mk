##
#
# \file dependencies.mk
#
# \author Boubacar DIENE <boubacar.diene@gmail.com>
# \date   2016 - 2019
#
# \brief  Build dependencies:
#         - libexpat
#         - libfreetype
#         - libSDL
#         - libSDL_image
#         - lSDL_ttf
#
##

MODULE_NAME := deps

#################################################################
#                             Include                           #
#################################################################

include build/common.mk

#################################################################
#                            Variables                          #
#################################################################

# Versions
EXPAT_LIB_VERSION    := 2.2.0
FREETYPE_LIB_VERSION := 2.4.0
JPEG_LIB_VERSION     := 8d

ifeq ($(SDL_BUILD_VERSION),2)
    SDL_MAIN_VERSION  := 2.0.8
    SDL_IMAGE_VERSION := 2.0.3
    SDL_TTF_VERSION   := 2.0.14
else
    SDL_MAIN_VERSION  := 1.2.15
    SDL_IMAGE_VERSION := 1.2.12
    SDL_TTF_VERSION   := 2.0.11
endif

# Path
EXPAT_SUB_DIR := expat
EXPAT_PATH    := $(OUT_BUILD_DIR)/$(EXPAT_SUB_DIR)
EXPAT_SRC     := expat-$(EXPAT_LIB_VERSION)

FREETYPE_SUB_DIR := freetype
FREETYPE_PATH    := $(OUT_BUILD_DIR)/$(FREETYPE_SUB_DIR)
FREETYPE_SRC     := freetype-$(FREETYPE_LIB_VERSION)

SDL_SUB_DIR := sdl/v$(SDL_BUILD_VERSION)
SDL_PATH    := $(OUT_BUILD_DIR)/$(SDL_SUB_DIR)

ifeq ($(SDL_BUILD_VERSION),2)
    SDL_MAIN_SRC  := SDL2-$(SDL_MAIN_VERSION)
    SDL_IMAGE_SRC := SDL2_image-$(SDL_IMAGE_VERSION)
    SDL_TTF_SRC   := SDL2_ttf-$(SDL_TTF_VERSION)
else
    SDL_MAIN_SRC  := SDL-$(SDL_MAIN_VERSION)
    SDL_IMAGE_SRC := SDL_image-$(SDL_IMAGE_VERSION)
    SDL_TTF_SRC   := SDL_ttf-$(SDL_TTF_VERSION)
endif

# Targets
EXPAT_TARGET    := expat
FREETYPE_TARGET := freetype
SDL_TARGET      := sdl

#################################################################
#                             Build                             #
#################################################################

# Override common rules
prepare:
	$(MKDIR) $(EXPAT_PATH) $(FREETYPE_PATH) $(SDL_PATH)

install:
	$(call copy-content,$(OUT_BUILD_DIR)/include,$(OUT_STAGING_INC))
	$(call copy-content,$(OUT_BUILD_DIR)/lib,$(OUT_STAGING_LIB))

all: prepare $(EXPAT_TARGET) $(FREETYPE_TARGET) $(SDL_TARGET)

clean:
	$(RM) $(EXPAT_PATH)/$(EXPAT_SRC).built       ||:
	$(RM) $(FREETYPE_PATH)/$(FREETYPE_SRC).built ||:
	$(RM) $(SDL_PATH)/$(SDL_MAIN_SRC).built      ||:
	$(RM) $(SDL_PATH)/$(SDL_IMAGE_SRC).built     ||:
	$(RM) $(SDL_PATH)/$(SDL_TTF_SRC).built       ||:

# expat
$(EXPAT_TARGET):
	$(PRINT) ***** Building $(EXPAT_SRC) *****
	if [ ! -f $(EXPAT_PATH)/$(EXPAT_SRC).built ]; then \
		tar xjvf $(DEPS)/$@/$(EXPAT_SRC).tar.bz2 -C $(EXPAT_PATH); \
		cd $(EXPAT_PATH)/$(EXPAT_SRC) \
		&& ./configure --prefix=$(OUT_BUILD_DIR) \
		&& make && make install \
		&& cd -; \
		touch $(EXPAT_PATH)/$(EXPAT_SRC).built; \
	fi

# freetype
$(FREETYPE_TARGET):
	$(PRINT) ***** Building $(FREETYPE_SRC) *****
	if [ ! -f $(FREETYPE_PATH)/$(FREETYPE_SRC).built ]; then \
		tar xvf $(DEPS)/$@/$(FREETYPE_SRC).tar.gz -C $(FREETYPE_PATH); \
		cd $(FREETYPE_PATH)/$(FREETYPE_SRC) \
		&& ./configure --prefix=$(OUT_BUILD_DIR) \
		&& make && make install \
		&& cd -; \
		touch $(FREETYPE_PATH)/$(FREETYPE_SRC).built; \
	fi

# sdl
$(SDL_TARGET)-main:
	$(PRINT) ***** Building $(SDL_MAIN_SRC) *****
	if [ ! -f $(SDL_PATH)/$(SDL_MAIN_SRC).built ]; then \
	    tar xvf $(DEPS)/$(SDL_SUB_DIR)/$(SDL_MAIN_SRC).tar.gz -C $(SDL_PATH); \
	    cd $(SDL_PATH)/$(SDL_MAIN_SRC) \
	    && sed -e '/_XData32/s:register long:register _Xconst long:' -i src/video/x11/SDL_x11sym.h ||: \
	    && ./configure --prefix=$(OUT_BUILD_DIR) \
	    && make && make install \
	    && cd -; \
	    touch $(SDL_PATH)/$(SDL_MAIN_SRC).built; \
	fi

$(SDL_TARGET)-image:
	$(PRINT) ***** Building $(SDL_IMAGE_SRC) *****
	if [ ! -f $(SDL_PATH)/$(SDL_IMAGE_SRC).built ]; then \
	    tar xvf $(DEPS)/$(SDL_SUB_DIR)/$(SDL_IMAGE_SRC).tar.gz -C $(SDL_PATH); \
	    cd $(SDL_PATH)/$(SDL_IMAGE_SRC) \
	    && ./configure --prefix=$(OUT_BUILD_DIR) --with-sdl-prefix=$(OUT_BUILD_DIR) \
	    && make && make install \
	    && cd -; \
	    touch $(SDL_PATH)/$(SDL_IMAGE_SRC).built; \
	fi

$(SDL_TARGET)-ttf:
	$(PRINT) ***** Building $(SDL_TTF_SRC) *****
	if [ ! -f $(SDL_PATH)/$(SDL_TTF_SRC).built ]; then \
	    tar xvf $(DEPS)/$(SDL_SUB_DIR)/$(SDL_TTF_SRC).tar.gz -C $(SDL_PATH); \
	    cd $(SDL_PATH)/$(SDL_TTF_SRC) \
	    && ./configure --prefix=$(OUT_BUILD_DIR) --with-freetype-prefix=$(OUT_BUILD_DIR) --with-sdl-prefix=$(OUT_BUILD_DIR) \
	    && make && make install \
	    && cd -; \
	    touch $(SDL_PATH)/$(SDL_TTF_SRC).built; \
	fi

$(SDL_TARGET): $(SDL_TARGET)-main $(SDL_TARGET)-image $(SDL_TARGET)-ttf
