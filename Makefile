CFLAGS += -g -fsanitize=address 
CFLAGS += -Iinclude
# CFLAGS += -Iexternal/taglib/build/include -Lexternal/taglib/build/lib
LDFLAGS += -lavformat -lavcodec -lavutil -lswresample -lavfilter -lm -lsqlite3 -ltag_c -ltag -lstdc++ -lz

SRC_DIR = src
BUILD_DIR = build

SRCS := $(wildcard $(SRC_DIR)/**/*.c)
SRCS := $(filter-out main.c, $(SRCS))

OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
LIBNAME = $(BUILD_DIR)/libcore.a
MAIN = $(BUILD_DIR)/main

all: $(LIBNAME) $(MAIN) 

# Libraries ===> MINIAUDIO <===
MINIAUDIO_DIR = external/miniaudio
MINIAUDIO_REPO = https://github.com/mackron/miniaudio.git

TAGLIB_DIR = external/taglib
TAGLIB_REPO = https://github.com/taglib/taglib.git

external: $(MINIAUDIO_DIR) $(TAGLIB_DIR)

$(MINIAUDIO_DIR): 
	# Use to get updated MINIAUDIO.c and MINIAUDIO.h
	# Must move to source manually 
	mkdir -p $(MINIAUDIO_DIR)
	git clone $(MINIAUDIO_REPO) $(MINIAUDIO_DIR)

$(TAGLIB_DIR):
	mkdir -p $(TAGLIB_DIR)
	git clone $(TAGLIB_REPO) $(TAGLIB_DIR)	

# LIBRARY ===> TAGLIB <===

# Libraries 

$(LIBNAME): $(OBJS)
	mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c 
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

$(MAIN): $(SRC_DIR)/main.c $(LIBNAME)
	mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $< $(CFLAGS) -I $(SRC_DIR)/include -L $(BUILD_DIR) -lcore $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)  
