//Author: @ D Anthony Balaraju 	
//						Edited On: Apr 2, 2017 

//g++ --static main.cc aes_1024.c -o main -std=c++11;clear;./main

#include<signal.h>
#include<stdio.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

//change1:
#include "AESProfilesv3.c"

int pid,p; 
//int startPower=18, endpower=18, numProfiels=1;
int startPower, endpower, numProfiels=1;

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

void callAES()
{    
    //change4: added
    char  spower[4];
    sprintf(spower, "%d",startPower);
    char  epower[4];
    sprintf(epower, "%d",endpower);
    for(int j=1; j<=numProfiels;j++)
    {      
      char  proNum[4];
      sprintf(proNum, "%d",j);
     // char* argv[] = { "AESProfiles", "0", startPower ,endpower, "deviationprofile", &proNum[0], NULL};
      char* argv[] = { "AESProfiles", "0", &spower[0] ,&epower[0], "deviationprofile", &proNum[0], NULL };
      //char* argv[] = { "AESProfiles", "0", "18" ,"18", "deviationprofile", &proNum[0], NULL };
      //char* argv[] = { "dummyName", &arg0[0], &arg1[0], &arg2[0], &arg3[0], &arg4[0], NULL };
      int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
      //change5: uncommented
      mymain(argc,argv);      
    } 
}

void callPythonToEvaluate()
{
    system("rm results");
    char buffer[100];
    for(int power=startPower; power <= endpower; power++)
    {
      //python calculateEntropiesForOneLUT.py $baseIndex $numProfiles $power >> results 
      sprintf(buffer,"python calculateEntropiesForOneLUT.py 0 %d %d >> results",numProfiels, power);		    
      system(buffer);	    
      sprintf(buffer,"python ComputeAndPlotSRv1.py 16 %d 0 %d",numProfiels, power);		    
      system(buffer);	          
      //python ComputeAndPlotSRv1.py 16 $5 $baseIndex $power
    }
    system("cat results");
}


void myhandler(int signum)
{
 startPower=18;
 endpower=18;
  //change2:
  callAES();
  kill(pid, SIGUSR1);    
}

void myhandler2(int signum)
{
  //print_tableAddresses();
 // printf("\n");
  startPower=18;
 endpower=18;
  //change3:
  callAES();
  callPythonToEvaluate();
  kill(pid, SIGUSR1);    
}

int main()
{
  //change3:added
  print_tableAddresses();
  printf("\n");
  signal(SIGUSR1, myhandler);
  signal(SIGUSR2, myhandler2);      
  //p=atoi(argv[0]);
  //endpower=atoi(argv[1]);
  //evalauteScript();
  pid=std::stoi(exec("pgrep Spyv5"));
  while(1);      
  //$coreTorun $startpower $endpower deviationprofile $i
}
  
