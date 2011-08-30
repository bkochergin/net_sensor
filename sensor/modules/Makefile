SUBDIRS=bt http httpLog

all:
	@for subdir in ${SUBDIRS}; do (cd $$subdir; echo "===>" \
		${dir}$$subdir "($@)"; make); done

clean:
	@for subdir in ${SUBDIRS}; do (cd $$subdir; echo "===>" \
		${dir}$$subdir "($@)"; make clean); done
