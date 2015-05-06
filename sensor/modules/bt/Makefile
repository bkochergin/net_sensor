include ../Makefile.inc

bt.so: udpTrackerSession.h udpTrackerSession.cpp bt.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC ${INCLUDES} -c udpTrackerSession.cpp
	${CXX} ${CXXFLAGS} -fPIC ${INCLUDES} \
		-I/usr/local/include -c bt.cpp
	${CXX} ${CXXFLAGS} -shared -lesmtp -L/usr/local/lib \
		-o bt.so udpTrackerSession.o bt.o ${LIBS}

clean:
	rm -f udpTrackerSession.o bt.o bt.so
