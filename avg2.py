import linecache
import re
for power in range(15,17, 1):	
    for length in range(1,11, 1):			      
		filename = 'Testing_'+str(power)+'_'+str(length)
		fd1 =open('Avg2','a')
		print filename
		print >> fd1, filename
		fd = open(filename,'r')
		numsum=0.0
		avg_y=0
		avg_n=0
		count=0
		count_y=0
		count_n=0
		arr_y= list()
		arr_n= list()
		temp= list()

		for line in fd:
		    count=count+1
		    if count > 33:
			temp=line.split()
			#print temp
			#print temp[10]
			if temp[5]=='Y' :
			    arr_y.append(temp[10])
			    count_y=count_y+1
			if temp[5]=='N' :
			    arr_n.append(temp[10])
			    count_n=count_n+1


		#print arr_n
		if count_y==0:
		    print "No Y found"
		    print >> fd1, "No Y found"
		else:
		    numsum = sum(map(float,arr_y))
		    avg_y=numsum/count_y
		    print "avg_y=", avg_y
		    print >> fd1, "avg_y=", avg_y	    
		if count_n==0:
		    print "No N found"
		    print >> fd1, "No N found"
		else:
		    numsum = sum(map(float,arr_n))
		    avg_n=numsum/count_n
		    print "avg_n=", avg_n
		    print >> fd1, "avg_n=", avg_n
		print "-----------------------------------------"
		print >> fd1,"-----------------------------------------"


	

 














