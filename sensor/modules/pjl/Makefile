include ../Makefile.inc

pjl.so: pjlSession.h pjl.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC ${INCLUDES} -I${DB_INCLUDES} -c pjl.cpp
	${CXX} ${CXXFLAGS} -shared -ldb -L${DB_LIBS} -o pjl.so pjl.o ${LIBS}

clean:
	rm -f pjl.o pjl.so
