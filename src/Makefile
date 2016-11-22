
include build_config.mk
export

all:
	make -C ${CURDIR}/ant/base
	make -C ${CURDIR}/ant/util
	make -C ${CURDIR}/ant/rpc
	make -C ${CURDIR}/ant/common
	make -C ${CURDIR}/ant/server
	make -C ${CURDIR}/ant/master
	make -C ${CURDIR}/ant/worker_server
	make -C ${CURDIR}/ant/tests/rpc

clean:
	make -C ${CURDIR}/ant/base clean
	make -C ${CURDIR}/ant/util clean
	make -C ${CURDIR}/ant/rpc clean
	make -C ${CURDIR}/ant/common clean
	make -C ${CURDIR}/ant/server clean
	make -C ${CURDIR}/ant/master clean
	make -C ${CURDIR}/ant/worker_server clean

