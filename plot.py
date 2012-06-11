#!/usr/bin/python

# python script to plot a two column matrix on disk takes two
# arguments, one is the data file and the other is the title string.
import sys                       # for command line arguments
from scipy import *              
from matplotlib.pyplot import *  # for plot commands
from time import sleep

if len(sys.argv)<=1:
    print "usage: ./plot.py filename [title]"
    print "Note that title, if present should be in single quotes"
    quit()

A=loadtxt(sys.argv[1])           # read in the file on disk
x=A[:,0]                         # separate into columns
y=A[:,1]
clf()
curve=plot(x,y,'k',linewidth=2)  # plot with a line
if len(sys.argv)>2:
    title(sys.argv[2])           # put in the title if it exists

show()
