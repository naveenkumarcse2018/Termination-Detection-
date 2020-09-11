/*
	K. Naveen Kumar
	CS19MTECH11009
	IIT Hyderabad
*/

/*
	This file contains the code for main() to start our program execution and create_process_and_socket() function to
	create process and socket for that process which will be used to communicate with other processes through the sockets.
*/



/* Including the headers files that are necessary for our program*/
#include<iostream>
#define MAX_BUFFER 512
#define Max 10000
#define DIFF_PORTS 35000
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include <cmath>
#include<vector>
#include<cmath>
#include <algorithm>
#include <ctime>





class localclock /* We are creating class for local clock here.*/
{
	/* We want make variables and methods of the class accessible from any other snippet of the program.
	   So , to give access through entire program we are making them available by declaring as public. The public
	   members are visible to all other classes. Now each process is able to access these members/variables in the program.
	*/

 	public: /* public key word used to declare "public". This is called public access modifier*/

	/* The Timeread() method is used to read the system time and getting the required format for our task. Then this function
		will return the value at the end, which is calculated using the formula given in the Assignment discription.
		This fucntion is invoked by processes (and their threads to read the current time)
	*/
	time_t Timeread() /*This function is not taking any arguments. This function just reads the time*/
	{
		struct timespec start; /* Variable to store the time value*/
		clock_gettime(CLOCK_MONOTONIC, &start); /* For obtaining time with much higher precision(nanoseconds) */
		return ((start.tv_sec*1e9) + start.tv_nsec)*1e-9 ;/* Time is read in the form of seconds and nanoseconds. So we are converting nano seconds into seconds and adding error_factor*driftFactor and we are returning this value*/
	}


};




double start_time,end_time; /*To calculate the Time taken by Termination Detection algorithm*/
using namespace std::chrono;
using namespace std;
int *number_of_messages; /*Used to count no.of messages*/
int *DD;


/*Below are some of the functions declaration , which are used in the program*/
float sleepTime(float);
void printPattern();
bool checkTTD();
bool chechParentNode(int);
void decreaseInitiator();
bool alreadySeen(int);
void callToReject(int);
bool alreadySeen(int);
void makeProcessD(int);
void chooseRandomRedProcesses();
void chooseRandomBlueProcesses();
void callDetermined();
vector<int>findRandomness(int,int);
void sendRedMessage(int,int);
void sendBlueMessage(int,int);
void redNeighbours(int);
void blueNeighbours(int);
char *timeTaken();



int process_init=-1; /*Used to know about the initiator node */
bool delayTime=true;/* used to  introduce blue messages */
int d_sent=0;/* used to  introduce blue messages */

pthread_cond_t conditioned_thread = PTHREAD_COND_INITIALIZER; /*Related to threads*/
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER; /**/

int process_done = 0; /* Used in the process barrier concept (function)*/


vector<vector<int>> adjacencyMatrix,spanningTree; /* Stores adjacent neighbours information */

/*Below information is used in the computation */
int Ir ,  Ib ,n;
float Wr,Wb, lamda_red, lamda_blue, p , q ,lamda_send;




/*
	K. Naveen Kumar
	CS19MTECH11009
	DC Programming Assignment-I
	IIT Hyderabad
*/


/*
  The process class will have send and receive function and other functions that need to be used for computation
  Each process will have all function of Process , which will be executed using threads for each process.

*/
using namespace std;
class Process
{

  public:
  int socket_file_discripter;/*Socket discriptor for process */

  thread receive_t; /* Each process should maintain two threads. one for local clock and another for receive*/

  int pid; /*Process identifier. Which will be initialized when the process constructor is created.*/

  bool flag = false; /*Used for wait*/

  char color[10];

  int C,D,parentNode;
  int minMsg;


  Process(int id)
      {

          receive_t =  thread(&Process :: receive,this); /*Thread for receive function */
          pid = id;/*Assign process id to pid variable of the process class*/
      		strcpy(color,"white"); // Initial color of process

          //Below code shows initial Assignment of each process
      		C=0;D=0;
      		parentNode=id;
          minMsg=2;


      		/* Detaches the thread represented by the object from the calling thread, allowing them to execute independently from each other*/
      		receive_t.detach();
      }


      /*Send function is used to send messages between processes. This function take source, destination and messages
      as arguments and sends the message to destination process using sockets.

      */
      void send(int source,int destination,char *message)
      {
            if(source<n+1&&destination<n+1)
            {
            struct sockaddr_in peeraddr;  /*Now socket address to the peer to whome the process want to send the message*/
            peeraddr.sin_family    = AF_INET; /*We use IPV4 protocol*/
            peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
            peeraddr.sin_port = htons(destination+DIFF_PORTS); /*Port for destination process*/

            char *tt=timeTaken();

            if(!strncmp(message,"ack",3)){printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,"ack",destination+1,tt );}
            else printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,color,destination+1,tt );
            sendto(socket_file_discripter, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &peeraddr,sizeof(peeraddr));


            if(delayTime&&d_sent==4) // We woke up blue messages after some time. If this is true then we wake up blue processes
            {

              delayTime=false;
              d_sent=12;
              chooseRandomBlueProcesses(); //Choosing random blue processes
              start_time=clock();

            }
            d_sent+=1;
          }
          
      }


/*
  This function is responsible for the receiving a message from the sender.
  We perform some computations after receiving the message.

*/
 void receive()
 {
      while(true)
      {
        char *message_received = new char[MAX_BUFFER];  /*Declaring message receive buffer*/
        unsigned int length, n,round_number; /*Variables declaration*/
        char garbage[8];

        struct sockaddr_in peeraddr;  /*socket addresses for  peer*/

        length = sizeof(peeraddr);  /*Storing the size of peeradd in length*/
        peeraddr.sin_family    = AF_INET; /*Using IPV4*/
        peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /*Local host address*/
        n = recvfrom(socket_file_discripter, (char *)message_received, MAX_BUFFER,MSG_WAITALL, ( struct sockaddr *) &peeraddr,&length);
        message_received[n] = '\0';  /*Appending null character at the end of the message*/

        

        if(!strncmp(message_received,"red",3)) // If received message is red
        {
          float tq=sleepTime(Wr);
          sleep(tq*2);
            if(C>0) // This condition is used to send signal message to neighbours
            {
                  strcpy(color,"red"); // We send red message to neighbhours only if C>0
                  char *ss=timeTaken();
                  printf("Cell %d receives red message from Cell %d at %s\n\n",pid+1,ntohs(peeraddr.sin_port)-DIFF_PORTS+1,ss);
                  printf("Cell %d turns red at %s\n\n",pid+1,ss);
                  if(pid!=process_init) redNeighbours(pid);
            }
            else if(C==0&&D==0) // If new node receives red message then we change its color and C
            {

                int pd=ntohs(peeraddr.sin_port)-DIFF_PORTS;
                if(!chechParentNode(pd))
                {
                strcpy(color,"red");
                C=1;
                D=0;
                parentNode=ntohs(peeraddr.sin_port)-DIFF_PORTS;
                showRedMessage(pid,ntohs(peeraddr.sin_port)-DIFF_PORTS); // Displays message received at which time
                makeProcessD(ntohs(peeraddr.sin_port)-DIFF_PORTS); //Increments sender's D
                redNeighbours(pid); //forward red message to neighours
              }
            }
        }
        else if(!strncmp(message_received,"blue",4)) // If received message is blue
        {

          strcpy(color,"blue");
          if(pid==process_init) //If initiator receive blue message
          {


            decreaseInitiator(); //We decrement initiators D
            *DD=*DD-1;
            continue;
          }
            if(C==0&&D==0) //If node with C==0 and D==0 receives blue msg, it just forwards it to neighbhours
            {
                C=0;
                D=0;
                parentNode=pid;

                strcpy(color,"blue");
                showBlueMessage(pid,ntohs(peeraddr.sin_port)-DIFF_PORTS);
                if(minMsg!=0) blueNeighbours(pid);
            }
            else if(C==1&&D==0)// If node's C==1 and D==0 receive blue msg then it should send sck msg to its parent
            {
                *number_of_messages+=1;
                strcpy(color,"blue");
                char message[10];
                strcpy(message,"ack");

                if (chechParentNode(pid)) continue;
                int dd=parentNode;
                parentNode=pid;
                C=0;
                D=0;

                struct sockaddr_in peeraddr;  /*Now socket address to the peer to whome the process want to send the message*/
                peeraddr.sin_family    = AF_INET; /*We use IPV4 protocol*/
                peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
                peeraddr.sin_port = htons(dd+DIFF_PORTS);
                char *tt=timeTaken();
                if(!strncmp(message,"ack",3)) printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,"ack",dd+1,tt );
                else printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,color,dd+1,tt );
                sendto(socket_file_discripter, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &peeraddr,sizeof(peeraddr));
                //Sending ack msg to parent here
               // blueNeighbours(pid);
            }
            else
            {
                  //Spread this message to it's neighbours
                  strcpy(color,"blue");
                  blueNeighbours(pid);
            }

        }
        else if(!strncmp(message_received,"ack",3)) //If process receives ack msg
        {

              strcpy(color,"blue");
              if(pid==process_init) //If process is initiator
              {

                decreaseInitiator(); //Decrements D of initiator
                *DD=*DD-1;

                continue;
              }
              if(C==1&&D>0) //We just decrese the value of D by 1, if we have this condition
              {
                printf("Cell %d gets blue message from Cell %d\n\n",pid+1,ntohs(peeraddr.sin_port)-DIFF_PORTS+1 );
                 D=D-1;


                if(D==0) //After decreaseing the D value if it becomes 0 then we send ack msg to its parent
                {
                  *number_of_messages+=1;
                  char message[10];
                  strcpy(message,"ack");

                  int dd=parentNode; //we send ack msg to parent
                  parentNode=pid;
                  C=0;
                  D=0;
                  struct sockaddr_in peeraddr;  /*Now socket address to the peer to whome the process want to send the message*/
                  peeraddr.sin_family    = AF_INET; /*We use IPV4 protocol*/
                  peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
                  peeraddr.sin_port = htons(dd+DIFF_PORTS);
                  char *tt=timeTaken();
                  if(!strncmp(message,"ack",3)) printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,"blue",dd+1,tt );
                  else printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,color,dd+1,tt );
                  sendto(socket_file_discripter, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &peeraddr,sizeof(peeraddr));
                 // blueNeighbours(pid);

                }
                else blueNeighbours(pid);
              }
              else if(D==0)
              {
                  if(C==0){ //If D==0 and C==0 we just forward blue msg
                  parentNode=pid;
                  strcpy(color,"blue");
                  showBlueMessage(pid,ntohs(peeraddr.sin_port)-DIFF_PORTS);

                  if(minMsg!=0) blueNeighbours(pid);
                  }
                  else if(C==1) //D==0 and C==1 then send ack msg to its parent
                  {
                      *number_of_messages+=1;
                      int dd=parentNode;
                      parentNode=pid;

                      C=0;
                      D=0;
                      strcpy(color,"blue");
                      char message[100];
                      strcpy(message,"ack");
                      //char *ttt=timeTaken();
                      struct sockaddr_in peeraddr;  /*Now socket address to the peer to whome the process want to send the message*/
                      peeraddr.sin_family    = AF_INET; /*We use IPV4 protocol*/
                      peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
                      peeraddr.sin_port = htons(dd+DIFF_PORTS);
                      char *tt=timeTaken();
                      if(!strncmp(message,"ack",3)) printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,"blue",dd+1,tt );
                      else printf("Cell %d sends %s message to Cell %d at %s\n\n",pid+1,color,dd+1,tt );
                      sendto(socket_file_discripter, (const char *)message, strlen(message),MSG_CONFIRM, (const struct sockaddr *) &peeraddr,sizeof(peeraddr));
                     
                  }

              }

        }
        else
        {

          printf(" Other message than blue , red and ack\n");

        }




      }

      if(checkTTD()) exit(0);

 }


/*
  This function displays a message which says receiving of red message and process turns to red

*/
void showRedMessage(int s,int d)
{
  char *tt=timeTaken();
  printf("Cell %d receives red message from Cell %d at %s\n\n",s+1,d+1,tt );
  tt=timeTaken();
  printf("Cell %d turns  red at %s\n\n",s+1,tt);


}

/*
  This function displays a message which says receiving of blue message and process turns to blue

*/
void showBlueMessage(int s,int d)
{
  char *tt=timeTaken();
  printf("Cell %d receives blue message from Cell %d at %s\n\n",s+1,d+1,tt );
  tt=timeTaken();
  printf("Cell %d turns  blue at %s\n\n",s+1,tt);
}



/*
  The below snippet of line shows the barrier synchronization of the process. We use this synchronization because
  process after creating the socket, it has to wait untill all other processes to create their sockets. So that they
  can communicate with each other. If we don't use this type synchronization, then process will send/receive messages to
  the processes which haven't created their sockets(No means of communication then). So, we use this barrier synchronizations.
*/

 void process_Synchronization_Barrier()
	{
		pthread_mutex_lock(&locker);/*all the processes should bind to socket before they start communicating each other*/
		process_done ++; /*Each process increments it if they bind the socket*/

		if(process_done == n) /*If all the processes are done with binding of the sockets then we release the lock and wake up the threads*/
		{
			for(int i=1;i<=process_done;i++) /*Here waking up the threads*/
				pthread_cond_signal(&conditioned_thread);  /*This concept is used from operating systems subject for synchronization*/
		}

		else /*If not all processes completed the binding then wait untill they do */
		{
		pthread_cond_wait(&conditioned_thread, &locker);  /*Waiting */
		}

		pthread_mutex_unlock(&locker);/*Do this after all processes completes the binding of socekts*/
		process_done = 0;
	}

  //This function shows C and D values of process
  void showDC()
  {
    printf("Process %d has C=%d and D=%d values\n",pid+1,C,D );
  }




  /*
		This socket_create() function creates socket for process.
		The port assigned to the socket is obtained from the process id.(i.e DIFF_PORT+processID)
		Each process will invoke this function to create socket for that process.
	*/
  int socket_create()
	{
		int  PORT = DIFF_PORTS+ pid; /* Making port number with process id. Here DIFF_PORTS=35000 as predefined */

  //  printf("Process %d is Port with %d\n",pid+1,PORT );
    struct sockaddr_in myaddr; /* Socket address variable*/


		/*
			In below if condition we used socket() function to create socket and the returned discriptor is stored in the
			socket_file_discripter. If socket() function returns the value less than 0 then socket is not created. i.e error
			occured while creating the socket.
		*/
		if ( (socket_file_discripter = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 )
		{
			printf("Socket is not created for process %d \n",pid+1);
			return -1;
		}



		/*
			After creating the socket we need to bind it with address and port number to get identified.
			Below snippet of code is doing this task.
		*/
		myaddr.sin_family    = AF_INET; // IPv4
		myaddr.sin_addr.s_addr = INADDR_ANY; /*Addressss*/
		myaddr.sin_port = htons(PORT);  /*Port number*/


		/*
			We use bind() function to bind the address and port number
		*/
		int status = bind(socket_file_discripter, (const struct sockaddr *)&myaddr,sizeof(myaddr))	;

		if (status < 0 ) /*If returned value from the bind() is negative then we can conclude that error occured during the binding process*/
		{
			printf("Binding error id occurred for process %d \n", myaddr.sin_port-DIFF_PORTS);
			perror("bind failed");
			exit(EXIT_FAILURE);
			return -1; //failed to associate particular port number with this process....
		}

		/*
			If we successfully bind the address then we can declare that socket creation for the process is completed successfully.
		*/
	if(pid!=n)	printf("Socket Created Successfully for process %d \n", pid+1);;
		//cout<<color<<endl;

		return 1;
	}

};



Process **P; /*Used to store processes details*/
Process *initiator_p; /*used to store initiator process details*/





/*	The create_process_and_socket() function is responsible for creating the process and socket for that process.  This
	function takes one argument as pid (which represents process id).
*/


void create_process_and_socket(int pid){

	if(pid<=n){

	/*Creating Constructor for the class ProcessClass. The Constructor will take one argument as process id.
	  As we are using threads, n processes will be created using different constructors in different threads.
	*/

	P[pid]=new Process(pid);
	int status = P[pid]->socket_create(); /* Calling socket creation function in Process Class*/

	P[pid]->process_Synchronization_Barrier(); /* Calling this function for synchronization of processes*/

	if(status<0) /*If socket creation fails*/
	{
		printf("Socket creation is failed\n");
		return ;
	}

	}
}


/*
	This function is used to split the given argumented string based on the delieter. When we read input from the text file , We need to break down
	the line to get required information.


*/


vector<string> split(string str, char delimiter)
{
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal; /*Vector of strings*/
}


int main(int argc,char **argv)
{


		  number_of_messages=(int *)malloc(sizeof(int)); /*allocating memory */

		  /*
				Below peice of code will read the text file and gets information needed for the computation.
				such as no.of processes, Adjacency information etc.

	
	  */

          fstream newfile;
          string tp;
          vector<string> sep;
          std::vector<int> neighbours;

          char filepath[100];
          strcpy(filepath,argv[1]);

          

          
          newfile.open(filepath,ios::in); //open a file to perform read operation using file object
          int i=0, l=0,newline=0;
          while(getline(newfile, tp))
          {
          sep= split(tp, ' ');

          if(l==0){
          /*Very first line contains information about n,Wr,Ir,WB,Ib, p,q ,lambda_red,lamda_blue etc */
          n=stoi(sep[0]);
          Wr=stof(sep[1]);
          Ir=stoi(sep[2]);
          Wb=stof(sep[3]);
          Ib=stoi(sep[4]);
          lamda_blue=stof(sep[5]);
          lamda_red=stof(sep[6]);
          lamda_send=stof(sep[7]);
          p=stof(sep[8]);
          q=stof(sep[9]);
            l=12;
          }

          else{	/*from second line onwards we will have adjacency information. So we read the information and store them in adjacencyMatrix*/

             if(i<n)
             {
                for(int j=0;j<sep.size();j++)
                {
                  int ii=stoi(sep[j]);
                  neighbours.push_back(ii);
                }
                adjacencyMatrix.push_back(neighbours); /*Storing in the adjacency matrix*/
                neighbours.clear();
                i++;
             }
             else{

             	if(newline==0){
             		newline=1;
             		continue;
             	}
             	else
				        {
						for(int j=0;j<sep.size();j++)
		                {
		                  int ii=stoi(sep[j]);
		                  neighbours.push_back(ii);
		                }
		                spanningTree.push_back(neighbours); /*Storing in the adjacency matrix*/
		                neighbours.clear();
             	}
             }
             
          	}
          }
          newfile.close();  /*Closing the file*/



/*
          std::cout << n << '\t';
		  std::cout << Wr << '\t';
		  std::cout << Ir << '\t';
		  std::cout << Wb << '\t';
		  std::cout << Ib << '\t';


		  std::cout << lamda_blue << '\t';
		  std::cout << lamda_red << '\t';
		  std::cout << lamda_send<< '\t';
		  std::cout << p << '\t';
		  std::cout << q << '\t';
          cout<<endl;
		  //std::cout << "Naveen here "<<adjacencyMatrix[0][2] << '\n';
		 for(int i=0;i<n;i++){
		    int size=adjacencyMatrix[i].size();
		    //std::cout << " Size in for loop "<<size << '\n';
		    for(int j=0;j<size;j++){
		      cout<<adjacencyMatrix[i][j]<<"\t";
		    }
		    cout<<endl;
		  }
		 

		  for(int i=0;i<n;i++){
		    int size=spanningTree[i].size();
		    //std::cout << " Size in for loop "<<size << '\n';
		    for(int j=0;j<size;j++){
		      cout<<spanningTree[i][j]<<"\t";
		    }
		    cout<<endl;
		  }
   
          return 0; */
                sleep(1);

          P=new Process*[n]; /*N processes objects*/
          thread threads[n]; /* Threads*/

          for(i=0;i<n;i++) /*Creating thread of each process. 'n' threads for 'n' processes*/
            threads[i] =  thread(create_process_and_socket,i); /*thread takes two arguments, 1. routine/function address 2. process id. Each thread will execute create_process_and_socket function. So, that means each process is mainted by the individual threads */

          for(i=0;i<n;i++)
            threads[i].join(); /* Joining the threads so that main will wait until all processes complete their execution*/

          srand((unsigned) time(0));

      	  cout<<endl;
      	  /*Below snippet of code will create initiator node*/
          initiator_p=new Process(n);
          initiator_p->socket_create();
					strcpy(initiator_p->color,"red");
					initiator_p->C=0;
					initiator_p->D=Ir;
          initiator_p->parentNode=n;

		  DD=(int *)malloc(sizeof(int));
		  *DD=Ir;
		  process_init=n;

		  float ff=sleepTime(Wr); /*Sleep exponential time with avg Wr before introducing the red messages*/
		  sleep(Wr);

          chooseRandomRedProcesses(); /* Choosing some random red  processes */

		  while(initiator_p->D!=0); /*Condition for termination */
		  end_time=clock();/*End clock time */

		  double total=(end_time-start_time)/double(CLOCKS_PER_SEC);
		  sleep(10);
		  std::cout << "\n Time taken by * Dijkstra Termination Detection * Algorithm is: " <<total<<"\n";

		  printPattern(); /*Prints related information after termination dection*/
}



/*
	This function computes the exponential with avg given by the argument and returns the result.

*/
float sleepTime(float a)
{
  double reqSleepTime;
  reqSleepTime = rand() / (double)(RAND_MAX);
  reqSleepTime=-log(reqSleepTime) / a;
  return reqSleepTime;
}




/*
	The chooseRandomRedProcesses() chooses some random process between 1 and n. It turns selected processes color to red and
	spreads the red color to its neighbhour.


*/

void chooseRandomRedProcesses()
{
	float t=sleepTime(Wr);
	sleep(2*t);

	std::vector<int> v=findRandomness(Ir,n); /* Finding random processes */


		char *stringTime=timeTaken();/* Time read from the system clock*/

		for(int i=0;i<Ir;i++)
		{
			stringTime=timeTaken();

			strcpy(P[v[i]]->color,"red"); /*Changing color to red */
			P[v[i]]->parentNode=initiator_p->pid; /*Making parent as initiator for dynamic tree construction in Dijkstra algorithm*/
			P[v[i]]->C=1; /* C, D are variables used for Dijkstra-Schelton TD algorithm*/
			P[v[i]]->D=0;
			*number_of_messages+=1; /*Counting the no.of messages */
			printf("Cell %d turns red at %s\n\n",v[i]+1,stringTime );
		}

		/*Below snippet of code will spread/forward red color to chosen processes's neighbours*/
		for(int i=0;i<v.size();i++)
		{
				int r=v[i];
				int adj=adjacencyMatrix[r].size();
				for(int j=1;j<adj;j++)
				{
					int in=adjacencyMatrix[r][j]-1;/*Adjacent node*/
					sendRedMessage(r,in); /*Sending red message to adjacent node*/
				}
		}

}




/*
	The chooseRandomBlueProcesses() chooses some  random process between 1 and n. It turns selected processes color to blue and
	spreads the blue color to its neighbhour.
*/

void chooseRandomBlueProcesses()
{
	float t=sleepTime(Wb);
	sleep(2*t);
	std::vector<int> v=findRandomness(Ib,n); /*Selecting random processes */

	char *stringTime=timeTaken();

		for(int i=0;i<Ib;i++)
		{
			stringTime=timeTaken();/*System time read*/

			if(P[v[i]]->D==0)
			{
				strcpy(P[v[i]]->color,"blue");
				printf("Cell %d turns blue at %s\n\n",v[i]+1,stringTime );
			}
			else
			{
					if((v[i]+1)!=n)
					{
						v[i]=v[i]+1;
						i--;
					}
			}
		}


	/* Below snippet of code will forward blue message to selected process's neighbours */

		for(int i=0;i<v.size();i++)
		{
				int r=v[i];
				int adj=adjacencyMatrix[r].size(); /**/
				for(int j=1;j<adj;j++)
				{

					int in=adjacencyMatrix[r][j]-1; /*neighbours node*/
					sendBlueMessage(r,in);
				}
		}

}


/*
	This findRandomness() function's computation will choose some random processes between given range(1 to n_adj_size).
	This function will produce no.of random processes as given size (size_subset).
	This function will return vector contain randomly selected processes.

*/

vector<int>findRandomness(int size_subset,int n_adj_size) //returns subset of indices of size size_subset range between 0 to n_adj_size
{
	//size_subset is no.of indices
	//n_adj_size is no.of neighbours

	int i=0,flag=1;
	vector<int>v;
	while(i<size_subset)
	{
		int index=rand()%n_adj_size; /* Generating random numbers*/
		if(index!=0){
		if(i==0){
				v.push_back(index);
		}
		else{
				flag=0;
				do{
					flag=0;

				if(find(v.begin(), v.end(), index) != v.end()&&index!=0) // Generated number should not be present in previously generated numbers
				{
					flag=1;
					index=rand()%n_adj_size;
				}
				else
				{
						v.push_back(index);
						flag=0;
				}
				}while(flag);
		}
		i++;

		}
	}

	return v;
}



/*
	This function takes two arguments and sends red message to destination process.
*/

void sendRedMessage(int s,int d) // Taking two argument as source and destination
{

  if(s!=d&&s<n+1&&d<n+1)
    {
      char message[MAX_BUFFER];
      sprintf(message,"%s %d\n","red",s	);
      if(P[s]->C>0)
			{

				P[s]->send(s,d,message); //Calling send() function
			}
    }
}



/*
	This function takes two arguments and sends blue message to destination process.
*/
void sendBlueMessage(int s,int d)
{

  if(s!=d&&s<n+1&&d<n+1)
    {
      char message[MAX_BUFFER];
      sprintf(message,"%s %d\n","blue",s	);
      P[s]->send(s,d,message); //Calling send() function in Process class
    }
}



/*
	This function will read the time from the local system and then truncate the taken time to get required representation of the time.
	This function will return stringTime

*/

char * timeTaken()
{
    localclock local_clock;
    struct tm * time_information; /* Structure containing a calendar date and time broken down into its components*/
    time_t system_time;/* time type variable, which stores the local_clock time from the tableValues*/
    time_information; /*Converts to local time*/
    char *stringTime;/*Convert tm structure to string*/
    system_time=local_clock.Timeread();
    time ( &system_time );
    time_information = localtime ( &system_time );
    stringTime= asctime(time_information);
    stringTime[19]='\0';
    stringTime=&stringTime[11];

    return stringTime;

}




/*
	This redNeighbours() function takes one argument and then sends red message to neighbours of the process that has taken as argument.
*/
void redNeighbours(int s)
{
	int n_size=ceil(adjacencyMatrix[s].size()*p);// p% of adjacent neighbours

	std::vector<int> v=findRandomness(n_size,adjacencyMatrix[s].size()); // Finding the random neighbours

	for(int i=0;i<v.size();i++)
	{
		float tt=sleepTime(Wr);
		sleep(tt);
		if(s<n+1&&adjacencyMatrix[s][v[i]]-1<n+1) sendRedMessage(s,adjacencyMatrix[s][v[i]]-1); // Sending message to neighbours
	}

}


/*
	This blueNeighbours() function takes one argument and then sends blue message to neighbours of the process that has taken as argument.
*/
void blueNeighbours(int s)
{
	int n_size=ceil(adjacencyMatrix[s].size()*q);// q% of adjacent neighbours

	std::vector<int> v=findRandomness(n_size,adjacencyMatrix[s].size()); // Finding random neighours
	for(int i=0;i<v.size();i++)
	{
		float tt=sleepTime(Wb);
		sleep(tt);
		if(s<n+1&&adjacencyMatrix[s][v[i]]-1<n+1)  sendBlueMessage(s,adjacencyMatrix[s][v[i]]-1); //sending blue message to neighbours
	}
}


/* Increments D of process , this function takes one argument as pid of process*/
void makeProcessD(int s)
{
	if(s!=process_init) P[s]->D+=1;
}

/* When ever initiator gets ack message then we use this function to decrement D of initiator*/
void decreaseInitiator()
{
	//printf("Came to decreaseInitiator function %d is D and *D=%d\tcount= %d\n",initiator_p->D,*DD,*number_of_messages );
	if(initiator_p->D>0) initiator_p->D-=1;
}

/*This function is used to check whether the initiator D is empty or not*/
bool checkTTD()
{
	if(initiator_p->D==0)
	{

		return true;
	}
	return false;
}

/*This function is used to check whether the parent node is itself or not*/
bool chechParentNode(int id)
{
	if(P[id]->parentNode==id) return true;
	return false;
}


/*After termination detection , we use this function to print the table which shows the details of the processes*/
void printPattern()
{
	printf("\npid\tcolor\tC\tD\tparentNode\n");
	for(int i=0;i<n;i++)
	{
		printf("%d\t%s\t%d\t%d\t%d\n\n",P[i]->pid+1,P[i]->color,P[i]->C,P[i]->D,P[i]->parentNode+1);
	}

	printf("%d is number of messages sent to detect Termination\n\n",*number_of_messages );

}
