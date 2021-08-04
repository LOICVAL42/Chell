DEBUG = no
GRAMMAR_NAZI = no

CC = gcc
CCFLAGS = -std=gnu17 -Wall -Wextra -Wconversion
LDFLAGS =
INCLUDES = -I. -I/usr/include -Ilibs/scplib
LIBS = -L/usr/lib
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXEC = Chell

ifeq ($(DEBUG),yes)
	CCFLAGS += -O0
else
	CCFLAGS += -O2
endif

ifeq ($(GRAMMAR_NAZI),yes)
	CCFLAGS += -Werror
else
	CCFLAGS +=
endif

empty =
space = $(empty) $(empty)
tab = $(empty)	$(empty)
comma = ,
define newline

$(empty)
endef

all: $(EXEC)

$(EXEC): $(SRC)
	@$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(CCFLAGS) $(INCLUDES)

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(EXEC)

run: $(EXEC)
	@./$(EXEC)

install:
	@cp $(EXEC) /usr/bin/

.PHONY: all clear mrproper run install