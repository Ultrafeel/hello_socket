#.SUFFIXES:
#.SUFFIXES: .a .c .o
CC=gcc
CFLAGS= -D_GNU_SOURCE -D__USE_POSIX2 -D__USE_POSIX -I/usr/include -c -Wall -g
LDFLAGS=
HEADERS=common.h 
  
SOURCES=server.c client.c
OBJDIR:=objdir
SERVER_NAME:=server
OUT_TARG_DIR := target_bin/bin
OBJECTS=$(OBJDIR)/server.o $(OBJDIR)/client.o

# $(OBJDIR)/client.o

RM := rm

# Служебный целевой модуль.
.PHONY: all main clean 

all:  main
main: $(OUT_TARG_DIR)/$(SERVER_NAME) $(OUT_TARG_DIR)/client


$(OUT_TARG_DIR)/$(SERVER_NAME): $(OBJDIR)/server.o | $(OUT_TARG_DIR) 
	$(CC) $< -lpthread $(LDFLAGS)  -o $@
	
$(OUT_TARG_DIR)/client: $(OBJDIR)/client.o  | $(OUT_TARG_DIR) 
	$(CC) $< -lpthread $(LDFLAGS)  -o $@
	

$(OBJDIR)/%.o:%.c  $(HEADERS)| $(OBJDIR)
	$(CC) -I. $(CFLAGS) -c -o $@ $<


$(OBJDIR):
	mkdir $(OBJDIR)

$(OUT_TARG_DIR):
	mkdir -p $(OUT_TARG_DIR)

clean:
	@echo clean : $(OBJS)  $(SERVER_NAME) $(LIBS1)
	-$(RM)  -rfv   $(OBJS) ./$(OUT_TARG_DIR)/client ./$(OUT_TARG_DIR)/$(SERVER_NAME) $(SERVER_NAME).o $(SERVER_NAME)  $(LIBS1) $(LIBS_FILENAMES) $(LIBS_OB) $(LIBSRCH:.h=.a) $(LIBSRCH:.h=.so)
	-$(RM)  -rfv   ./$(OBJDIR)/*.o
