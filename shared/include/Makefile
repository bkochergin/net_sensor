include ../../Makefile.inc

INCLUDES=-I../../shared -I..

all: address.o configuration.o country.o dns.o oui.o string.o timeStamp.o
	ar rcs ../lib/shared.a *.o

address.o: address.h address.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o address.o address.cpp

configuration.o: configuration.h configuration.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o configuration.o configuration.cpp

country.o: country.h country.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} -o country.o country.cpp

dns.o: dns.h dns.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o dns.o dns.cpp

oui.o: oui.h oui.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o oui.o oui.cpp

string.o: string.h string.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o string.o string.cpp

timeStamp.o: timeStamp.h timeStamp.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o timeStamp.o timeStamp.cpp

clean:
	rm -f *.o ../lib/shared.a
