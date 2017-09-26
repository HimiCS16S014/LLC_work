#!/usr/bin/python

import os, sys

# Path to be created
path = "/scratch/researchWork_from_May2017/New_work/1.storing_files"
for power in range(19,20, 1):	
    for length in range(13,17, 1):	
		os.mkdir( path +'/' + 'Deviation_profiles_'+str(power)+'_'+str(length) , 0755 );
		os.mkdir( path +'/' + 'Timing_profiles_'+str(power)+'_'+str(length) , 0755 );

