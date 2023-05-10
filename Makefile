IDIR =./
ODIR=obj

CC=g++
CXXFLAGS=-I$(IDIR) -std=c++20 -g
LIBS=
TEST_CXXFLAGS=-I$(IDIR)/third_party

_DEPS = ThreadSafeMemOp.hpp Debug.hpp
_OBJ = ThreadSafeMemOp.o
_MAIN_OBJ = main.o 
_TEST_OBJ = ThreadSafeMemOpTest.o

all: mkdir main unittests

DEPS = $(patsubst %,$(IDIR)/include/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
MAIN_OBJ = $(patsubst %,$(ODIR)/%,$(_MAIN_OBJ))
TEST_OBJ = $(patsubst %,$(ODIR)/tests/%,$(_TEST_OBJ))


$(ODIR)/main.o: main.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(ODIR)/%.o: src/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(ODIR)/tests/%.o: tests/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS) $(TEST_CXXFLAGS)

main: $(OBJ) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS)

unittests: $(OBJ) $(TEST_OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: all main unittests clean mkdir

MKDIR_P = mkdir -p

mkdir:
	${MKDIR_P} ${ODIR}
	${MKDIR_P} ${ODIR}/tests

clean:
	rm -f ./main ./unittests
	rm -rf ${ODIR}