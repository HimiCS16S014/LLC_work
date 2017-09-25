# This uses sum as the measure and decide the ranking based on avg sum across things... 
# How to run:
# python plotDeviations11_v4.py 0 31 
# python plotDeviations11_v4.py 0 34 
# python plotDeviations11_v4.py 5 5 


import linecache #to read a random line from a file..
import matplotlib.pyplot as plt
import numpy as np
import sys
import operator

start = int(sys.argv[1])	#How many files u want to process...start file=0
end = int(sys.argv[2])		#end file= 31 or something else...	
S=int(sys.argv[3])
E=int(sys.argv[4])
SetNumOfL1 = int(sys.argv[5])
counter=int(sys.argv[6])
increment = 1
numProfiles = 1
base = 0


KeyRelated_ByteValue = 0	#Dont know
count = 1 #for ranking a group...counts the position
numOfFiles = 4

# < File'i' , Property, ValueofThisGroupNum >
# <2,1,5> : 2nd File , groupNum 5's #sumwise# rank/position 
# Total : NumFiles*4*16
positions = np.zeros((numOfFiles, 4,16))  #Store ranks of each group of a byte: ..(rank for each profile separate)2D list for a single byte


def computeEntropy(pmax):
    global KeyRelated_ByteValue 
    guesslist=list()
    found=0
    entropy=0
		
    for index in range(256):
	for index1 in range(0,16):
	    element=index1+base
		#byte 0 s PT byte ^ arrayIndex (sbox)  PT^K =S so PT^K=S
		# return byte value for which high deviation is there...
		# element indicates the index into lookup table array (sbox array )..	: lookup table element accessed index
	    kguess = pmax[index][0] ^ element
	    if(kguess not in guesslist):                
		guesslist.append(kguess)
		entropy+=1                                                
		if (kguess == int(key1[byteNum])):
		    KeyRelated_ByteValue=pmax[index][0]	
		    found=1
		    break
	    else:
		continue
	if (found == 1):		  
	      break 
    return entropy
	    

# xxx can be sum or dev or variance..Data will change..
# can use it for any group..expected group or actual group ...
def xxxWisePosition(data,groupToFindPosition):
    global count 
    listOfVisited = []
    for j in range(0,256):#256 is max (dev case), for certain data only 16 will be there...		    
	if data[j][0]==groupToFindPosition:
	    break
	else:
	    if (data[j][0] not in listOfVisited):
		count += 1
	    listOfVisited.append(data[j][0])    
    return count
    


def computeAllPositionsOfByte(profileNum, groupNum,sortedDeviations_grpIds,zsums,zvariancesOfGroups):
    global count
    count=1
    devPosition = xxxWisePosition(sortedDeviations_grpIds,groupNum)		
    positions[profileNum][0][groupNum] = devPosition
    
    count=1
    sumPosition = xxxWisePosition(zsums,groupNum)
    positions[profileNum][1][groupNum] = sumPosition
    
    count=1		
    varPosition = xxxWisePosition(zvariancesOfGroups,groupNum)
    positions[profileNum][2][groupNum] = varPosition
    
    positions[profileNum][3][groupNum] = sum([positions[profileNum][i][groupNum] for i in range(0,3)])/3.0
    	
	
def printPositions():  
    print 'GroupNum  DeviationWise SumWise VarianceWise Overall 	'
    #print 'GroupNum  DeviationWise SumWise VarianceWise Overall 	VarianceOf3Measures'
    for g in range(0,16):
	print g,
	print '  	  ',
	print positions[0][g],
	print '    	',
	print positions[1][g],
	print '   	 ',
	print positions[2][g],
	print '		',
	print "%.2f" % round(positions[3][g],2)
	#print '		',
	#print "%.2f" % round(positions[4][g],2)


def printGroupRanks():
    for k in range(0, numOfFiles):
	groupRanking=zip(range(0,16),positions[k][3])
	groupRanking=sorted(groupRanking, key=operator.itemgetter(1),reverse=False)	  
	print 'Group Ranking:',
	print [groupRanking[i][0] for i in range(0,16)] 	  
    #return [groupRanking[i][0] for i in range(0,16)] 	  


def printTopGroups(sortedDeviations_grpIds,zsums,zvariancesOfGroups):
    print 'Deviation wise:',
    print [sortedDeviations_grpIds[i][0] for i in range(0,16)]		    
    print 'Sumwise:',
    print [zsums[i][0] for i in range(0,10)]
    print "Variance wise:",		    
    print [zvariancesOfGroups[i][0] for i in range(0,10)],
    print zvariancesOfGroups[0:3]

			  
def printGroupInfo():
    #printTopGroups(sortedDeviations_grpIds,zsums,zvariancesOfGroups)
    #printPositions()
    ranks = printGroupRanks()
    #print 'Sum of all vairations',
    #print sum([positions[4][g] for g in ranks[0:4]])

def printByteInfo(entropy,sums,deviations):
    print 'Entropy:', entropy,	
    #print '  <---> Guessed Entropy is b/n '+str((count*16)-15)+' to '+str(count*16)
    #print '---> Dev:' + str(devPostion)+ ' Sum:' + str(sumPosition) + ' Var:'+ str(varPosition)
    print "variance of <Sums> of all groups:",
    print np.var(sums),
    print "	 V<Devs>:",
    print np.var(deviations)
		      

#initilaize() 
for power in range(start, end+1, increment):
    var_ofSums = []
    #print key1
    #Process Look Up table wise bytes......
    for LUT in range(0,1): 
	#each LUT properties	
	avgRanksOfGroups = [0]*16
	varOfPositionOfGroups = [0]* 16
	
	#Process Look Up table wise bytes......All bytes in a LUT (+4)
	avgsRanksPerBytePerLUT = [0]*4 
	varsPerBytePerLUT = [0]*4
	indexIntoAvg = 0
	for byteNum in range(0,16,4):
	    byteNum += LUT
	    
	    for ithFile in range(1, numOfFiles+1, 1):			      
		filename = str(power)+'_'+str(ithFile)		
		keys=linecache.getline(filename, 1) #reading keys
		key1 = keys.split()	    
		
		for profile in range(1,numProfiles+1):#DUMMY RUNS ONLY ONCE	    	    		    
		    #print filename	    	     
		    line = linecache.getline(filename, byteNum+2)  #will fetch 16 deviation values    (as one grp)  
		    deviations = [abs(float(dev)) for dev in line.split()]
		    
		    x=zip(range(0,256),[abs(float(dev)) for dev in deviations])
		    pmax=sorted(x,key=operator.itemgetter(1),reverse=True)	#sorted deviations
		    groups=[x[i:i+16] for i in range(0, len(x), 16)] 	#divide into groups of each size 16
		    for i in range(0,16):	#sorting the elements within a group...
			groups[i] = sorted(groups[i],key=operator.itemgetter(1),reverse=True) 
		
		    
		    #Dont know
		    # ---------------- SortedDeviations_grpIds contains tuples of <groupId, deviations> pairs-------
		    # total 16 groups (0-15): byteValue 0-15 form group0, 16-31 group1...so on 		    
		    sortedDeviations_grpIds=pmax[:] 			
		    for i in range(0,256):
			pair = list(sortedDeviations_grpIds[i])
			pair[0]/=16
			sortedDeviations_grpIds[i] = tuple(pair)
			#sortedDeviations_grpIds[i][0] /= 16	    #cant edit a tuple directly..need to convert into list first...
		    #Dont know upto here
		    
		    #----------------SUMs of groups (each group is of 16 elements)-------------		    	
		    sums=[sum(pair[1] for pair in groups[i]) for i in range (0,16)]  #sum of all 16 deviations within a group...16 such sums for each group
		    #zipped variables..SUMs as a group!!
		    zsums=zip(range(0,16),sums)
		    zsums=sorted(zsums, key=operator.itemgetter(1),reverse=True)

		    #Dont know 
		    #----------------Variances of groups (each group is of 16 elements)-------------		    			    
		    variancesOfGroups = []
		    for i in range (0,16):
			group = (zip(*groups[i]))[1]	#group[i] is tuple <groupNum, all 16 deviations in sorted order>
			variancesOfGroups.append(np.var(group))
		    zvariancesOfGroups=zip(range(0,16),variancesOfGroups)
		    zvariancesOfGroups=sorted(zvariancesOfGroups, key=operator.itemgetter(1),reverse=True)		
		    #Dont know upto here
		    
		    #--------------Compute Position of each group wrt Sum, TopDev and var : For a single byte...
		    for groupNum in range(0,16):
			#computed and stored in positions[] list
			computeAllPositionsOfByte(ithFile-1 ,groupNum,sortedDeviations_grpIds,zsums,zvariancesOfGroups)
		    
	    	    
	    for grp in range(0,16):	      
		avgRanksOfGroups[grp] = sum([positions[i][3][grp] for i in range(0,numOfFiles)])/numOfFiles  # 1 represent the property sum
		#print sum([positions[i][3][grp] for i in range(0,numOfFiles)])/numOfFiles,	    
		varOfPositionOfGroups[grp] = np.var([positions[i][3][grp] for i in range(0,numOfFiles)])
		#print np.var([positions[i][3][grp] for i in range(0,numOfFiles)]),	    
		
		
	    avgsRanksPerBytePerLUT[indexIntoAvg]= round(min(avgRanksOfGroups),2)
	    varsPerBytePerLUT[indexIntoAvg] = round(varOfPositionOfGroups[avgRanksOfGroups.index(min(avgRanksOfGroups))],2)
	    #print '<',
	    #print avgsRanksPerBytePerLUT[indexIntoAvg], 
	    #print ',',
	    #print varsPerBytePerLUT[indexIntoAvg] ,
	    #print '>  ',	    
	    indexIntoAvg += 1	#incremented for each byte related to a LUT; 4 bytes realted to each LUT; Index0-> byte0   Index1->byte4   Index2->byte8   Index3->byte12

    print power-32, 
    fd = open('avgRank_variation','a')
    print >> fd,sum(avgsRanksPerBytePerLUT)/4 # Take sum of ranks of all 4 bytes together....to reperesnet that ets/profiles property
    fd.close()	      
   
    
    fd1 = open('Testing','a')
    
    if(counter == 1) :
        print >> fd1
        print >> fd1, "SetNumOfL1 = ", SetNumOfL1
        print >> fd1
   
    print >> fd1, power-32, 
    print >> fd1, ' S=',
    print >> fd1, S,
    print >> fd1, ' E=',
    print >> fd1, E,

    if(sum(avgsRanksPerBytePerLUT)/4 < 2) :
	print 'Y',
	print avgsRanksPerBytePerLUT
	print
	print >> fd1,' Y',
	print >> fd1,avgsRanksPerBytePerLUT,
	print >> fd1,sum(avgsRanksPerBytePerLUT)/4
	
    else :
        print 'N',
	print avgsRanksPerBytePerLUT,
        print >> fd1,' N',
	print >> fd1,avgsRanksPerBytePerLUT,
	print >> fd1,sum(avgsRanksPerBytePerLUT)/4
    	

	
	
    #print >> fd,sum(avgsRanksPerBytePerLUT)/4 # Take sum of ranks of all 4 bytes together....to reperesnet that sets/profiles property
   
       # print >> fd,'------------------------------------------------------------------------------------------'
    fd1.close()	      

    
