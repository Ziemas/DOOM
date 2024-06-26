TARGET=doom

TOOLCHAIN_PREFIX ?= mips64r5900el-ps2-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
CXX := $(TOOLCHAIN_PREFIX)g++
AS := $(TOOLCHAIN_PREFIX)as

SRCDIRS := . ps2
OBJDIR := build

CXXSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
CSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
VUSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.dsm))

CFLAGS := -D_EE -std=gnu99 -G0 -Og -g -fno-common -Wall -Werror-implicit-function-declaration

LIBS=

OBJ := $(addprefix $(OBJDIR)/,$(CSRC:.c=.o) $(CXXSRC:.cpp=.o) $(VUSRC:.dsm=.o))
DEP := $(addprefix $(OBJDIR)/,$(CSRC:.c=.d) $(CXXSRC:.cpp=.d))

ASINC := $(addprefix -I,$(SRCDIRS))
INC := $(addprefix -I,$(SRCDIRS))	\
	-I$(PS2SDK)/common/include	    \
	-I$(PS2SDK)/ee/include

$(TARGET).elf: $(OBJ)
	echo $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LIBS) -T $(PS2SDK)/ee/startup/linkfile -L$(PS2SDK)/ee/lib

clean:
	rm -rf $(OBJ) $(TARGET) $(DEP)

$(OBJDIR)/%.o: $(OBJDIR)/%.dsm_x
	@mkdir -p $(@D)
	dvp-as -alm=$(@:.o=.lst) $(ASINC) $< -o $@

$(OBJDIR)/%.dsm_x: %.dsm
	@mkdir -p $(@D)
	sh joinvu $< | cpp $(ASINC) | grep -v '^#' > $@

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
