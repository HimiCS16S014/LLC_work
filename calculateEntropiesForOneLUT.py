# 2 argumets: 1st argument is the array index of LUT table that is evicted by LLC...
# say LUT0 stored from set 10 to 25.. so base set is 10; 
# If LUT0s 4 set is evicted then then the argument passed should be 4*16= 64; set0 of LUT evicted means pass 0 as argument
# Second argument is how many files to be processed.  (profiled information)
# Third argument tells which files you are processing...AES encrypted for 2^24 files, 2^22 files, or 2^20 files...That power is taken as input 20,22,24..
# example:  python calculateEntropiesForOneLUT.py 0 11 26
import sys
import operator
base = int(sys.argv[1])
numOfFiles = int(sys.argv[2])
power = int(sys.argv[3])
cumulative_entropies0=[0,0,0,0]
cumulative_entropies1=[0,0,0,0]
cumulative_entropies2=[0,0,0,0]
cumulative_entropies3=[0,0,0,0]
for i in range(1,numOfFiles+1):
    f1=open("DataFiles/deviationprofile"+str(i)+"_"+str(power)+".txt","r")
    key1=f1.readline().split()
    entropies0=list()
    entropies1=list()
    entropies2=list()
    entropies3=list()
    #count specifies which byte cumulative entropy you are adding up..
    count0=0
    count1=0
    count2=0
    count3=0
    for byte, line in enumerate(f1):
	if byte in [0,4,8,12]:
            guesslist=list()
            devValues1=line.split()
            x=zip(range(0,256),[abs(float(dev)) for dev in devValues1])
            pmax=sorted(x,key=operator.itemgetter(1),reverse=True)
            found=0
            entropy0=0
            for index in range(256):
                for index1 in range(0,16):
                    element=index1+base
			#byte 0 s PT byte ^ arrayIndex (sbox)  PT^K =S so PT^K=S
			# return byte value for which high deviation is there...
			# element indicates the index into lookup table array (sbox array )..	: lookup table element accessed index
                    kguess = pmax[index][0] ^ element
                    if(kguess not in guesslist):                
                        guesslist.append(kguess)
                        entropy0+=1
                        if (kguess == int(key1[byte])):
                            found=1
                            break
                    else:
                        continue
                if (found == 1):		  
		      break
	    # If you want to see, for each profile byte wise entropy...
	    #print entropy0 , 	    
	    #if byte==12:
		#print 
		
            entropies0.append(entropy0)
            cumulative_entropies0[count0]+=entropy0
            count0+=1 

	if byte in [1,5,9,13]:
            guesslist=list()
            devValues1=line.split()
            x=zip(range(0,256),[abs(float(dev)) for dev in devValues1])
            pmax=sorted(x,key=operator.itemgetter(1),reverse=True)
            found=0
            entropy1=0
            for index in range(256):
                for index1 in range(0,16):
                    element=index1+base
                    kguess = pmax[index][0] ^ element
                    if(kguess not in guesslist):                
                        guesslist.append(kguess)
                        entropy1+=1
                        if (kguess == int(key1[byte])):			   
                            found=1
                            break
                    else:
                        continue
                if (found == 1):		    	
                    break
            entropies1.append(entropy1)
            cumulative_entropies1[count1]+=entropy1
            count1+=1

	if byte in [2,6,10,14]:
            guesslist=list()
            devValues1=line.split()
            x=zip(range(0,256),[abs(float(dev)) for dev in devValues1])
            pmax=sorted(x,key=operator.itemgetter(1),reverse=True)
            found=0
            entropy2=0
            for index in range(256):
                for index1 in range(0,16):
                    element=index1+base
                    kguess = pmax[index][0] ^ element
                    if(kguess not in guesslist):                			
                        guesslist.append(kguess)
                        entropy2+=1
                        if (kguess == int(key1[byte])):
                            found=1
                            break
                    else:
                        continue
                if (found == 1):		    
                    break
            entropies2.append(entropy2)
            cumulative_entropies2[count2]+=entropy2
            count2+=1

        if byte in [3,7,11,15]:
            guesslist=list()
            devValues1=line.split()
            x=zip(range(0,256),[abs(float(dev)) for dev in devValues1])
            pmax=sorted(x,key=operator.itemgetter(1),reverse=True)
            found=0
            entropy3=0
            for index in range(256):
                for index1 in range(0,16):
                    element=index1+base
                    kguess = pmax[index][0] ^ element
                    if(kguess not in guesslist):                
                        guesslist.append(kguess)
                        entropy3+=1
                        if (kguess == int(key1[byte])):
                            found=1
                            break
                    else:
                        continue
                if (found == 1):
                    break
            entropies3.append(entropy3)
            cumulative_entropies3[count3]+=entropy3
            count3+=1

#    print entropies0		#entropies store entropy for each of the byte valuel 0,4,8,12; cumulative entropies takes the average of the four...
#    print entropies1
#    print entropies2
#    print entropies3    
    f1.close()

#print count0, count1, count2, count3    

res1=[float(x)/numOfFiles for x in cumulative_entropies0]
res2=[float(x)/numOfFiles for x in cumulative_entropies1]
res3=[float(x)/numOfFiles for x in cumulative_entropies2]
res4=[float(x)/numOfFiles for x in cumulative_entropies3]

print "E(0,4,8,12) :",sum(res1)/4,  "ByteWise:",res1
fd = open('setwise_SR','w')
#print >> fd,res1[0] # byte0 instead of 4/8/12
print >> fd,sum(res1)/4 # Take all 4 bytes together....This is the best measure... than taking single byte
fd.close()
#print "E(1,5,9,13) :",sum(res2)/4,  "ByteWise:",res2
#print "E(2,6,10,14):",sum(res3)/4,  "ByteWise:",res3
#print "E(3,7,11,15):",sum(res4)/4,  "ByteWise:",res4
#print
