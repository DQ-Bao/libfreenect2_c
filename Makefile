OBJ := $(addsuffix .o,$(basename $(wildcard *.cpp)))
DEP := $(OBJ:%.o=%.d)

all: libfreenect2_c.a

libfreenect2_c.a: $(OBJ)
	ar rcs $@ $^

shared: libfreenect2_c.dll libfreenect2_c.dll.a

libfreenect2_c.dll: $(OBJ)
	g++ $^ -shared -Llib \
		-O3 -Os -s \
		-static \
		-static-libgcc \
		-static-libstdc++ \
		-Wl,--out-implib,libfreenect2_c.dll.a \
		-lfreenect2 \
		-lusb-1.0 \
		-lturbojpeg \
		-o $@

%.o: %.cpp
	g++ -Wall -Wextra -Wpedantic -std=c++17 -Iinclude -DLIBFREENECT2_STATIC_DEFINE -MMD -MP -c $< -o $@

examples: libfreenect2_c.a main.exe

main.exe: examples/main.c
	gcc -Wall -Wextra -Wpedantic \
		-O3 -Os -s \
		-I. -Iinclude \
		$< \
		-L. -Llib \
		-lraylib \
		-Wl,-Bstatic \
		-static-libgcc \
		-lfreenect2_c \
		-lfreenect2 \
		-lusb-1.0 \
		-lturbojpeg \
		-lstdc++ \
		-lwinpthread \
		-o $@

# examples: libfreenect2_c.dll main.exe
#
# main.exe: examples/main.c
# 	gcc -Wall -Wextra -Wpedantic -O3 -Os -s -I. -Iinclude $< -L. -lfreenect2_c -lraylib -o $@

-include $(DEP)

db: compile_commands.json

compile_commands.json: Makefile
	compiledb -n make all examples --cmd=mingw32-make

clean:
	rm -rf $(OBJ) $(DEP) *.a *.dll *.exe

.PHONY: all clean db examples
