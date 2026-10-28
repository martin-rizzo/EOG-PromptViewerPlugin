# File    : Makefile
# Brief   : Compiles, installs, and uninstalls the SD Prompt Viewer plugin
# Author  : Martin Rizzo | <martinrizzo@gmail.com>
# Date    : Apr 25, 2023
# Repo    : https://github.com/martin-rizzo/SDPromptViewer
# License : MIT
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#                       Prompt Viewer Plugin for EOG
#      A plugin for "Eye of GNOME" that displays SD embedded prompts.
#   
#     Copyright (c) 2023-2026 Martin Rizzo
#     
#     Permission is hereby granted, free of charge, to any person obtaining
#     a copy of this software and associated documentation files (the
#     "Software"), to deal in the Software without restriction, including
#     without limitation the rights to use, copy, modify, merge, publish,
#     distribute, sublicense, and/or sell copies of the Software, and to
#     permit persons to whom the Software is furnished to do so, subject to
#     the following conditions:
#     
#     The above copyright notice and this permission notice shall be
#     included in all copies or substantial portions of the Software.
#     
#     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#     EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#     TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE
#     SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _

# Default build type
BUILD_TYPE ?= release

# Definicion de colores ANSI
RESET      := \033[0m
MSGINFO    := \033[1;32m[PvP-INFO] \033[1;36m
MSGERROR   := \033[1;31m[PvP-ERR] \033[1;33m
MSGSUCCESS := \033[1;32m

#============================================================================
# ???
#============================================================================
PROJECT_NAME    := prompt-viewer
GSCHEMA_NAME    := org.gnome.eog.plugins.$(PROJECT_NAME)
GETTEXT_PACKAGE := $(shell echo $(PROJECT_NAME) | tr '[:lower:]' '[:upper:]')

#============================================================================
# Directory Paths and Installation targets
#============================================================================
XDG_DATA_HOME   ?= $(HOME)/.local/share
EOG_PLUGINS_DIR := $(XDG_DATA_HOME)/eog/plugins
GIO_SCHEMAS_DIR := $(XDG_DATA_HOME)/glib-2.0/schemas
PODIR           := po
SRCDIR          := src
OBJDIR          := build/$(BUILD_TYPE)
BINDIR          := bin
ifeq ($(BUILD_TYPE),debug)
    BINDIR := bin/debug
endif

#============================================================================
# External Dependencies & Sub-projects
#============================================================================
GLIB_CFLAGS     := $(shell pkg-config --cflags glib-2.0)
GTK_CFLAGS      := $(shell pkg-config --cflags gtk+-3.0)
LIBPEAS_CFLAGS  := $(shell pkg-config --cflags libpeas-gtk-1.0)
EOG_CFLAGS      := $(shell pkg-config --cflags eog)

# Sub-project: PromptExtractor
PROMPTEXT_DIR    := external/PromptExtractor
PROMPTEXT_LIB    := $(PROMPTEXT_DIR)/lib/libpromptex.a
PROMPTEXT_CFLAGS := -I$(PROMPTEXT_DIR)/include
ifeq ($(BUILD_TYPE),debug)
    # le agrega el sufijo 'd' a la version debug
    PROMPTEXT_LIB := $(PROMPTEXT_DIR)/lib/libpromptexd.a
endif


#============================================================================
# Compiler Options and Flags
#============================================================================

# Flags for automatic dependency generation (.d files)
DEPFLAGS := -MMD -MP

# Compiler and linker settings
CC      := gcc
CFLAGS  := -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -fPIC \
           -DGETTEXT_PACKAGE='"$(GETTEXT_PACKAGE)"' \
           $(GLIB_CFLAGS) $(GTK_CFLAGS) $(LIBPEAS_CFLAGS) $(EOG_CFLAGS) $(PROMPTEXT_CFLAGS) \
           $(DEPFLAGS) $(EXTRA_CFLAGS)
LDFLAGS :=

# Build type specific flags
ifeq ($(BUILD_TYPE),debug)
    CFLAGS  += -g -O0 -DDEBUG
    LDFLAGS += 
else
    CFLAGS  += -Os -s -ffunction-sections -fdata-sections -DNDEBUG
    LDFLAGS += -Wl,--gc-sections
endif

#============================================================================
# Final Target Names & Configuration
#============================================================================
GSCHEMA_INPUT := system/sdprompt-viewer.gschema.xml.in
PLUGIN_INPUT  := system/sdprompt-viewer.plugin.desktop.in
RESOURCES_C   := $(OBJDIR)/$(PROJECT_NAME)-resources.c
RESOURCES_XML := resources.xml

PLUGIN_MODULE := $(BINDIR)/lib$(PROJECT_NAME).so
PLUGIN_DESC   := $(BINDIR)/lib$(PROJECT_NAME).plugin
GSCHEMA       := $(GSCHEMA_NAME).gschema.xml

#============================================================================
# Automatic Source and Object File Detection
#============================================================================

# List of main source files for the project
SRCS_C := $(SRCDIR)/sdprompt-viewer-plugin.c \
          $(SRCDIR)/sdprompt-viewer-preferences.c \
		  $(SRCDIR)/prompt-viewer-ui.c \
		  $(SRCDIR)/prompt-viewer-ui.resizer.c

# Object files derived from the main source files.
# The auto-generated 'resources.c' is also included,
# which is already located in the $(OBJDIR) directory.
OBJS_SRCS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS_C))
OBJ_RES   := $(RESOURCES_C:.c=.o)
OBJS      := $(OBJS_SRCS) $(OBJ_RES)

# Dependency files for automatic header inclusion tracking
ALL_DEPS := $(OBJS:.o=.d)

#============================================================================
# Compilation Rules (Main Rules)
#============================================================================

# The default target builds everything
all: $(PLUGIN_DESC) $(PLUGIN_MODULE) $(GSCHEMA)

# Rule to compile the sub-project invoking its own Makefile
$(PROMPTEXT_LIB):
	@echo -e "$(MSGINFO)Building sub-project PromptExtractor ($(BUILD_TYPE))$(RESET)"
	$(MAKE) -C $(PROMPTEXT_DIR) BUILD_TYPE=$(BUILD_TYPE) EXTRA_CFLAGS="-fPIC"

#============================================================================
# Intermediate and Auxiliary File Generation Rules
#============================================================================

# Generate resources source file
$(RESOURCES_C): $(RESOURCES_XML)
	@mkdir -p $(dir $@)
	@echo -e "$(MSGINFO)Generating resources source file$(RESET)"
	glib-compile-resources --target="$@" --generate-source $(RESOURCES_XML)

# Generate shared library (.so)
$(PLUGIN_MODULE): $(OBJS) $(PROMPTEXT_LIB)
	@mkdir -p $(dir $@)
	@echo -e "$(MSGINFO)Linking shared library: $@ ($(BUILD_TYPE))$(RESET)"
	$(CC) -shared $(LDFLAGS) -o $@ $(OBJS) $(PROMPTEXT_LIB)
	@echo -e "$(MSGINFO)Build successful! Library created at: $@$(RESET)"

# Generate GSchema XML file
$(GSCHEMA): $(GSCHEMA_INPUT)
	@echo -e "$(MSGINFO)Generating gschema file...$(RESET)"
	@sed -e 's/@GETTEXT_PACKAGE@/$(GETTEXT_PACKAGE)/g' $< > $@

# Generate Plugin desktop file
$(PLUGIN_DESC): $(PLUGIN_INPUT)
	@mkdir -p $(dir $@)
	@echo -e "$(MSGINFO)Generating plugin file...$(RESET)"
	@msgfmt --desktop --keyword=Name --keyword=Description \
	        --template $< -d $(PODIR) -o $@

# Pattern rule to compile objects (automatically creates subdirectories)
$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	@mkdir -p $(dir $@)
	@echo -e "$(MSGINFO)Compiling component: $< ($(BUILD_TYPE))$(RESET)"
	$(CC) $(CFLAGS) -c $< -o $@

#============================================================================
# Dynamic Directory Creation
#============================================================================
$(SRCDIR):
	@mkdir -p $@

#============================================================================
# Installation and System Integration
#============================================================================
install: $(PLUGIN_DESC) $(PLUGIN_MODULE) $(GSCHEMA)
	install -D $(PLUGIN_DESC)  $(EOG_PLUGINS_DIR)/$(PLUGIN_DESC)
	install -D $(PLUGIN_MODULE) $(EOG_PLUGINS_DIR)/$(PLUGIN_MODULE)
	install -D $(GSCHEMA) $(GIO_SCHEMAS_DIR)/$(GSCHEMA)
	glib-compile-schemas $(GIO_SCHEMAS_DIR)

remove:
	rm -f "$(EOG_PLUGINS_DIR)/$(PLUGIN_MODULE)"
	rm -f "$(EOG_PLUGINS_DIR)/$(PLUGIN_DESC)"
	rm -f "$(GIO_SCHEMAS_DIR)/$(GSCHEMA)"
	glib-compile-schemas "$(GIO_SCHEMAS_DIR)"

run:
	EOG_DEBUG_PLUGINS=true GOBJECT_DEBUG=instance-count eog

info:
	@echo "Makefile for building and installing the EOG plugin."
	@echo "  BUILD_TYPE      = $(BUILD_TYPE)"
	@echo "  EOG_PLUGINS_DIR = $(EOG_PLUGINS_DIR)"
	@echo "  GIO_SCHEMAS_DIR = $(GIO_SCHEMAS_DIR)"
	@echo "  RESOURCES_C     = $(RESOURCES_C)"
	@echo "  ALL_DEPS        = $(ALL_DEPS)"
#	@echo "  CFLAGS          = $(CFLAGS)"

#============================================================================
# Project Cleanup
#============================================================================
clean:
	@echo -e "$(MSGINFO)Cleaning up all compilation files...$(RESET)"
	rm -rf build $(BINDIR) $(GSCHEMA)
	$(MAKE) -C $(PROMPTEXT_DIR) clean
	@echo -e "$(MSGINFO)Cleanup completed.$(RESET)"

# Specific targets for debug and release builds
debug:
	@$(MAKE) all BUILD_TYPE=debug

release:
	@$(MAKE) all BUILD_TYPE=release

# Generates "compile_commands.json" automatically using bear
compile_commands.json: clean
	@echo -e "$(MSGINFO)Generating 'compile_commands.json'$(RESET)"
	@if command -v bear >/dev/null 2>&1; then \
		bear -- $(MAKE) debug; \
		echo -e "$(MSGINFO)Compilation database generated successfully.$(RESET)"; \
	else \
		echo -e "$(MSGERROR)The command 'bear' is not installed. Run 'sudo dnf install bear' first.$(RESET)"; \
		exit 1; \
	fi

# Prevents conflicts if files named like targets exist
.PHONY: all clean install remove run info debug release $(PROMPTEXT_LIB)

#============================================================================
# Include Dependencies (crucial for .h changes)
#============================================================================

# Listado de targets que NO necesitan cargar dependencias (.d)
NODEPS_TARGETS := clean remove info

ifeq ($(filter $(NODEPS_TARGETS),$(MAKECMDGOALS)),)
-include $(ALL_DEPS)
endif
