include ../Makefile.inc

dumpHTTP: ${DEPENDENCIES} message.hpp dumpHTTP.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} -I${DB_INCLUDES} \
		 -L${DB_LIBS} -ldb -o dumpHTTP dumpHTTP.cpp ${LIBS}

clean:
	rm -f dumpHTTP
