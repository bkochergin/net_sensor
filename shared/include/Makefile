all: address.o string.o timeStamp.o
	ar rcs ../lib/shared.a *.o

address.o: address.h address.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC -c -o address.o address.cpp

string.o: string.h string.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC -c -o string.o string.cpp

timeStamp.o: timeStamp.h timeStamp.cpp Makefile
	${CXX} ${CXXFLAGS} -Wall -Wextra -fPIC -c -o timeStamp.o timeStamp.cpp

clean:
	rm -f *.o ../lib/shared.a
