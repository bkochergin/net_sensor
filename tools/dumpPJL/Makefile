include ../Makefile.inc

dumpPJL: ${DEPENDENCIES} dumpPJL.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} \
		-I/usr/local/include/db51 \
		-L/usr/local/lib/db51 -ldb -o dumpPJL \
		dumpPJL.cpp ${LIBS}

clean:
	rm -f dumpPJL
