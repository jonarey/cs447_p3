//Jonathan Reynolds
//server.cpp

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
//used for the nonblocking socket
//#include <fcntl.h>
using namespace std;

const int NUM_SECONDS = 3;



#define BACKLOG 10     // how many pending connections queue will hold
int MAX= 1080;
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
//returns all char * of all values up to first space
char * returnFirst(char * cmd){
  char * copy = new char[strlen(cmd)+1];
  const char * semi = ";";
  copy = strcat(cmd,semi);
  char * first = strtok(copy,semi);
  return first;
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
void send(int socket, string msg, char * buffer)
{
	memset(&buffer,0,sizeof buffer);
	buffer = strToCharStar(msg);
        if (send(socket,buffer,MAXDATASIZE-1,0)==-1)
           	printf("error sending to client");

}
string receive(int socket,char * buffer)
{
        int numbytes;
        if ((numbytes = recv(socket, buffer, MAXDATASIZE-1, 0)) == -1)
        {
                printf("error receiving from client\n");
                exit(1);
        }
        char * received = trimtrailing(buffer);
        string receivedstr = charStarToStr(received);
        //cout << received << endl;
	memset(&buffer,0,sizeof buffer);
        return received;

}
//check command - checks if str2 is matches str1 from 0 to str2.len -1
bool checkCommand(string str1, string str2)
{
        int check = str2.compare(str1.substr(0,str2.length()));
        //cout << "checkCommand Value:" << check << endl;
        if(check == 0)
                return true;
        else return false;
}


//file paths
string O_PATH = "";
string T_PATH = "";
string P_PATH = "";

//RSTP TERMS
string SETUP = "SETUP";
string PLAY = "PLAY";
string PAUSE = "PAUSE";
string TEARDOWN = "TEARDOWN";
string cseq_str = "CSeq:";
string transport_str = "Transport:";
string sensor_str = "Sensor:";
string dest_addr = "dest_addr";

//state
int state = 0;

//Server Responses
string ACK = "ACK";
string msg_200 = "200 OK";
string date = "Date: ";


//used to store multiple lines from client
string line_1 = "";



string response1 = "";
string response2 = "";
string response3 = "";
string response4 = "";
string setup_response = "";
//variables to be filled by setup request
string setup_req = "";
string cseq_req = "";
string transport_req = "";
string sensor_req = "";
string play_req = "";
//server variables
string src_addr = "";
string protocol = "";
string cseq_val = "";

// ************** dest addr ***********
string data_port;

const char * udp_port;

//string data_port;

// ********************* PROCESS SETUP ***********************
int processSetup()
{
size_t setup_pos = line_1.find(SETUP);
size_t cseq_pos = line_1.find(cseq_str);
size_t transport_pos = line_1.find(transport_str);
size_t sensor_pos = line_1.find(sensor_str);	

setup_req = line_1.substr(0, cseq_pos);

//cout << (int)cseq_pos << endl;
char * trimmer = trimtrailing(strToCharStar(setup_req));
setup_req = charStarToStr(trimmer);
line_1.erase(0,cseq_pos);

cseq_req =  line_1.substr(0, transport_pos-cseq_pos);

trimmer = trimtrailing(strToCharStar(cseq_req));
cseq_req = charStarToStr(trimmer);
line_1.erase(0,transport_pos-cseq_pos);

transport_req =  line_1.substr(0, sensor_pos-transport_pos);
trimmer = trimtrailing(strToCharStar(transport_req));
transport_req = charStarToStr(trimmer);
line_1.erase(0,sensor_pos-transport_pos);


sensor_req = line_1; 
//cout << cseq_req << endl << transport_req << endl << sensor_req << endl;

return 0;	
}

bool setupVerify()
{
        protocol = setup_req.substr(setup_req.length()-8,8);
	//cout << protocol << endl;
return true;
}
//extracts cseq value
bool cseqVerify()
{	
	size_t cseq_pos = cseq_req.find(cseq_str);
	int num = cseq_req.length() - cseq_str.length();
	cseq_val = cseq_req.substr(cseq_str.length(),num);
	//cout << cseq_val << endl;	
        return true;
}
//extracts udp port 
bool transportVerify()
{
	size_t dest_pos = transport_req.find(dest_addr);
	string port = transport_req.substr(dest_pos+11,transport_req.length()-dest_pos-12);
	cout << "UDP Port: " << port << endl;
	udp_port = strToCharStar(port);
	return true;
}
bool sensorVerify()
{
	//size_t dest_pos = transport_req.find(dest_addr);
        return true;
}


//proccess play command
int processPlay()
{
	size_t play_pos = line_1.find(PLAY);
	size_t cseq_pos = line_1.find(cseq_str);
	size_t sensor_pos = line_1.find(sensor_str);

	play_req = line_1.substr(0, cseq_pos);
	char * trimmer = trimtrailing(strToCharStar(play_req));
	play_req = charStarToStr(trimmer);
	line_1.erase(0,cseq_pos);
	
	//protocol
	protocol = play_req.substr(play_req.length()-8,8);

	cseq_req =  line_1.substr(0, sensor_pos-cseq_pos);
	trimmer = trimtrailing(strToCharStar(cseq_req));
	cseq_req = charStarToStr(trimmer);

	line_1.erase(0,sensor_pos-cseq_pos);
	sensor_req = line_1;

	return 0;
}
//READS BITS FROM BIN FILE
string readBits(ifstream & f, int range)
{
        char c;
        string bits;

        for(int j = 1; j <= range; j++){
                f.get(c);
                //i converts char to string of bits
                for (int i = 7; i >= 0; i--)
                {
                        bits += to_string(((c >> i) & 1));
                }
        }

        return bits;
}

// ************ GLOBAL VARIABLES FOR SENSORS ************
int O_RANGE = 5;
int T_RANGE = 8;
int P_RANGE = 11;
string O_BITS = "";
string O_KEY = "79";
string T_BITS = "";
string T_KEY = "84";
string P_BITS = "";
string P_KEY = "80";

bool O_OUT = false;
bool T_OUT = false;
bool P_OUT = false;

string O_SEND = "";
string T_SEND = "";
string P_SEND = "";

string DATA_BUFF = "";
// ASSIGNS SENSORS TO SEND
string sensor_test = "SENSOR: *";
void processSensor(string sensor)
{

size_t pos = sensor.find(O_KEY);

if(pos == string::npos)
        {
                O_OUT = false;
                
        } else O_OUT = true;

pos = sensor.find(T_KEY);

if(pos == string::npos)
        {
                T_OUT = false;

        } else T_OUT = true;
pos = sensor.find(P_KEY);

if(pos == string::npos)
        {
                P_OUT = false;

        } else P_OUT = true;

string star = "*";
pos = sensor.find(star);

if(pos != string::npos)
        {
		//cout << "all sensors" << endl;
		O_OUT = true;
		T_OUT = true;
                P_OUT = true;

        }
}

//determines size the if streams read from bin file
char IF_GET;

//loads bits into requested sensor bufer if empty & requested
void loadBits( ifstream & o, ifstream & t, ifstream & p)
{
	if(O_OUT && O_BITS.length() == 0)
	{
		//readOxygen();
		//cout << "load O\n";
		 O_BITS =  readBits(o, O_RANGE);
	}
	if(T_OUT && T_BITS.length() == 0)
	{
		//readTemp();
		//cout << "load T\n";
		T_BITS =  readBits(t, T_RANGE);
	}
	if(P_OUT && P_BITS.length() == 0)
	{
		//readPressure();
		//cout << "load P\n";
		P_BITS =  readBits(p, P_RANGE);
	}
}

// LOAD BITS TO SEND TO DATA
void loadSend()
{
	string semi = ";";
	string colon = ":";
	if(O_OUT)
	{  
		O_SEND = O_BITS.substr(0,O_RANGE);
		O_BITS.erase(0,O_RANGE);
		DATA_BUFF += O_KEY + colon + O_SEND + semi;
	}
	if(T_OUT)
	{
                T_SEND = T_BITS.substr(0,T_RANGE);
                T_BITS.erase(0,T_RANGE);
		DATA_BUFF += T_KEY + colon + T_SEND + semi;
	}
	if(P_OUT)
	{
                P_SEND = P_BITS.substr(0,P_RANGE);
                P_BITS.erase(0,P_RANGE);
		DATA_BUFF += P_KEY + colon + P_SEND + semi;
	}

//	cout << DATA_BUFF << endl;
}

int main(int argc, char * argv[])
{
state = 0;
// https://stackoverflow.com/questions/10807681/loop-every-10-second
double time_counter = 0;
clock_t this_time = clock();
clock_t last_time = this_time;
//
//
if (argc != 5) {
                fprintf(stderr,"usage:<TCP Port> <Oxygen> <Temperature> <Pressure>");
                exit(1);
        }
	
	//assign bin paths from parameters
	//
	O_PATH = argv[2];
	T_PATH = argv[3];
	P_PATH = argv[4];

        
	// DECLARE IFSTREAM TO EACH FILE
	ifstream o_f(argv[2], ios::binary | ios::in);
	ifstream t_f(argv[3], ios::binary | ios::in);
	ifstream p_f(argv[4], ios::binary | ios::in);
               
	
	int sockfd, server_socket, udp_socket;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
        struct sigaction sa;
        int yes = 1;
    	char address[INET6_ADDRSTRLEN];
    	int rv;
    	//char buf[MAXDATASIZE];
    	int numbytes;

    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC;
    	hints.ai_socktype = SOCK_STREAM; //declares it as tcp connection SOCK_DGRAM for UDP
    	hints.ai_flags = AI_PASSIVE; // use my IP
   
    //running tcp port number as to ensure TCP connection for all clients
    char * port = argv[1];
    //use if else on port == tcp for email else if port == udp receiver
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

   // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }


        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }
   
    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: couldn't bind with client!\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        printf("error listening");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    //pid = fork();

    while(1)
    {

	    	sin_size = sizeof their_addr;
                //accept tcp connection from client
                server_socket = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                if (server_socket == -1) {
                        printf("accept error when establishing server socket descriptor");
                        continue;
                }
                //convert the client IP to readable format
                inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),address, sizeof address);
                printf("server: got connection from %s\n", address);

                //close down server connection upon client closing
                if (!fork())
                { // this is the child process
                        close(sockfd); // child doesn't need the listener
                        exit(1);
                }

		//******************************** SETUP ************************* 
		//cout << "BEFORE STATE 0\n";
		int setup_result;
		if(state == 0)
		{
	//		cout << "In State 0\n";
		
			line_1 = receive(server_socket,buf);
			setup_result = processSetup();	

			transportVerify();
			cseqVerify();	
			setupVerify();	

			string msg = protocol + " " + msg_200 + "\n" + cseq_str + cseq_val+ "\n" + date + "\n" + transport_req + ";" + "src_addr: \n" ;  
			send(server_socket,msg,buf);
			state = 1;
		}
		//PLAY
		int play_result = 0;
		if(state == 1)
		{
			//cout << "In state 1\n";
			
			//receive play command		
			line_1 = receive(server_socket,buf);
			play_result = processPlay();
			cseqVerify();
			sensorVerify();
			
			string msg = protocol + " " + msg_200 + "\n" + cseq_str + cseq_val+ "\n" + date + "\n";
			send(server_socket,msg,buf);
			//line_2 = receive(server_socket,buf);
			//send(server_socket,ACK,buf);
			//
			//process play
						

		state = 2;

		}

	
		memset(&hints,0, sizeof hints);
		memset(&servinfo,0,sizeof servinfo);
		hints.ai_family = AF_UNSPEC;    //use IPV4 or V6
	        hints.ai_socktype = SOCK_DGRAM; //DATAGRAM (UDP)
	        hints.ai_flags = AI_PASSIVE;    //fill IP from user

		int getaddr_udp;
		//udp_port = strToCharStar(receivedstr);
		if((getaddr_udp = getaddrinfo(address,udp_port,&hints,&servinfo)) != 0)
        	{	
                	cout << "Error getaddrinfo: client" << endl;
        	}

		if ((udp_socket = socket(servinfo->ai_family, SOCK_DGRAM, servinfo->ai_protocol)) < 0)
        	{	
        	        perror("cannot create socket");
        	}
		//cout << "Data Socket Created\n";
		struct sockaddr * x = (struct sockaddr *) servinfo;	//PLAYING


		if(state == 2)
		{
		//cout << "state 2" << endl; 
		processSensor(sensor_req);
		loadBits(o_f,t_f,p_f);
		state = 3;	
		}	
		
		//HOLDS BITS TO BE SENT TO DATA.CPP
		char * data_send;
		// SENDING TO DATA.CPP
		if(state == 3)
		{
		//makes server socket nob_blocking for receive
///		fcntl(server_socket, F_SETFL, O_NONBLOCK);
		line_1 = "";
		while(true)
		{
			//line_1 = receive(server_socket,buf);
			//cout << "from client " << line_1 << endl;
			
			
			//loads from bin file is buffer is empty
			loadBits(o_f,t_f,p_f);
			//reset DATA_BUFF
			DATA_BUFF = "";	
	        	this_time = clock();
        		time_counter += (double)(this_time - last_time);
        		last_time = this_time;

			//sends every 3 seconds
        		if(time_counter > (double)(NUM_SECONDS * CLOCKS_PER_SEC))
        		{
            			time_counter -= (double)(NUM_SECONDS * CLOCKS_PER_SEC);
            			//printf("printing every 3 seconds\n");
				loadSend();
				data_send = strToCharStar(DATA_BUFF);
				//send to data.cpp
				int sensor_send = sendto(udp_socket,data_send,sizeof buf,0,servinfo->ai_addr,servinfo->ai_addrlen);

        		}

            		}
	
		}
		//PAUSE
				
		//TEARDOWN

		//cout << "End\n";
    }



	return 0;
}
