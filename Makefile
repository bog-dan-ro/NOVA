#
# Makefile
#
# Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
# Economic rights: Technische Universitaet Dresden (Germany)
#
# Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
# Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
#
# This file is part of the NOVA microhypervisor.
#
# NOVA is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# NOVA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License version 2 for more details.
#

-include Makefile.conf

# Defaults
ARCH	?= x86_64

# Configuration
ifneq ("$(wildcard conf/archs/$(ARCH).conf)","")
include conf/archs/$(ARCH).conf
else
$(error conf/archs/$(ARCH).conf is not a valid architecture config)
endif

ifneq ("$(wildcard conf/archs/common.conf)","")
include conf/archs/common.conf
else
$(error conf/archs/common.conf is missing)
endif

# Tools
INSTALL	?= install -m 644
MKDIR	?= mkdir -p
ifeq ($(COMP),gcc)
HST_CC	?= g++
TGT_CC	:= $(PREFIX)g++
TGT_LD	:= $(PREFIX)ld
TGT_OC	:= $(PREFIX)objcopy
TGT_SZ	:= $(PREFIX)size
else
$(error $(COMP) is not a valid compiler type)
endif

# In-place editing works differently between GNU/BSD sed
SEDI	:= $(shell if sed --version 2>/dev/null | grep -q GNU; then echo "sed -i"; else echo "sed -i ''"; fi)

# Directories
CMD_DIR	:= cmd
SRC_DIR	:= src/$(ARCH) src
INC_DIR	:= inc/$(ARCH) inc
BLD_DIR	?= build/$(ARCH)

# Configure board feature set
ifneq ("$(wildcard conf/boards/$(ARCH)/$(BOARD).conf)","")
-include conf/boards/$(ARCH)/$(BOARD).conf
$(foreach feature,$(FEATURES), \
	$(eval INC_DIR += inc/$(feature)) \
	$(eval INC_DIR += inc/$(ARCH)/$(feature)) \
	$(eval SRC_DIR += src/generic/$(feature)) \
	$(eval SRC_DIR += src/$(ARCH)/$(feature)) \
	$(eval DEFINES += FEATURE_$(feature)))
else
$(error conf/boards/$(ARCH)/$(BOARD).conf is not a valid board type)
endif


# Patterns
OBJ_DIR	?= $(BLD_DIR)/obj
PAT_CMD	:= $(BLD_DIR)/%
PAT_OBJ	:= $(OBJ_DIR)/%.o

# Files
MFL	:= $(MAKEFILE_LIST)
SRC	:= hypervisor.ld $(sort $(notdir $(foreach d,$(SRC_DIR),$(wildcard $(d)/*.S)))) $(sort $(notdir $(foreach d,$(SRC_DIR),$(wildcard $(d)/*.cpp))))
OBJ	:= $(patsubst %.ld,$(PAT_OBJ), $(patsubst %.S,$(PAT_OBJ), $(patsubst %.cpp,$(PAT_OBJ), $(SRC))))
OBJ_DEP	:= $(OBJ:%.o=%.d)

DIG	:= $(BLD_DIR)/digest
HYP	:= $(HYP_NAMING)
DBG	:= $(HYP).debug
MAP	:= $(HYP).map
ELF	:= $(HYP).elf
BIN	:= $(HYP).bin

# Messages
ifneq ($(findstring s,$(MAKEFLAGS)),)
message = @echo $(1) $(2)
endif

# Tool check
tools = $(if $(shell command -v $($(1)) 2>/dev/null),, $(error Missing $(1)=$($(1)) *** Configure it in Makefile.conf (see Makefile.conf.example)))

# Feature check
check = $(shell if $(TGT_CC) $(1) -Werror -c -xc++ /dev/null -o /dev/null >/dev/null 2>&1; then echo "$(1)"; fi)

# Version check
gitrv = $(shell (git rev-parse HEAD 2>/dev/null || echo 0) | cut -c1-7)

# Search path
VPATH	:= $(SRC_DIR)

# Optimization options
DFLAGS	:= -MP -MMD -pipe
OFLAGS	:= -g -Os

# Preprocessor options
DEFINES	+= $(ARCH_DEFINES)
PFLAGS	:= $(addprefix -D, $(DEFINES))
PFLAGS	+= $(addprefix -I, $(INC_DIR))

# Language options
FFLAGS	:= $(or $(call check,-std=gnu++26), $(call check,-std=gnu++23))
FFLAGS	+= -ffreestanding -fdata-sections -ffunction-sections -fdiagnostics-color=auto -fno-asynchronous-unwind-tables -fno-exceptions -fno-pic -fno-rtti -fno-stack-protector -fno-use-cxa-atexit -fomit-frame-pointer
FFLAGS	+= $(call check,-fcf-protection=$(CFP))
# Language options added in gcc-12
FFLAGS	+= $(call check,-ftrivial-auto-var-init=uninitialized)

# Warning options
WFLAGS	:= -Wall -Wextra -Walloca -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wdisabled-optimization -Wduplicated-branches -Wduplicated-cond -Wenum-conversion -Wextra-semi -Wformat=2 -Wlogical-op -Wmismatched-tags -Wmissing-format-attribute -Wmissing-noreturn -Wmultichar -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wredundant-tags -Wregister -Wshadow -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wsuggest-override -Wvirtual-inheritance -Wvolatile -Wvolatile-register-var -Wwrite-strings -Wzero-as-null-pointer-constant
# Warning options added in gcc-12
WFLAGS	+= $(call check,-Wbidi-chars=any)
# Warning options added in gcc-14
WFLAGS	+= $(call check,-Wnrvo)
# Warning options added in gcc-15
WFLAGS	+= $(call check,-Wleading-whitespace=spaces)
WFLAGS	+= $(call check,-Wtrailing-whitespace=any)
WFLAGS	+= $(ARCH_WFLAGS)

# Compiler flags
CFLAGS	:= $(PFLAGS) $(DFLAGS) $(MFLAGS) $(FFLAGS) $(OFLAGS) $(WFLAGS)

# Linker flags
LFLAGS	:= --defsym=GIT_VER=0x$(call gitrv) --gc-sections --warn-common -Map=$(MAP) -static -n -T

# Rules
$(HYP):			$(OBJ)
			$(call message,LNK,$@)
			$(TGT_LD) $(LFLAGS) $^ $(ARCH_LFLAGS) -o $@
			$(call message,DBG,$(DBG))
			$(TGT_OC) --only-keep-debug $@ $(DBG)
			$(TGT_OC) --strip-all $@

$(ELF):			$(HYP)
			$(call message,ELF,$@)
			$(H2E) $< $@

$(BIN):			$(HYP)
			$(call message,BIN,$@)
			$(H2B) $< $@

$(PAT_OBJ):		%.ld
			$(call message,PRE,$@)
			$(TGT_CC) $(CFLAGS) -xassembler-with-cpp -E -P -MT $@ $< -o $@
			@$(SEDI) 's|$<|$(notdir $<)|' $(@:%.o=%.d)

$(PAT_OBJ):		%.S
			$(call message,ASM,$@)
			$(TGT_CC) $(CFLAGS) -c $< -o $@
			@$(SEDI) 's|$<|$(notdir $<)|' $(@:%.o=%.d)

$(PAT_OBJ):		%.cpp
			$(call message,CXX,$@)
			$(TGT_CC) $(CFLAGS) -c $< -o $@
			@$(SEDI) 's|$<|$(notdir $<)|' $(@:%.o=%.d)

$(PAT_CMD):		$(CMD_DIR)/%.cpp
			$(call message,CMD,$@)
			$(HST_CC) $< -o $@

$(BLD_DIR):
			$(call message,DIR,$@)
			@$(MKDIR) $@

$(OBJ_DIR):
			$(call message,OBJ,$@)
			@$(MKDIR) $@

Makefile.conf:
			$(call message,CFG,$@)
			@cp $@.example $@

$(DIG):			$(MFL) | $(BLD_DIR) tool_hst_cc
$(OBJ):			$(MFL) | $(OBJ_DIR) tool_tgt_cc

# Zap old-fashioned suffixes
.SUFFIXES:

.PHONY:			clean install run tool_hst_cc tool_tgt_cc

clean:
			$(call message,CLN,$@)
			$(RM) $(DIG) $(OBJ) $(HYP) $(DBG) $(MAP) $(ELF) $(BIN) $(OBJ_DEP)

install:		$(foreach d,$(INS_DIR),install-to-$(subst :,@,$(d))) | $(DIG)
			@echo "Section Sizes for $(HYP)"
			@$(TGT_SZ) $(HYP)
ifeq ($(INSTALL_INTEGRITY),yes)
			@echo "Reference Integrity Measurements for $(HYP)"
			@echo $(shell $(DIG) $(HYP) | sha1sum)   "SHA1-160"
			@echo $(shell $(DIG) $(HYP) | sha256sum) "SHA2-256"
			@echo $(shell $(DIG) $(HYP) | sha384sum) "SHA2-384"
			@echo $(shell $(DIG) $(HYP) | sha512sum) "SHA2-512"
endif

run:			$(ELF)
			$(RUN) -kernel $<

debug:			$(ELF)
			$(RUN) -s -S -kernel $<

tool_hst_cc:
			$(call tools,HST_CC)

tool_tgt_cc:
			$(call tools,TGT_CC)

# Create a rule for each install directory
define INSTALL_RULE =
install-to-$(subst :,@,$(2)): $(1)
			$(call message,INS,$(1) =\> $(2))
			$(INSTALL) $(1) $(2)
endef
$(foreach d,$(INS_DIR),$(eval $(call INSTALL_RULE,$(HYP),$(d))))

# Include Dependencies
ifneq ($(MAKECMDGOALS),clean)
-include		$(OBJ_DEP)
endif
