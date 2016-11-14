CXXFLAGS += -std=c++11
CXXFLAGS += -I./
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-deprecated -g -c -o

CFLAGS += -I./
CFLAGS += -Wall -g -c -o

LIB_FILES :=-lglog -lgflags -levent  -lpthread -lssl -lcrypto -lz
TEST_LIB_FILES :=  -L/usr/local/lib -lgtest -lgtest_main -lpthread

PROTOC = protoc
GRPC_CPP_PLUGIN=grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
PROTOS_PATH = ./protos

C_SOURCES := \
	./ant/base/dynamic_annotations.c

CPP_SOURCES := \
	./ant/base/bits.cc \
	./ant/base/callback_internal.cc \
	./ant/base/cpu.cc \
	./ant/base/int128.cc \
	./ant/base/mathlimits.cc \
	./ant/base/once.cc \
	./ant/base/ref_counted.cc \
	./ant/base/ref_counted_memory.cc \
	./ant/base/spinlock.cc \
	./ant/base/spinlock_internal.cc \
	./ant/base/sysinfo.cc \
	./ant/base/walltime.cc \
	./ant/base/threading/thread_collision_warner.cc \
	\
	./ant/base/stringprintf.cc \
	./ant/base/strtoint.cc \
	./ant/base/strings/ascii_ctype.cc \
	./ant/base/strings/charset.cc \
	./ant/base/strings/escaping.cc \
	./ant/base/strings/human_readable.cc \
	./ant/base/strings/join.cc \
	./ant/base/strings/memutil.cc \
	./ant/base/strings/numbers.cc \
	./ant/base/strings/serialize.cc \
	./ant/base/strings/split.cc \
	./ant/base/strings/strcat.cc \
	./ant/base/strings/stringpiece.cc \
	./ant/base/strings/strip.cc \
	./ant/base/strings/substitute.cc \
	./ant/base/strings/util.cc \
	./ant/base/utf/rune.cc \
	\
	./ant/base/hash/jenkins.cc \
	./ant/base/hash/hash.cc \
	./ant/base/hash/city.cc \
	\
	\
	./ant/util/faststring.cc \
	./ant/util/malloc.cc \
	./ant/util/slice.cc \
	./ant/util/status.cc \
	./ant/util/status_callback.cc \
	./ant/util/monotime.cc \
	./ant/util/mutex.cc \
	./ant/util/condition_variable.cc \
	./ant/util/errno.cc \
	./ant/util/thread_stats.cc \
	./ant/util/atomic.cc \
	./ant/util/semaphore.cc \

C_OBJECTS := $(C_SOURCES:.c=.o)

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)
CPP_OBJECTS += $(C_OBJECTS)

TESTS := \
	./ant/util/slice_unittest \
	./ant/util/status_unittest \
	./ant/util/monotime_unittest \

all: $(CPP_OBJECTS) $(TESTS)
.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<
.c.o:
	@echo "  [CXX]  $@"
	@$(CC) $(CFLAGS) $@ $<

./ant/util/slice_unittest: ./ant/util/slice_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./ant/util/slice_unittest.o: ./ant/util/slice_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./ant/util/status_unittest: ./ant/util/status_unittest.o \
	./ant/util/status.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./ant/util/status_unittest.o: ./ant/util/status_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./ant/util/monotime_unittest: ./ant/util/monotime_unittest.o \
	./ant/util/monotime.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./ant/util/monotime_unittest.o: ./ant/util/monotime_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<


#####################################################################
./unittests/strings/string_piece_unittest: \
	./unittests/strings/string_piece_unittest.o \
	./strings/string_piece.h \
	./strings/string_piece.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/strings/string_piece_unittest.o: \
	./unittests/strings/string_piece_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./unittests/strings/split_unittest: \
	./unittests/strings/split_unittest.o \
	./strings/split.h \
	./strings/split.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/strings/split_unittest.o: \
	./unittests/strings/split_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./unittests/event/event_loop_unittest: \
	./unittests/event/event_loop_unittest.o \
	./event/event_loop_libevent.h \
	./event/event_loop_interface.h \
	./event/event_loop_libevent.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/event/event_loop_unittest.o: \
	./unittests/event/event_loop_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./unittests/net/tcp_server_socket_unittest: \
	./unittests/net/tcp_server_socket_unittest.o \
	./event/event_loop_libevent.h \
	./event/event_loop_libevent.o \
	./net/test_completion_callback.o \
	./event/io_event_loop.h \
	./base/run_loop.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/net/tcp_server_socket_unittest.o: \
	./unittests/net/tcp_server_socket_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<


./unittests/server/http/http_server_unittest: \
	./unittests/server/http/http_server_unittest.o \
	./server/http/http_server.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/server/http/http_server_unittest.o: \
	./unittests/server/http/http_server_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

./unittests/event/io_event_loop_unittest: \
	./unittests/event/io_event_loop_unittest.o \
	./event/io_event_loop.o \
	./event/event_loop_libevent.o \
	./event/io_event_loop.h \
	./base/run_loop.o \
	./base/run_loop.h
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/event/io_event_loop_unittest.o: \
	./unittests/event/io_event_loop_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<


./unittests/threading/watchdog_unittest: \
	./unittests/threading/watchdog_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(LIB_FILES) $(TEST_LIB_FILES)
./unittests/threading/watchdog_unittest.o: \
	./unittests/threading/watchdog_unittest.cc
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

## /////////////////////////////

clean:
	rm -fr ./unittests/base/*.o
	rm -fr ./unittests/strings/*.o
	@rm -fr $(TESTS)
	@echo "rm *_unittest"
	@rm -fr $(CPP_OBJECTS)
	@echo "rm *.o"
