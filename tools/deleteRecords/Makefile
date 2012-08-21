include ../Makefile.inc

dumpPJL: ${DEPENDENCIES} deleteRecords.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES}  -I${DB_INCLUDES} \
		-L${DB_LIBS} -ldb -o deleteRecords deleteRecords.cpp ${LIBS}

clean:
	rm -f deleteRecords
