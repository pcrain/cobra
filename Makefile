RM := rm -rf
MKDIR_P = mkdir -p

NOWARN := -Wno-unused-variable -Wno-format-truncation

OBJS += \
build/cobra.o \
build/alarm.o \
build/notifier.o \
build/main.o

CPP_DEPS += \
build/cobra.d \
build/alarm.d \
build/notifier.d \
build/main.d

LIBS := -ljsoncpp -lncurses -lnotify -lgtk-3 -lglib-2.0 -lgobject-2.0 -ltinfo

INCLUDES = \
	-I/usr/include/gtk-3.0 \
	-I/usr/include/pango-1.0 \
	-I/usr/include/cairo/ \
	-I/usr/include/atk-1.0 \
	-I/usr/include/jsoncpp \
	-I/usr/include/glib-2.0/ \
	-I/usr/include/gdk-pixbuf-2.0 \
	-I/usr/lib/glib-2.0/include/

BUILD_DIR = build

all: directories cobra

cobra: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L/usr/lib -std=c++11 -fopenmp -o "./cobra" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

build/%.o: ./src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ $(INCLUDES) -O3 -g3 -Wall -c -fmessage-length=0 -std=c++11 $(NOWARN) -fopenmp -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean:
	-$(RM) $(OBJS)$(C++_DEPS) ./cobra
	-@echo ' '

directories: ${BUILD_DIR}

${BUILD_DIR}:
	${MKDIR_P} ${BUILD_DIR}

.PHONY: all clean directories
.SECONDARY:
