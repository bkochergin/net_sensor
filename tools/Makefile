SUBDIRS=include countPJL dumpHTTP dumpPJL

all: ${SUBDIRS} Makefile
	@for subdir in ${SUBDIRS}; do (cd $$subdir; echo "===>" \
		${dir}$$subdir "($@)"; export dir=${dir}$$subdir/; make); done

clean:
	@for subdir in ${SUBDIRS}; do (cd $$subdir; echo "===>" \
		${dir}$$subdir "($@)"; export dir=${dir}$$subdir/; make \
		clean); done
