cc=gcc
cc2=g++

lib_o_cflags= -W -Wall -O3 -fpic -c
cflags= -W -Wall -O3
lib_so_cflags= -shared -Wl,-soname

deps=forkmonitor.h

target_obj=libforkmonitor

target1=forkmonitor
target2=filemonitor

current_dir=$(shell pwd)
build_dir=$(current_dir)/build
src_dir=$(current_dir)/src


all: dir $(target1) $(target2)
	echo "export LD_PRELOAD=$(build_dir)/libforkmonitor.so" >> ~/.bashrc
	echo "export FORKMONITOR_SOCKET=$(build_dir)/commsocket" >> ~/.bashrc


dir:
	mkdir build
	#chmod +777 build/
	#chmod +777 .*/*

%.o: $(src_dir)/%.c
	$(cc) $(lib_o_cflags) $^ -o $(build_dir)/$@

$(target_obj).so: $(target_obj).o
	$(cc) $(lib_so_cflags),$(target_obj).so $(build_dir)/$^ -ldl -o $(build_dir)/$(target_obj).so

$(target1): $(target_obj).so $(src_dir)/$(target1).c
	$(cc)   $(cflags) $(word 2,$^) -o $(build_dir)/$(target1) -I .
	#$(cc) -shared $(build_dir)/$(target1).o -o $(build_dir)/$(target1).so

$(target2): $(src_dir)/$(target2).cpp
	$(cc2)  $^ -o $(build_dir)/$(target2) -lpthread
	#$(cc2) -shared $(build_dir)/$(target2).o -o $(build_dir)/$(target2).so

clean:
	rm -rf build/
	head -n -2 ~/.bashrc > ~/.bashrc.temp
	head -n -2 ~/.bashrc.temp > ~/.bashrc

