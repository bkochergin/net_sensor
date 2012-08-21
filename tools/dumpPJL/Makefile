include ../Makefile.inc

dumpPJL: ${DEPENDENCIES} dumpPJL.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} -I${DB_INCLUDES} \
		-L${DB_LIBS} -ldb -o dumpPJL dumpPJL.cpp ${LIBS}

clean:
	rm -f dumpPJL
