include ../../Makefile.inc

DEPENDENCIES=../../shared/include/*
INCLUDES=-I../../shared -I..

all: berkeleyDB.o endian.o flowID.o httpParser.o httpSession.o logger.o \
		module.o packet.o smtp.o Makefile
	ar rcs ../lib/sensor.a *.o

berkeleyDB.o: berkeleyDB.h berkeleyDB.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -I${DB_INCLUDES} \
		-c -o berkeleyDB.o berkeleyDB.cpp

endian.o: endian.h endian.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o endian.o endian.cpp

flowID.o: ${DEPENDENCIES} flowID.h flowID.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} -o flowID.o flowID.cpp

httpParser.o: httpParser.h httpParser.c Makefile
	${CC} ${CFLAGS} -fPIC -c -o httpParser.o httpParser.c

httpSession.o: ${DEPENDENCIES} httpSession.h httpSession.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} -o \
		httpSession.o httpSession.cpp

logger.o: logger.h logger.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c -o logger.o logger.cpp

module.o: ${DEPENDENCIES} module.h module.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} -o module.o \
		module.cpp

packet.o: ${DEPENDENCIES} packet.h packet.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} -o packet.o \
		packet.cpp

smtp.o: ${DEPENDENCIES} smtp.h smtp.cpp Makefile
	${CXX} ${CXXFLAGS} -fPIC -c ${INCLUDES} \
		-I/usr/local/include -I/opt/local/include -o smtp.o smtp.cpp

clean:
	rm -f *.o ../lib/sensor.a
