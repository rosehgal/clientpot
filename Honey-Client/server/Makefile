cc=g++
current_dir=$(shell pwd)
build_dir=$(current_dir)/build
src_dir=$(current_dir)/src

target = server

all: $(target)

$(target): $(src_dir)/$(target).cpp
	mkdir build
	$(cc) $^ -o $(build_dir)/$@ -lpthread

clean:
	rm -f $(build_dir)/$(target)
	rm -rf $(build_dir)/
