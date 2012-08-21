include ../Makefile.inc

pjl.so: pjl.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC ${INCLUDES} -I${DB_INCLUDES} \
		-c pjl.cpp
	${CXX} ${CXXFLAGS} -Wall -Wextra -shared -ldb -L${DB_LIBS} -o pjl.so \
		pjl.o ${LIBS}

clean:
	rm -f pjl.o pjl.so
