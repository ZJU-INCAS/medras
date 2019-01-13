.PHONY: all clean realclean

CXX := g++
CXXFLAGS := -std=c++11 -g -O3
CPPFLGS :=
LDFLAGS :=
LDLIBS := -g


all : main
main_objs := main.o	medrAlg.o medrDP.o
OBJS := $(main_objs)

main : ${main_objs}


# The rule to link the executables.
main:
	${CXX} -o $@ ${LDFLAGS} $^ ${LDLIBS}

# The rule to compile object files and produce dependencies.
%.o : %.cc
	${CXX} -o $@ ${CPPFLAGS} ${CXXFLAGS} -MD -MP $<

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) main

-include $(OBJS:.o=.d)



