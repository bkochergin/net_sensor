include ../Makefile.inc

http.so: http.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC ${INCLUDES} -c http.cpp
	${CXX} ${CXXFLAGS} -Wall -Wextra -shared -o http.so http.o ${LIBS}

clean:
	rm -f http.o http.so
