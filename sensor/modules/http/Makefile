include ../Makefile.inc

http.so: http.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC ${INCLUDES} -c http.cpp
	${CXX} ${CXXFLAGS} -shared -o http.so http.o ${LIBS}

clean:
	rm -f http.o http.so
