TARGET=imp

TOOLCHAIN_PREFIX ?= mipsel-ps2-irx-
CC := $(TOOLCHAIN_PREFIX)gcc
CXX := $(TOOLCHAIN_PREFIX)g++
AS := $(TOOLCHAIN_PREFIX)as

SRCDIRS := .
OBJDIR := build

CXXSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
CSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

CFLAGS := -D_IOP -D_PS2SDK -std=gnu99 -G0 -Og -g -fno-common -fno-builtin -Wall -Werror-implicit-function-declaration -msoft-float -mno-explicit-relocs
LDFLAGS := -L$(PS2SDK)/iop/lib -nostdlib

LIBS=

OBJ := $(addprefix $(OBJDIR)/,$(CSRC:.c=.o) $(CXXSRC:.cpp=.o))
DEP := $(addprefix $(OBJDIR)/,$(CSRC:.c=.d) $(CXXSRC:.cpp=.d))

ASINC := $(addprefix -I,$(SRCDIRS))
INC := $(addprefix -I,$(SRCDIRS)) -I$(PS2SDKSRC)/iop/kernel/include -I$(PS2SDKSRC)/common/include

$(TARGET).irx: $(OBJ)
	echo $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LIBS) $(LDFLAGS)

clean:
	rm -rf $(OBJ) $(TARGET) $(DEP)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@

$(OBJDIR)/%.d: %.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $(INC) $< > $@
$(OBJDIR)/%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MT $(@:.d=.o) $(CFLAGS) $(INC) $< > $@

-include $(DEP)
