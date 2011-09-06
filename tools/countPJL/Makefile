include ../Makefile.inc

countPJL: ${DEPENDENCIES} countPJL.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} \
		-I/usr/local/include/db51 \
		-L/usr/local/lib/db51 -ldb -o countPJL \
		countPJL.cpp ${LIBS}

clean:
	rm -f countPJL
