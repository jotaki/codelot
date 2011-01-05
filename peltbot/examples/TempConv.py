#! /usr/bin/python

def f2c(f):
	try:
		return str((float(f) - 32) / 1.8);
	except ValueError:
		return "Not a tempeture";

def c2f(c):
	try:
		return str((float(c) * 1.8) + 32);
	except ValueError:
		return "Not a tempeture";
