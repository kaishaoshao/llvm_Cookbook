SOURCES = $(shell find ast kaleidoscope lexer logger parser -name '*.cc')
HEADERS = $(shell find ast kaleidoscope lexer logger parser -name '*.h')
OBJ = ${SOURCES:.cc=.o}

CC =  clang++  -std=c++14
CFLAGS = -g -O0 -I llvm/include -I llvm/build/include -I ./
LLVMFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs all`

.PHONY: main

main: main.cc ${OBJ}
	${CC} ${CFLAGS}  ${LLVMFLAGS} ${OBJ} $< -o $@

clean:
	rm -r ${OBJ}

%.o: %.cpp ${HEADERS}
	${CC} ${CFLAGS} ${LLVMFLAGS} -c $< -o $@
