#Changes: commandline argments, filename argument doesnt have suffix, ...

# ADDED PARAMETER: learnCost

#python ComputeAndPlotSR.py 16 20 timing4 16 21 0 0

# o_th order avg success rate:  probability of correct key in the top 'o' results
# Say o=16; we make guesslist of 16 keys. 
# Run 100 times; Number of times out of 100 key present in the guess list (of 16 keys) is x then
# Success rate = x /100

#	--> learningFlag != 1  OR  	learningFlag = 0  so  
#	--> *invoke python 'ComputeAndPlotSR' checks parameter learnCost=5, that plots enc 16...21 as +5 ie 21..26 vs SRs	
#				this 'ComputeAndPlotSR' reads from files names timingx_16,...21 ..But while plotting it adds learnCost=5
#				if learnCost != 0
#				    add statically constant SR for encs 16,20..what SR to be added.??
#				    compute from 'setwise_SR' file, take average of all.. and use that SR and *delete that file...*

import sys
import operator
import linecache
import matplotlib.pyplot as plt
import time
import os

order = int(sys.argv[1]) 
numProfiles = int(sys.argv[2]) 
#filename = sys.argv[3]
filename = "timing"
startpower = int(sys.argv[4])
endpower = int(sys.argv[4])
base = int(sys.argv[3]) 


def plotMyValues(x,y,style,lab):
    plt.xticks(fontsize = 15)
    plt.yticks(fontsize = 15)
    plt.plot(x, y,  style , label=lab)
    
def setAttributesPlot():
    plt.axis([min(16,startpower),max(22,endpower),0,105])
    legend = plt.legend(loc='upper left', shadow=True, fontsize='x-large')
    # Put a nicer background color on the legend.
    #legend.get_frame().set_facecolor('#00FFCC')
    plt.xlabel('#of encryptions (power of 2)',fontsize=20)	
    plt.ylabel('SuccessRate',fontsize=20)	#optional    
    plt.grid(True)



fig = plt.figure()

#File has line 1: has key; line  2-17 have byte0 to 15 deviation information..
#line 18 key			line 35 key			1,18,35...
#lines 19-34 have byteinfo	lines 36-51 have byteinfo	byte0: 2,19,36...

# Every 17*i + 1  th line has key information... numberOfProfiles> i >=0 ( i=Profile Number(starts from 0)  )
# Every 17*i + j line has profiled information about Byte (j-2)		i>=0; 17>= j >=2 (j=2 means byte0, j=3 means byte1...)

#Just dummy
for Suffix in range(0,1):
    
    listOfEncs = range(startpower,endpower+1)

    srForManyEncs = []	#list of lists
    #for power in range(startpower,endpower+1):
    for power in range(startpower,endpower+1):
	#print "power:",power	#DEBUG
	successRatesPerEnc = [] # for each bytes
	for byteNum in range(0,1,1):
	#for byteNum in range(0,1,1):
	    #byteNum = 0
	    successes = 0
	    #for profileNum in range(0,numProfiles):	    
	    for i in range(1,numProfiles+1):
		f1=open("DataFiles/deviationprofile"+str(i)+"_"+str(power)+".txt","r")
		line = linecache.getline("DataFiles/deviationprofile"+str(i)+"_"+str(power)+".txt", 1)

		#print profileNum, filename+str(Suffix)+"_"+str(power)+".txt"		
		#print profileNum
		#line = linecache.getline(filename+str(Suffix)+"_"+str(power)+".txt",17*profileNum+1)
		knownkeys = [int(key) for key in line.split()]
		lineNum = byteNum +2 		
		line = linecache.getline("DataFiles/deviationprofile"+str(i)+"_"+str(power)+".txt", lineNum)
		#line = linecache.getline(filename+str(Suffix)+"_"+str(power)+".txt", 17*profileNum+lineNum)
		if line == '':
		    print "-------WRONG!!---cant read lineNum ", (lineNum), "from profile"+str(i)+"_"+str(power)+".txt"
		devValuesStr =line.split()            
		deviations =zip(range(0,256),[abs(float(dev)) for dev in devValuesStr])            		
		#print knownkeys
		#print "first 2 Deviations ",deviations[0:2]

		guesslist=list()
		#sorted values of deviation : (93, 2.715881), (87, 2.697266), (89, 2.692505)..
		devSorted=sorted(deviations,key=operator.itemgetter(1),reverse=True)
		found=0
		entropy0=0
		for index in range(256):
		    for index1 in range(0,16):
			element=index1+base                    
			kguess = devSorted[index][0] ^ element
			if(kguess not in guesslist):                
			    #guesslist.append(kguess)
			    if len(guesslist)!=order:
				guesslist.append(kguess)
			    entropy0+=1
			    if (kguess == knownkeys[byteNum]):
				found=1
				break
			else:
			    continue
		    if (found == 1):
			break                        
		#print "Guesslist",len(guesslist), guesslist            
		if knownkeys[byteNum] in guesslist:
		    successes += 1 
		    #print "Key byte", byteNum , " found in position",guesslist.index(knownkeys[byteNum]) 
		#else:
		    #print "Key byte", byteNum ,"Not found" 	    
	    #End of all profiles for one encryption power
	    
		
	    sr = 1.0*successes/numProfiles * 100 	  
	    #print "2^Encryptions=",power," Byte ",byteNum,": Order-",order," Success Rate is ",sr
	    successRatesPerEnc.append(sr)  
	    #End of processing success rate of a byte
	    
	srForManyEncs.append(successRatesPerEnc)    
	#End of processing success rate of all bytes for one encryption power
	
    #print srForManyEncs    
    #byte = 0
    byte = 0
    listOfSRs = [srperEnc[byte] for srperEnc in srForManyEncs]
    #print "2^Power ->: ",listOfEncs," SR: ", listOfSRs
  #  print "2^",listOfEncs[0],"Encs: Byte:",byte," SR:", listOfSRs[0]

    fd = open('setwise_SR','a')
    print >> fd,listOfSRs[0] # byte0 instead of 4/8/12
    fd.close()



























