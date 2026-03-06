SRC := $(wildcard *.cpp)
OBJ := $(addsuffix .o,$(basename $(SRC)))
DEP := $(OBJ:%.o=%.d)

all: libfreenect2_c.dll

libfreenect2_c.dll: $(OBJ)
	g++ $^ -shared -Llib -lfreenect2 -o $@ 

%.o: %.cpp
	g++ -Wall -Wextra -Wpedantic -std=c++17 -Iinclude -MMD -MP -c $< -o $@

examples: main.exe

main.exe: examples/main.c libfreenect2_c.dll
	gcc -Wall -Wextra -Wpedantic -I. -Iinclude $< -lfreenect2_c -lfreenect2 -lraylib -L. -Llib -o $@

-include $(DEP)

db: compile_commands.json

compile_commands.json: Makefile
	compiledb -n make all examples --cmd=mingw32-make

clean:
	rm -rf $(OBJ) $(DEP) libfreenect2_c.dll main.exe

.PHONY: all clean db example
