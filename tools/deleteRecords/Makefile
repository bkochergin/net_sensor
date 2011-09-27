include ../Makefile.inc

dumpPJL: ${DEPENDENCIES} deleteRecords.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} \
		-I/usr/local/include/db51 \
		-L/usr/local/lib/db51 -ldb -o deleteRecords \
		deleteRecords.cpp ${LIBS}

clean:
	rm -f deleteRecords
