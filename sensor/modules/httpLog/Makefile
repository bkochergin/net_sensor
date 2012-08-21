include ../Makefile.inc

httpLog.so: httpLog.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC ${INCLUDES} -I${DB_INCLUDES} \
		-c httpLog.cpp
	${CXX} ${CXXFLAGS} -Wall -Wextra -shared -ldb -L${DB_LIBS} \
		-o httpLog.so httpLog.o ${LIBS}

clean:
	rm -f httpLog.o httpLog.so
