include ../Makefile.inc

dumpHTTP: ${DEPENDENCIES} message.hpp dumpHTTP.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} \
		-I/usr/local/include/db51 \
		-L/usr/local/lib/db51 -ldb -o dumpHTTP \
		dumpHTTP.cpp ${LIBS}

clean:
	rm -f dumpHTTP
