#!/bin/python

import subprocess as sub


def file_handler(file_t,type_t):
	if type_t == "html":
		sub.call(["lynx",file_t])

	
	
	
	
	
	
	
	
	
