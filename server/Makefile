CXX=g++
INCFLAGS=-I.
LIBFLAGS=-L.
CXXFLAGS=-std=c++11 -Wno-deprecated $(DEF)
LDFLAGS=$(LIBFLAGS)
DEF=
REL_FLAGS=-Ofast -DNDEBUG
DBG_FLAGS=-g -DDEBUG
PROF_FLAGS=-pg
SRCS=$(shell ls *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
REL_OBJS=$(patsubst %.cpp,%_release.o,$(SRCS))
DBG_OBJS=$(patsubst %.cpp,%_debug.o,$(SRCS))
PROF_OBJS=$(patsubst %.cpp,%_prof.o,$(SRCS))
program=main_release 
all: $(program)
main: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
main_release: $(REL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(REL_FLAGS) $(LDFLAGS)
main_debug: $(DBG_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(DBG_FLAGS) $(LDFLAGS)
main_prof: $(PROF_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(PROF_FLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCFLAGS) $(LIBFLAGS)
%_release.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(REL_FLAGS) $(INCFLAGS) $(LIBFLAGS)
%_debug.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(DBG_FLAGS) $(INCFLAGS) $(LIBFLAGS)
%_prof.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(PROF_FLAGS) $(INCFLAGS) $(LIBFLAGS)

clean:
	rm -rf *.o main main_release main_prof main_debug
