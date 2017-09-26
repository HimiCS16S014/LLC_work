/*
 * Takes  command line arguments:
 * core number to run this AES (0-7), startpower (2power'x' encryptions), endpower , fileNameTosaveProfiledInformation, profileNum
 */

/*
    make AESProfiles

    Command line arguments: coreNumberToRun, power, fileName ,profileNum
    ./AESProfiles  0 16 17 deviationprofile x	

    For 2^16, 2^17 encryptions: create 2 files jumba_16.txt, jumba_17.txt
    
    Running the above command  will append the key and deviations of 16 bytes (total 17 lines) to the file jumba_16.txt,
    creates new file named jumba_16.txt if is already not there and writes the prior mentioned data..)
    
    These 17 lines forms one profile ...!!
    
    Next time the same command runs (with same power and filename), another 17 lines are appended...Second profile..!!

    ***Remember it every time appends...***     
    if you need do "rm jumba_16.txt" before you run AES..!!
      
 */

//change1:
//#define _GNU_SOURCE	//for CPU_SET functions and varaibels
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sched.h>
#include "aes.h"
#define TIME_THRESHOLD 2500


//change2: (uncommented)
 //extern "C" {
void print_tableAddresses();
static __inline__ unsigned long long rdtsc(void);
int mymain(int argc, char **argv);
//}

 
#if defined(__i386__)
static __inline__ unsigned long long rdtsc(void)
{
	unsigned long long int x;
	__asm__ volatile("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	__asm__ volatile("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	return x;
}
#elif defined(__x86_64__)
static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	__asm__ __volatile__ ("xorl %%eax,%%eax\n cpuid \n" ::: "%eax", "%ebx", "%ecx", "%edx"); // flush pipeline
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif  

int gotThekey=0;
unsigned char key[16]; 

void setupTheKey()
{
  printf("New key is setup..\n");
  gotThekey = 1;
  
  	
	char output[100];
	FILE *fp;
	// Open the command for reading. 
	fp = popen("openssl rand 16 | od -An -t u1", "r");
	if (fp == NULL) {
	  printf("Failed to run command\n" );
	  exit(1);
	}
	// Read the output a line at a time - output it. 
	if(fgets(output, sizeof(output)-1, fp)!=NULL)
	{	  
	  char * pch;	
	  pch = strtok (output," ");
	  int i=0;
	  //FILE* d1;
	  //d1=fopen("keys","w"); 
	  while (pch != NULL)
	  {
	    //printf ("%d %d ",i,atoi(pch));
	    key[i]=atoi(pch);
	    pch = strtok (NULL, " ,.-");
	    i++;	    	    
	  }  	  
	  /*
	  printf("KEY: ");
	  for (i = 0; i < 16; ++i)
	  {	   
	    printf("%u ",key[i]);
	    //fprintf(d1,"%u\t",key[i]);
	  }
	  printf("\n");
	  */
	  //fclose(d1);
	}
	else
	{
	  printf("something wrong1");
	}
	
  
}

//change3: mymain to main
int mymain(int argc, char **argv) {
  
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(atoi(argv[1]), &set);
	if(sched_setaffinity(0, sizeof(set), &set) < 0)
	{
	  perror("affinity error...");
	}
	
 	if(gotThekey==0)
	  setupTheKey();
		
	/* unsigned char key[16]; 
	 * char output[100];
	FILE *fp;
	// Open the command for reading. 
	fp = popen("openssl rand 16 | od -An -t u1", "r");
	if (fp == NULL) {
	  printf("Failed to run command\n" );
	  exit(1);
	}
	// Read the output a line at a time - output it. 
	if(fgets(output, sizeof(output)-1, fp)!=NULL)
	{	  
	  char * pch;	
	  pch = strtok (output," ");
	  int i=0;
	  //FILE* d1;
	  //d1=fopen("keys","w"); 
	  while (pch != NULL)
	  {
	    //printf ("%d %d ",i,atoi(pch));
	    key[i]=atoi(pch);
	    pch = strtok (NULL, " ,.-");
	    i++;	    	    
	  }  	  
	  
	  printf("KEY: ");
	  for (i = 0; i < 16; ++i)
	  {	   
	    printf("%u ",key[i]);
	    //fprintf(d1,"%u\t",key[i]);
	  }
	  printf("\n");
	  
	  //fclose(d1);
	}
	else
	{
	  printf("something wrong1");
	}
	*/
	
	unsigned char key1[16];
	unsigned char scrambled[16];
	unsigned long long int i = 0;
	unsigned int j = 0,k=0;
	float avg;
	AES_KEY expanded;
	float A[256][16] = { 0.0 };
	float C[256][16] = { 0.0 };
	unsigned long long start, end;
	float t = 0, c = 0;
	float D1[256][16];	
	
	//---------------
		
		
		int power = atoi(argv[2]);		
		int maxpower=atoi(argv[3]);
		int profileNum = atoi(argv[5]);
		//int power=24;
		char buf[25];
		FILE* d1;
		FILE* d2;
		//if((profileNum==1 )|| (profileNum % 10 ==0))
		if(profileNum==1 )
		{
		  //printf("TABLES BASE ADDRESSES: ");
		  print_tableAddresses();
		}

		AES_set_encrypt_key(key, 128, &expanded);
		srand(time(NULL));

		for (i = 1; i <= (1 << 26); i++) // for 2^28 times max; power can be less than or equal to 28
		{
			unsigned char pt[16], j;
			for (j = 0; j < 16; j++) {
				pt[j] = rand() % 256;  //instead the scrambled text of prev iteration can be set as pt this time.
			}

			start = rdtsc();
			AES_encrypt(pt, scrambled, &expanded, 0);
			end = rdtsc();

			if ((end - start) > TIME_THRESHOLD) continue;
			//if(i<16) continue; //to warm up the cache with table contents
	
			for (j = 0; j < 16; j++) {
				A[pt[j]][j] += (end - start);
				C[pt[j]][j]++;
			}
			
			if( i == (1 << power)){
				//printf("i = %llu\n", i);
				t=0;c=0;
				//power++;
				for (j = 0; j < 16; j++) {
					for (k = 0; k < 256; k++) {
						D1[k][j] = A[k][j] /  C[k][j];
						t += A[k][j];
						c += C[k][j];
					}
				}
				
				sprintf(buf,"DataFiles/%s%d_%d.txt",argv[4],profileNum, power);
				d1=fopen(buf,"w");
				sprintf(buf,"TimingFiles/%s%d_%d.txt",argv[4],profileNum, power);
				d2=fopen(buf,"w");
				avg = t / c;
				for(j=0;j<16;j++)
				{
					fprintf(d1,"%d\t",key[j]);
					fprintf(d2,"%d\t",key[j]);
					//printf("\n---------\n");
				}
				fprintf(d1,"\n");
				fprintf(d2,"\n");
				//calculating deviation vector
				for (j = 0; j < 16; j++) {
					for (k = 0; k < 256; k++) {
						fprintf(d1,"%f\t",D1[k][j]-avg);
						fprintf(d2,"%f\t",D1[k][j]);
					}
					fprintf(d1,"\n");
					fprintf(d2,"\n");								
				}
				fclose(d1);
				fclose(d2);
				
				if(power ==  maxpower)
				  return 0;
				
				power++;
			}

		}
		return 0;
}
