//Jonathan Reynolds
//data.cpp
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <ctime>
using namespace std;

#define BACKLOG 10     // how many pending connections queue will hold
int MAX= 1080;

string bits = "";

string O_KEY = "79";
string T_KEY = "84";
string P_KEY = "80";

int O_SIZE = 5;
int T_SIZE = 8;
int P_SIZE = 11;

int O_VAL = 0;
int T_VAL = 0;
int P_VAL = 0;

double O_MAX = 31;
double T_MAX = 255;
double P_MAX = 2047;

bool O_IN = false;
bool T_IN = false;
bool P_IN = false;

string test = "79:11111;84:11111111;80:11111111111;";
//************* SENSOR METHODS *************

//SCANS SENSOR STRING  TO DETERMINE READINGS
void readO(string data)
{
//find 79 in data
size_t pos = data.find(O_KEY);
//O not in data
if(pos == string::npos)
	{	O_IN = false;		
		O_VAL = 0;
		cout << "no O" << endl;
		return;
	}
O_IN = true;
int o_pos = (int)pos;
//cout << o_pos << endl;
string o_bits = data.substr(o_pos+3,O_SIZE);
//cout << o_bits <<endl;
O_VAL = stoi(o_bits,nullptr,2);
//cout << O_VAL << endl;
}

void readT(string data)
{
//find 84 in data
size_t pos = data.find(T_KEY);
//O not in data
if(pos == string::npos)
        {
		T_IN = false;
                T_VAL = 0;
                return;
        }
T_IN = true;
int t_pos = (int)pos;
//cout << o_pos << endl;
string t_bits = data.substr(t_pos+3,T_SIZE);
//cout << o_bits <<endl;
T_VAL = stoi(t_bits,nullptr,2);
//cout << T_VAL << endl;
}

void readP(string data)
{
//find 89 in dataind(P_KEY);
size_t pos = data.find(P_KEY);
//O not in data
if(pos == string::npos)
        {
		P_IN = false;
                P_VAL = 0;
                return;
        }
int p_pos = (int)pos;
P_IN = true;
//cout << o_pos << endl;
string p_bits = data.substr(p_pos+3,P_SIZE);
//cout << o_bits <<endl;
P_VAL = stoi(p_bits,nullptr,2);
//cout << P_VAL << endl;
}



void readBits(string data)
{
//return Oxygen bitval 

readO(data);

readT(data);

readP(data);

}

// *****************************************

//******* HISTOGRAM METHODS *************


//number of stars in histogram
int STAR_COUNT = 20;
void clearScreen()
{
	for(int i = 0; i = 10; i++)
	{
		cout << endl;
	}
}

void printHist()
{
	double o_double = double(O_VAL);
	double o_stars = o_double/O_MAX*20.0;
        double t_double = double(T_VAL);
        double t_stars = t_double/T_MAX*20.0;
        double p_double = double(P_VAL);
        double p_stars = p_double/P_MAX*20.0;	
	if(O_IN)
	{
		cout << "O:";

		for(double i = 0; i < o_stars; i++)
		{
			cout << "*";
		}
		cout << endl;
	}
        if(T_IN)
        {
                cout << "T:";

                for(double i = 0; i < t_stars; i++)
                {
                        cout << "*";
                }
                cout << endl;
        }	
        if(P_IN)
        {
                cout << "P:";

                for(double i = 0; i < p_stars; i++)
                {
                        cout << "*";
                }
                cout << endl;
        }

//	clearScreen();
}


//******** END HISTOGRAM METHODS *************


#define MAXDATASIZE 1000 // max number of bytes we can get at once
char buf[MAXDATASIZE];
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
char *trimtrailing(char *str) //removes trailing spaces from client input
{
  char *end;
  // Trim trailing spaces
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  // Write new null terminator
  *(end+1) = 0;
  return str;
}
//string converters
char * strToCharStar(string str)
{
        char * charstar = new char[str.length()+1];
        strcpy(charstar,str.c_str());
        return charstar;
}
string charStarToStr(char * cstr)
{
        string str = cstr;
        return str;
}
//struct to store incoming connections information
struct sockaddr_storage their_addr;
socklen_t addr_size;


int main(int argc, char * argv[])
{
	if (argc != 2) {
                fprintf(stderr,"usage: <UDP Port>");
                exit(1);
        }

	//***************** TEST DISPLAY METHODS ***************
	//readBits(test);
	//printHist();

	int status;
        struct addrinfo hints,*res, *x;
        //stores client data from recvfrom
        struct sockaddr_storage their_addr;
        socklen_t addr_len;
        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;
        // get ready to connect (creating socket)
        char * port = argv[1];
        if((status = getaddrinfo(NULL, port, &hints, &res)) != 0 )
        {
                cout << "Error getting address info: server" << endl;
        }

	int listen_socket;
        int bind_status;
        x = res;
        //for(x=res; x!= NULL; x = x->ai_next)
        //{
                listen_socket = socket(x->ai_family,x->ai_socktype,x->ai_protocol);
          //      cout << "Listen Socket: " << listen_socket << endl;
                bind_status = bind(listen_socket,x->ai_addr,x->ai_addrlen);
          //     cout << "Bind Status: " << bind_status << endl;
        //}

        if(x == NULL)
        {
                cout << "Listener Failed to bind()\n";
        }

        freeaddrinfo(res);

        addr_size = sizeof their_addr;

	char address[INET6_ADDRSTRLEN];
        //inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),address, sizeof address);
        //printf("talking with: %s",address);
        //int rc= recvfrom(server_socket,buf,sizeof(buf),0,(struct sockaddr *)&their_addr,&addr_size);
        int go = 1;

        while(go)
        {
        int numbytes = recvfrom(listen_socket, buf, sizeof buf , 0,(struct sockaddr *)&their_addr, &addr_len);


	// ************ CLEAR SCREEN **************	
	//clearScreen();

        char * received_char = trimtrailing(buf);
        string received = charStarToStr(received_char);

        //cout << "From Server: " << received << endl;
	//***************** READ BITS & PRINT HIST
	readBits(received);
	if(numbytes != 0){
	printHist();
	cout << "\n\n\n\n\n\n\n\n\n\n";
	}
        }

	return 0;
}


