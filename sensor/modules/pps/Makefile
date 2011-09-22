include ../Makefile.inc

pps.so: pps.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC ${INCLUDES} \
		-I/usr/local/include -c pps.cpp
	${CXX} ${CXXFLAGS} -Wall -Wextra -shared -lesmtp -L/usr/local/lib \
		-o pps.so pps.o ${LIBS}

clean:
	rm -f pps.o pps.so
