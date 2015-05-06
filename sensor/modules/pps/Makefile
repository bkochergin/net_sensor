include ../Makefile.inc

pps.so: mail.hpp stats.hpp pps.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC ${INCLUDES} \
		-I/usr/local/include -c pps.cpp
	${CXX} ${CXXFLAGS} -shared -lesmtp -L/usr/local/lib \
		-o pps.so pps.o ${LIBS}

clean:
	rm -f pps.o pps.so
