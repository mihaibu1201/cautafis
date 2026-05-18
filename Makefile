CFLAGS = -O2 -Wall -std=c23 -Iinclude
LDFLAGS = -lrgxfis
CC = cc
DBG = gdb
BUILD_DIR = build
TARGET_DIR = /usr/local/bin
SRC_DIR = src
EXE = cautafis
TARGET = $(BUILD_DIR)/$(EXE)
HELPFILE = cautafis_hlp.txt
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
ARGS = -c /home/mihai/3 -n 1_1

#Linkare
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)
	cp $(HELPFILE) $(BUILD_DIR)

#Compilare + generare dependințe
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

#Include dependințe (headere)
-include $(DEPS)

#Target principal
all: $(TARGET)

#Curățare
clean:
	rm -rf $(BUILD_DIR)

#Rulare
run:	$(TARGET) $(ARGS)
	./$(TARGET) $(ARGS)

#Debug
debug_rulare:	$(TARGET)
	$(DBG) -tui --args $(TARGET) $(ARGS)

# Cu simboluri pt debug
debug:	CFLAGS += -g
debug:	clean all

.PHONY: all clean run debug

install:
	cp $(TARGET) $(TARGET_DIR)
	cp $(HELPFILE) $(TARGET_DIR)
uninstall:
	rm $(TARGET_DIR)/$(EXE)
	rm $(TARGET_DIR)/$(HELPFILE)
