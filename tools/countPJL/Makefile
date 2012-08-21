include ../Makefile.inc

countPJL: ${DEPENDENCIES} countPJL.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra ${INCLUDES} -I${DB_INCLUDES} \
		-L${DB_LIBS} -ldb -o countPJL countPJL.cpp ${LIBS}

clean:
	rm -f countPJL
