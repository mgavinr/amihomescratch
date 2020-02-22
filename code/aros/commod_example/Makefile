#====================#
# Definitions
#====================#
include Makefile.opts
SRCS    = Gavin-Blanker.c
HDRS    = 
NAME    = Gavin-Blanker
COBJS_DIR = ./cobjs
OBJS_DIR = ./objs
# cpp c files
OBJS1   = $(SRCS:%.cpp=$(OBJS_DIR)/%.o)
OBJS   = $(OBJS1:%.c=$(OBJS_DIR)/%.o)
# i files
OBJSE1   = $(SRCS:%.cpp=$(OBJS_DIR)/%.i)
BINDIR  = ./bin
BIN 	= $(BINDIR)/$(NAME)
INSTALL = bin/$(NAME).tar
LIB     = ./objs/libsendsctp_cpp.a
CLIB     = ./objs/libsendsctp_c.a
BDATE   = `date | sed 's/ /-/g' | sed 's/:/-/g'`

#====================#
# Make options
#====================#
all:    $(BIN) $(LIB) $(INSTALL)

clean:  
	@echo "*** Deleting $(BIN) $(OBJS) ..."
	@-$(RM) $(BIN) $(OBJS)
	@-$(RM) $(INSTALL)
	@echo " "

#====================#
# Make commands
#====================#
$(BIN): $(OBJSE1) $(OBJS)
	@echo "*** Creating $@:"
	$(CPP) $(OBJS) $(LD_FLAGS) -o $@
	date >> bin/buildsize.txt
	ls -l $@ >> bin/buildsize.txt
	@echo "valgrind --tool=memcheck --leak-check=full ${BIN}"
	@echo " "

$(INSTALL):
	@echo "*** Creating $@:"
	rm -f $@
	tar -cvf $@ `find . -type f | grep -v tar | grep -v \.o` | wc -l
	ls -l $@
	@echo " "

install:
	tar -hcvf bin/build$(BDATE)_$(NAME).tar bin/$(NAME) | wc -l
	tar -hcvf bin/build_$(NAME).tar bin/$(NAME) | wc -l
	rm -rf bin/out
	mkdir bin/out
	cd bin/out && tar -xvf ../build_$(NAME).tar && cd -
	gzip bin/build$(BDATE)_$(NAME).tar
	@echo " "

#====================#
# Make commands
#====================#
$(OBJS):    $(HDRS)

$(LIB): $(OBJS)
	@echo "Creating $@:"
	$(AR) $@ $(OBJS)
	@echo " "

$(OBJS_DIR)/%.o:    $*%.cpp
	@echo "*** Creating $@:"
	$(CPP) $(CPP_FLAGS) $*.cpp -o $@
	@echo " "

$(OBJS_DIR)/%.i:    $*%.cpp
	@echo "*** Creating $@:"
	$(CPP) $(CPP_FLAGS) -E $*.cpp -o $@
	@echo " "

$(OBJS_DIR)/%.o:    $*%.c
	@echo "*** Creating $@:"
	$(CPP) $(CPP_FLAGS) $*.c -o $@
	@echo " "
