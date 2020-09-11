/*
	K. Naveen Kumar
	CS19MTECH11009
	DC Programming Assignment-II
	IIT Hyderabad
*/




/*Including required files, which will help us in this program*/
#include <iostream>
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
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>


/*This class is used to required read local system clock*/
class localclock
{



	/* We want make variables and methods of the class accessible from any other snippet of the program.
	   So , to give access through entire program we are making them available by declaring as public. The public
	   members are visible to all other classes. Now each process is able to access these members/variables in the program.
	*/
public:
	time_t Timeread() /*This function is not taking any arguments. This function just reads the time*/
	{
		struct timespec start;								  /* Variable to store the time value*/
		clock_gettime(CLOCK_MONOTONIC, &start);				  /* For obtaining time with much higher precision(nanoseconds) */
		return ((start.tv_sec * 1e9) + start.tv_nsec) * 1e-9; /* Time is read in the form of seconds and nanoseconds. So we are converting nano seconds into seconds and adding error_factor*driftFactor and we are returning this value*/
	}
};


/*Below mentioned are function declarations and global variables which will be used in  main functions
	and processes class which will be accessible to every function in it.
	*/
bool _flag=true;

void makeParentToken(int);
bool termination_detected = false;
double start_time, end_time;
using namespace std::chrono;
using namespace std;
int *number_of_messages;
int non_leaves;
int *DD;
bool areChildrenTerminated(int);
void reinitiateTD();
void callPrint();
float sleepTime(float);



void setRootNode();
bool checkBlackToken();
bool alreadySeen(int);
void callToReject(int);
bool alreadySeen(int);
void chooseRandomRedProcesses();
void chooseRandomBlueProcesses();
void callDetermined();
vector<int> findRandomness(int, int);
void sendRedMessage(int, int);
void sendBlueMessage(int, int);
void redNeighbours(int);
void blueNeighbours(int);
char *timeTaken();
bool terminationDetected();
int process_init = -1;
//int *socket_file_discripter;
bool delayTime = true;
int d_sent = 0;
pthread_cond_t conditioned_thread = PTHREAD_COND_INITIALIZER; /**/
pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;			  /**/
int process_done = 0;


/*
	Below declared variables are used to store input details
*/
vector<vector<int>> adjacencyMatrix, spanningTree;
int Ir, Ib, n;
float Wr, Wb, lamda_red, lamda_blue, p, q,lamda_send;



/* This process class will be used for each process, which holds send,receive and other functions
	 which will be used for each process. We will create objects for this class to make use of them.
*/
class Process
{

public:
  int socket_file_discripter; /*Socket discriptor for process */

  thread receive_t; /* Each process should maintain two threads. one for local clock and another for receive*/

  int pid; /*Process identifier. Which will be initialized when the process constructor is created.*/

  bool flag = false; /*Used for wait*/

  char color[10], tokenColor[15]; /*Used for process color and token color*/

  int parentNode; /*Used to store parent node*/
  int minMsg;

  bool hasToken, isLeaf; /*Boolean values for whether a node is leaf of not and a node has token or not*/
  int no_of_children;/*Which shows no.of remaining children need to submit the token to parent (for each node we will have these)*/
  bool reported, tokenSeen;

  Process(int id)
  {
    /*Below snippet of code is used to initialize each process with default values */
    receive_t = thread(&Process ::receive, this);//Thread for receive function
    pid = id; /*Assign process id to pid variable of the process class*/
    strcpy(color, "white");
    parentNode = -1;
    hasToken = false;
    isLeaf = false;
    no_of_children = 0;
    reported = false;
    tokenSeen = false;
    strcpy(tokenColor, "white");
    receive_t.detach();
  }

  ~Process()
  {
    close(socket_file_discripter); //Closing socket
  }




	/*Send function is used to send messages between processes. This function take source, destination and messages
		 as arguments and sends the message to destination process using sockets.

		 */
  void send(int source, int destination, char *message)
  {

    if(checkBlackToken()) //If root node has black token then we need to re-initialize the TD algorithm
    {

      reinitiateTD();
      exit(0);
    }

   
    char *tt = timeTaken();

    struct sockaddr_in peeraddr;                       /*Now socket address to the peer to whome the process want to send the message*/
    peeraddr.sin_family = AF_INET;                     /*We use IPV4 protocol*/
    peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
    peeraddr.sin_port = htons(destination + DIFF_PORTS); /*Port number*/
    printf("Cell %d sends %s message to Cell %d at %s\n\n", pid + 1, message, destination + 1, tt);
    if (!reported && hasToken&&(strcmp(color,"red")==0))
    {
      *number_of_messages += 1;
      hasToken=true;
      strcpy(tokenColor, "black");

    }
    sendto(socket_file_discripter, (const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&peeraddr, sizeof(peeraddr));

    if (delayTime && d_sent == 4)
    {

      delayTime = false;
      d_sent = 12;
      chooseRandomBlueProcesses(); //Introducing blue processes after some time
      start_time = clock();
    }
    d_sent += 1;
  }




	/*
This function is responsible for the receiving a message from the sender.
We perform some computations after receiving the message.

*/

  void receive()
  {

    while (1)
    {
      char *message_received = new char[MAX_BUFFER]; /*Declaring message receive buffer*/
      unsigned int length, n, round_number;          /*Variables declaration*/
      char garbage[8];

      struct sockaddr_in peeraddr; /*socket addresses for  peer*/

      length = sizeof(peeraddr);                         /*Storing the size of peeradd in length*/
      peeraddr.sin_family = AF_INET;                     /*Using IPV4*/
      peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /*Local host address*/
      n = recvfrom(socket_file_discripter, (char *)message_received, MAX_BUFFER, MSG_WAITALL, (struct sockaddr *)&peeraddr, &length);
      message_received[n] = '\0'; /*Appending null character at the end of the message*/

      if (n < 0) //If we read nothing
        continue;

      if (!strncmp(message_received, "red", 3)) //If we receive red message
      {


        if (!reported)
        {
          if (strcmp(color, "red") != 0)
          {
            strcpy(color, "red"); //Changing process's color
            int sender = ntohs(peeraddr.sin_port) - DIFF_PORTS; //Getting sender's port number
            char *tt = timeTaken();
            printf("Cell %d received red message from Cell %d at %s\n\n", pid + 1, sender + 1, tt);
            tt = timeTaken();
            printf("Cell %d turns red at %s\n\n", pid + 1, tt);
            redNeighbours(pid);//forwarding the red color message to its neighours
          }

        }
        else
        {
          strcpy(tokenColor, "white");
          blueNeighbours(pid);
        }
      }
      else if (!strncmp(message_received, "blue", 4))//If we receive blue message
      {

        strcpy(color, "blue");//Changing color to blue


        if(areChildrenTerminated(0))// We are checking whether root node's children are terminated or not
				{
          setRootNode();
          terminationDetected();// Termination has occured so we display the details of all the processes
          exit(0);
        }


        int sender = ntohs(peeraddr.sin_port) - DIFF_PORTS;

        char *tt = timeTaken();

        printf("Cell %d received blue message from Cell %d at %s\n\n", pid + 1, sender + 1, tt);
        tt = timeTaken();
        printf("Cell %d turns blue at %s\n\n", pid + 1, tt);
        strcpy(color, "blue");
        //callPrint();

        if (hasToken)
        {
          if (no_of_children == 0)
          {
            if (pid == 0)//Checking condition for void terminatation of the TD algorithm
            {
              if (strcmp(tokenColor, "black") == 0)
              {

                process_done = 0;

                reinitiateTD();
                return;
              }
              terminationDetected();
              return;
            }


						//If process has token and all its children has terminated then it sends token to its parent
            char token[100];
            strcpy(token, tokenColor);
            int parent = parentNode;
            hasToken = false;
            strcpy(tokenColor, "white"); // We don't know about the previous color of the token

            makeParentToken(parent);
            //Directly send token to parentNode
            struct sockaddr_in peeraddr;                       /*Now socket address to the peer to whome the process want to send the message*/
            peeraddr.sin_family = AF_INET;                     /*We use IPV4 protocol*/
            peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
            peeraddr.sin_port = htons(parent + DIFF_PORTS); /*Port number of destination*/
            char *tt = timeTaken();
            reported = true;
            tokenSeen = false;
            printf("Cell %d sends %s token  to Cell %d at %s\n\n", pid + 1, token, parent + 1, tt);
            sendto(socket_file_discripter, (const char *)token, strlen(token), MSG_CONFIRM, (const struct sockaddr *)&peeraddr, sizeof(peeraddr));

          }

          else
          {
            //checking whether all the children of a node are terminated or not
            if(areChildrenTerminated(pid))
            {
                strcpy(color,"blue");
                strcpy(tokenColor,"white");
                hasToken=false;
                no_of_children=0;
            }
            blueNeighbours(pid);
          }


        }

        else
        {
          blueNeighbours(pid);
        }
      }
      else if (!strncmp(message_received, "black", 5))//If black token is received
      {
        *number_of_messages += 1;
        hasToken=true;
        if (pid == 0) //If root node receives black token then we need to re-initiate TD algorithm
        {

          process_done = 0;

          reinitiateTD();
          exit(0);
        }

				//If process is not root node then we change color of token
        strcpy(tokenColor, "black");

        int sender = ntohs(peeraddr.sin_port) - DIFF_PORTS;//Getting sender's port number here

        char *tt = timeTaken();
        printf("Cell %d received %s token from Cell %d at %s\n\n", pid + 1, message_received, sender + 1, tt);

        if (hasToken == false)
          hasToken = true; //Because we have received the token from one of its child

        if (no_of_children > 0)//decreasing no_of_children by 1, because this process got token from one of its chilndren
          no_of_children -= 1;


        if (pid!=0&&no_of_children == 0 )//Checking for termination of children of a process
        {
					//If all children are terminated then we send token to its parent
          char token[100];
          strcpy(token, tokenColor);//Getting token
          int parent = parentNode;//Getting parent node id
          hasToken = false;
          tokenSeen = false;

          strcpy(tokenColor, "white"); // We don't know about the previous color of the token
           makeParentToken(parent);
          //Directly send token to parentNode
          struct sockaddr_in peeraddr;                       /*Now socket address to the peer to whome the process want to send the message*/
          peeraddr.sin_family = AF_INET;                     /*We use IPV4 protocol*/
          peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
          peeraddr.sin_port = htons(parent + DIFF_PORTS);/*Destination node port */
          char *tt = timeTaken();
          printf("Cell %d sends %s token  to Cell %d at %s\n\n", pid + 1, token, parent + 1, tt);
          sendto(socket_file_discripter, (const char *)token, strlen(token), MSG_CONFIRM, (const struct sockaddr *)&peeraddr, sizeof(peeraddr));

        }
        else
        {
            blueNeighbours(pid);
        }


      }
      else if (!strncmp(message_received, "white", 5)) //If we receive white token
      {
        *number_of_messages += 1;
        hasToken=true;
        if (pid == 0 && no_of_children == 0)//Checking for root node's termination
        {
          cout << " termination_detected the process and exit - white\n";
          process_done = 0;
          terminationDetected();
          return;
        }

        int sender = ntohs(peeraddr.sin_port) - DIFF_PORTS;

        char *tt = timeTaken();
        printf("Cell %d received %s token from Cell %d at %s\n\n", pid + 1, message_received, sender + 1, tt);

        if (hasToken == false)
          hasToken = true; //Because we have received the token from one of its child

        if (no_of_children > 0)
          no_of_children -= 1;//Decreseing no_of_children value by one after receiving the token


        if (pid!=0&&no_of_children == 0)// checking whether all children of node is terminated or not
        {

					//If a process terminates then it needs to send the token to its parent
          char token[100];
          strcpy(token, tokenColor);
          int parent = parentNode;
          hasToken = false;
          tokenSeen = false;
          strcpy(tokenColor, "white"); // We don't know about the previous color of the token
          makeParentToken(parent);


          struct sockaddr_in peeraddr;                       /*Now socket address to the peer to whome the process want to send the message*/
          peeraddr.sin_family = AF_INET;                     /*We use IPV4 protocol*/
          peeraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Local host address here*/
          peeraddr.sin_port = htons(parent + DIFF_PORTS); /*Destination node's port*/
          char *tt = timeTaken();
          printf("Cell %d sends %s token  to Cell %d at %s\n\n", pid + 1, token, parent + 1, tt);
          sendto(socket_file_discripter, (const char *)token, strlen(token), MSG_CONFIRM, (const struct sockaddr *)&peeraddr, sizeof(peeraddr));

        }
      }
      else
      {

        if(areChildrenTerminated(pid)) //Checking whether the children of process node are terminated or not
        {
            strcpy(color,"blue");
            strcpy(tokenColor,"white");
            hasToken=false;
            no_of_children=0;
        }

      }
    }

  }



	/*
   The below snippet of line shows the barrier synchronization of the process. We use this synchronization because
   process after creating the socket, it has to wait untill all other processes to create their sockets. So that they
   can communicate with each other. If we don't use this type synchronization, then process will send/receive messages to
   the processes which haven't created their sockets(No means of communication then). So, we use this barrier synchronizations.
 */
  void process_Synchronization_Barrier()
  {
    pthread_mutex_lock(&locker); /*all the processes should bind to socket before they start communicating each other*/
    process_done++;              /*Each process increments it if they bind the socket*/

    if (process_done == n) /*If all the processes are done with binding of the sockets then we release the lock and wake up the threads*/
    {
      for (int i = 1; i <= process_done; i++)     /*Here waking up the threads*/
        pthread_cond_signal(&conditioned_thread); /*This concept is used from operating systems subject for synchronization*/
    }

    else /*If not all processes completed the binding then wait untill they do */
    {
      pthread_cond_wait(&conditioned_thread, &locker); /*Waiting */
    }

    pthread_mutex_unlock(&locker); /*Do this after all processes completes the binding of socekts*/
    process_done = 0;
  }





	/*
			This socket_create() function creates socket for process.
			The port assigned to the socket is obtained from the process id.(i.e DIFF_PORT+processID)
			Each process will invoke this function to create socket for that process.
		*/
  int socket_create()
  {
    int PORT = DIFF_PORTS + pid; /* Making port number with process id. Here DIFF_PORTS=35000 as predefined */

    //  printf("Process %d is Port with %d\n",pid+1,PORT );
    struct sockaddr_in myaddr; /* Socket address variable*/

    /*
			In below if condition we used socket() function to create socket and the returned discriptor is stored in the
			socket_file_discripter. If socket() function returns the value less than 0 then socket is not created. i.e error
			occured while creating the socket.
		*/
    if ((socket_file_discripter = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
      printf("Socket is not created for process %d \n", pid + 1);
      return -1;
    }

    /*
			After creating the socket we need to bind it with address and port number to get identified.
			Below snippet of code is doing this task.
		*/
    myaddr.sin_family = AF_INET;         // IPv4
    myaddr.sin_addr.s_addr = INADDR_ANY; /*Addressss*/
    myaddr.sin_port = htons(PORT);       /*Port number*/

    /*
			We use bind() function to bind the address and port number
		*/
    int status = bind(socket_file_discripter, (const struct sockaddr *)&myaddr, sizeof(myaddr));

    if (status < 0) /*If returned value from the bind() is negative then we can conclude that error occured during the binding process*/
    {
      printf("Binding error id occurred for process %d \n", myaddr.sin_port - DIFF_PORTS);
      perror("bind failed");
      exit(EXIT_FAILURE);
      return -1; //failed to associate particular port number with this process....
    }

    /*
			If we successfully bind the address then we can declare that socket creation for the process is completed successfully.
		*/
    if (pid != n)
      printf("Socket Created Successfully for process %d \n", pid + 1);
    ;


    return 1;
  }


	/* This function will reset every attribute of the processes after calling the reinitiation of TD algorithm
	*/
  void reset()
  {
		/*Following snippet of the code will assign default values for processes*/
    strcpy(color, "white");
    strcpy(tokenColor, "white");
    hasToken = false;
    reported = false;
    tokenSeen = false;
    no_of_children = 0;

    if (isLeaf) /*If node is a leaf node*/
    {
      hasToken = true;
    }
		else//If process is not leaf node
		{
			no_of_children = spanningTree[pid].size() - 1;
		}

  }
};


//Processes global variables
Process **P;
Process *initiator_p;




/*	The create_process_and_socket() function is responsible for creating the process and socket for that process.  This
	function takes one argument as pid (which represents process id).
*/
void create_process_and_socket(int pid)
{

	if (pid <= n)
	{
		P[pid] = new Process(pid); //Creating Process object
		int status = P[pid]->socket_create(); //Calling socket_create() function to creat socket for process

		P[pid]->process_Synchronization_Barrier();//This is for synchronization

		if (status < 0)
		{
			printf("Socket creation is failed\n");
			return;
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
	stringstream ss(str);
	string tok;

	while (getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}

	return internal;
}

int main(int argc,char **argv)
{

		  		number_of_messages = (int *)malloc(sizeof(int));

					/*Below snippet of code will be used to read input file , whose path is given as argument to main function*/
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

             	if(newline==0)
							{
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



	P = new Process *[n];
	thread threads[n];

	for (i = 0; i < n; i++) /*Creating thread of each process. 'n' threads for 'n' processes*/
		threads[i] = thread(create_process_and_socket, i);

	for (i = 0; i < n; i++)
		threads[i].join();

	for (int i = 0; i < n; i++)
	{
		if (spanningTree[i].size() == 1)
		{
			P[i]->isLeaf = true;
			P[i]->hasToken = true;
			strcpy(P[i]->color, "white");
			strcpy(P[i]->tokenColor, "white");
			P[i]->no_of_children = 0;
		}
		else
		{
			P[i]->no_of_children = spanningTree[i].size() - 1;
			for (int j = 1; j < spanningTree[i].size(); j++)
				if (P[spanningTree[i][j] - 1]->parentNode == -1)
					P[spanningTree[i][j] - 1]->parentNode = i;
		}
	}

	srand((unsigned)time(0));


	std::cout << '\n';
	
	float f=sleepTime(Wr);
        sleep(f);
	chooseRandomRedProcesses();

	std::cout << '\n';


	while (_flag);

	return 0;

}


//calculates sleep time and returns that value
float sleepTime(float a)
{
	double reqSleepTime;
	reqSleepTime = rand() / (double)(RAND_MAX);
	reqSleepTime = -log(reqSleepTime) / a;
	return reqSleepTime;
}



/*
	The chooseRandomRedProcesses() chooses some random process between 1 and n. It turns selected processes color to red and
	spreads the red color to its neighbhour.

*/
void chooseRandomRedProcesses()
{

	std::vector<int> v = findRandomness(Ir, n); //Choosing some random processes as red processes

	char *stringTime=timeTaken();
	/*Below snippet of code will turn chosen processes as red*/
	for (int i = 0; i < Ir; i++)
	{
		stringTime=timeTaken();
		strcpy(P[v[i]]->color, "red");
		if (P[v[i]]->hasToken)
			strcpy(P[v[i]]->tokenColor, "black"); // We can make it as red/back
		*number_of_messages += 1;
		printf("Cell %d turns %s at %s\n\n", v[i] + 1, P[v[i]]->color, stringTime);
	}

	/*Need to forward the red message to chosen processes's neighours*/
	for (int i = 0; i < v.size(); i++)
	{
		int r = v[i];
		int adj = adjacencyMatrix[r].size();
		int n_size = ceil(adj * p);
		std::vector<int> vv = findRandomness(n_size, adj);
		for (int j = 0; j < n_size; j++)
		{
			int in = adjacencyMatrix[r][vv[j]] - 1;
			sendRedMessage(r, in);
		}
	}

}




/*
	The chooseRandomBlueProcesses() chooses some  random process between 1 and n. It turns selected processes color to blue and
	spreads the blue color to its neighbhour.
*/
void chooseRandomBlueProcesses()
{

	std::vector<int> v = findRandomness(Ib, n);//Choose random processes to make then blue

	char *stringTime=timeTaken();			 /*reading local system's time*/

	for (int i = 0; i < Ib; i++)
	{
		stringTime=timeTaken();

		strcpy(P[v[i]]->color, "blue");
		printf("Cell %d turns blue at %s\n\n", v[i] + 1, stringTime);
	}

	//Spreding the bluem message to chosen processe's neighours
	for (int i = 0; i < v.size(); i++)
	{
		int r = v[i];
		int adj = adjacencyMatrix[r].size();
		for (int j = 1; j < adj; j++)
		{
			int in = adjacencyMatrix[r][j] - 1;
			sendBlueMessage(r, in);
		}
	}

}



/*
	This findRandomness() function's computation will choose some random processes between given range(1 to n_adj_size).
	This function will produce no.of random processes as given size (size_subset).
	This function will return vector contain randomly selected processes and are distinct.

*/
vector<int> findRandomness(int size_subset, int n_adj_size) //returns subset of indices of size size_subset range between 0 to n_adj_size
{
	//size_subset is no.of indices
	//n_adj_size is no.of neighbours

	int i = 0, flag = 1;
	vector<int> v;
	while (i < size_subset)
	{
		int index = rand() % n_adj_size;
		if (index != 0)
		{
			if (i == 0)
			{
				v.push_back(index);
			}
			else
			{
				flag = 0;
				do
				{
					flag = 0;
					if (find(v.begin(), v.end(), index) != v.end() && index != 0)
					{
						flag = 1;
						index = rand() % n_adj_size;
					}
					else
					{
						v.push_back(index);
						flag = 0;
					}
				} while (flag);
			}
			i++;
		}
	}

	return v;
}



/*
	This function takes two arguments and sends red message to destination process.
*/
void sendRedMessage(int s, int d)
{

	if (s != d)//Source and destination processes should not be same
	{
		char message[MAX_BUFFER];
		sprintf(message, "%s ", "red");

		P[s]->send(s, d, message);
	}
}


/*
	This function takes two arguments and sends blue message to destination process.
*/
void sendBlueMessage(int s, int d)
{

	if (s != d)//Source and destination processes are not same
	{
		char message[MAX_BUFFER];
		sprintf(message, "%s ", "blue");
		P[s]->send(s, d, message);
	}
}


/*
	This function will read the time from the local system and then truncate the taken time to get required representation of the time.
	This function will return stringTime

*/
char *timeTaken()
{
	localclock local_clock;
	struct tm *time_information; /* Structure containing a calendar date and time broken down into its components*/
	time_t system_time;			 /* time type variable, which stores the local_clock time from the tableValues*/
	time_information;			 /*Converts to local time*/
	char *stringTime;			 /*Convert tm structure to string*/
	system_time = local_clock.Timeread();
	time(&system_time);
	time_information = localtime(&system_time);
	stringTime = asctime(time_information);
	stringTime[19] = '\0';
	stringTime = &stringTime[11];

	return stringTime;
}



/*
	This redNeighbours() function takes one argument and then sends red message to neighbours of the process that has taken as argument.
*/
void redNeighbours(int s)
{
	int n_size = ceil(adjacencyMatrix[s].size() * p);
	std::vector<int> v = findRandomness(n_size, adjacencyMatrix[s].size());
	for (int i = 0; i < v.size(); i++)
	{
		float tt = sleepTime(lamda_red);
		sleep(tt);
		sendRedMessage(s, adjacencyMatrix[s][v[i]] - 1);
	}
}

/*
	This blueNeighbours() function takes one argument and then sends blue message to neighbours of the process that has taken as argument.
*/
void blueNeighbours(int s)
{
	int n_size = ceil(adjacencyMatrix[s].size());
	std::vector<int> v = findRandomness(n_size, adjacencyMatrix[s].size());
	for (int i = 0; i < v.size(); i++)
	{
		float tt = sleepTime(lamda_blue);
		sleep(tt);
		sendBlueMessage(s, adjacencyMatrix[s][v[i]] - 1);
	}
}

/*This function is used to set root node*/
void setRootNode()
{

		strcpy(P[0]->color,"blue");
         P[0]->no_of_children=0;
          P[0]->hasToken=true;
}


/*
	This function will re-initiates the TD algorithm by making all processes attribute values to default.
*/

void reinitiateTD()
{

	for (int i = 0; i < n; i++)
	{
		P[i]->reset();

		P[i]->process_Synchronization_Barrier();
	}

	delayTime = true;
	d_sent = 0;
	cout << endl;
	chooseRandomRedProcesses();
}


/*This function will print all details of the processes after termination detection*/
void callPrint()
{
	printf("Pid\tcolor\tisLeaf\thasToken\ttokenColor\tParent Node\n");
	for (int i = 0; i < n; i++)
		cout << P[i]->pid + 1 << "\t"<< P[i]->color << "\t" << P[i]->isLeaf << "\t" << P[i]->hasToken << "\t\t" << P[i]->tokenColor << "\t\t"  << P[i]->parentNode + 1 << "\t" << endl;
}


/* This function will check whether all children of the node (taken as argument) are terminated or not
	 If all children are terminated the it returns true other wise it will return false.
*/
bool areChildrenTerminated(int s)
{
	if(P[s]->isLeaf) return false;

	bool flag=true;
	std::vector<char> v;
	for(int i=0;i<spanningTree[s].size();i++) v.push_back(spanningTree[s][i]);
	for(int i=1;i<v.size();i++)
	{
			if(P[v[i]-1]->no_of_children!=0||(strcmp(P[v[i]-1]->color,"blue")!=0)||(strcmp(P[v[i]-1]->tokenColor,"black")==0)) return false;
	}
	return true;
}


/*	This function will calculate the end time of termination detection.
		and displays details of time , no.of messages and it will call callPrint() functions
		to display details of all the processes

*/
bool terminationDetected(){

	 end_time=clock();/*End clock time */
	 double total=(end_time-start_time)/double(CLOCKS_PER_SEC);
	std::cout << "\n Time taken by * Spanning-Tree Termination Detection * Algorithm is: " <<total<<"\n";
	printf("%d is number of messages sent to detect Termination\n\n",*number_of_messages );
	callPrint();
	exit(0);
	return true;
}

/*When child sends token to parent , then we need to make sure that parent should have tokenSeen
	so we need to make hasToken value true for parent node
	*/
void makeParentToken(int s)
{
	P[s]->hasToken=true;
}


/* This function is used to check whether the root node has black token or not.
	If root node has black token this function will return true, otherwise returns false
*/
bool checkBlackToken()
{
	if(strcmp(P[0]->tokenColor,"black")==0) return true;
	return false;
}
