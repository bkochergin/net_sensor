include ../Makefile.inc

pjl.so: pjl.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC ${INCLUDES} \
		-I/usr/local/include/db51 -c pjl.cpp
	${CXX} ${CXXFLAGS} -Wall -Wextra -shared -ldb -L/usr/local/lib/db51 \
		-o pjl.so pjl.o ${LIBS}

clean:
	rm -f pjl.o pjl.so
