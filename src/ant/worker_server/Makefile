
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++


CPP_SOURCES := \
	heartbeater.cc \
	worker_server.cc \
	worker_server_options.cc \

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)


STATIC_LIB=libworker_server.a

LIBS += $(STATIC_LIB)

OBJECTS += $(CPP_OBJECTS)

all: $(OBJECTS) $(LIBS) ant_worker_server

$(STATIC_LIB): $(OBJECTS)
	@echo "  [LINK] $@"
	@ar rs $@ $(OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<


%.pb.cc: %.proto
	@echo "  [GEN]  $@"
	@protoc  --cpp_out $(SRC_DIR) --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<
%.service.pb.cc: %.proto
	protoc  --plugin=$(SRC_DIR)/ant/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
		--proto_path /usr/local/include $(CURDIR)/$<
%.proxy.pb.cc: %.proto
	protoc  --plugin=$(SRC_DIR)/ant/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
		--proto_path /usr/local/include $(CURDIR)/$<

ant_worker_server: worker_server_main.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ worker_server_main.o \
		$(SRC_DIR)/ant/worker_server/libworker_server.a \
		$(SRC_DIR)/ant/master/libmaster.a \
		$(SRC_DIR)/ant/server/libserver.a \
		$(SRC_DIR)/ant/common/libcommon.a \
		$(SRC_DIR)/ant/rpc/librpc.a \
		$(SRC_DIR)/ant/util/libutil.a \
		$(SRC_DIR)/ant/base/libbase.a \
		$(SRC_DIR)/ant/http/libhttp.a \
	-lglog -lgflags -L/usr/local/lib -lprotobuf -lprotoc -lpthread -lssl -lcrypto \
	-lz -lev -lsasl2 -lpcre -ldl

clean:
	@rm -fr $(OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr *.pb.h *.pb.cc ant_worker_server
	@rm -fr *.o
