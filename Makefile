CFLAGS += -g -fsanitize=address
CFLAGS += -Iinclude
LDFLAGS += -lavformat -lavcodec -lavutil -lswresample -lavfilter -lm

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

CFLAGS += -I $(MINIAUDIO_DIR)

$(MINIAUDIO_DIR):
	git clone $(MINIAUDIO_REPO) $(MINIAUDIO_DIR)
# Libraries 

$(LIBNAME): $(OBJS)
	mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(MINIAUDIO_DIR)
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) $(LDFLAGS)

$(MAIN): $(SRC_DIR)/main.c $(LIBNAME)
	mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $< $(CFLAGS) -I $(SRC_DIR)/include -L $(BUILD_DIR) -lcore $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) external
