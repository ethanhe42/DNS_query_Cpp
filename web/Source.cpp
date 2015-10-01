//Header Files
#include "winsock2.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include <iostream>
#include<fstream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;

#define D 0
#define skip 256
#define maxthreads 800

int ii,jj;
int threadsCnt;

int domainCnt = 0;
string domains[2000];

int dnsCnt = 0;
string DNSlist[2000];

const int num_threads = 2000;

string logs[maxthreads];


//Type field of Query and Answer
#define T_A 1 /* host address */
#define T_NS 2 /* authoritative server */
#define T_CNAME 5 /* canonical name */
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 /* mail routing information */

#define FIR 0
#define SEC 1
#define THR 2
#define FUR 3

//Function Declarations
short ngethostbyname (unsigned char*, string, unsigned char);
void ChangetoDnsNameFormat (unsigned char*, unsigned char*);
unsigned char* ReadName (unsigned char*, unsigned char*, int*);

//DNS header structure
struct DNS_HEADER {
	unsigned short id; // identification number

	unsigned char rd : 1; // recursion desired
	unsigned char tc : 1; // truncated message
	unsigned char aa : 1; // authoritive answer
	unsigned char opcode : 4; // purpose of message
	unsigned char qr : 1; // query/response flag

	unsigned char rcode : 4; // response code
	unsigned char cd : 1; // checking disabled
	unsigned char ad : 1; // authenticated data
	unsigned char z : 1; // its z! reserved
	unsigned char ra : 1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION {
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA {
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD {
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

//Structure of a Query
typedef struct {
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;



void readDomain () {
	ifstream fin ("malurl.txt");
	while (!fin.eof()) {
		fin >> domains[domainCnt];
		domainCnt++;
	}
}

void readDNS () {

	ifstream fin ("dnslist.txt");
	while (!fin.eof ()) {
		fin >> DNSlist[dnsCnt];
		dnsCnt++;
	}
}

void call_from_thread (string destination, 
					   string host, 
					   int threadID) {
	
	string copy = destination;

	int ansnon = ngethostbyname ((unsigned char*)destination.c_str (),
								 host,
								0);
	////////////////////////////////////////////////////////////////////////
	int ans = ngethostbyname ((unsigned char*)destination.c_str (),
							  host,
							  1);
	////////////////////////////////////////////////////////////////////////
	if (ans != 0) {
		logs[threadID] = to_string (ans) + " " + 
			to_string (ansnon) + " " + 
			host + " " + 
			copy+"\n";
	}
	else {
		logs[threadID] = "";
	}

}


void searchCache (//unsigned int start, 
				//unsigned int over,
				const char* filename
				) {
	
	ofstream f1;
	f1.open (filename);

	
	//initial
	WSADATA firstsock;
	if(D)
		printf ("\nInitialising Winsock...");
	if (WSAStartup (MAKEWORD (2, 2), &firstsock) != 0) {
		printf ("Failed. Error Code : %d", WSAGetLastError ());
	}
	if(D)
		printf ("Initialised.");

	unsigned int ip;
	bool finished;

	for (int domain = 0; domain < domainCnt; domain++) {


//************************************************************************
		ip = 0;
		finished = false;

	Work:
		thread t[num_threads];//
		threadsCnt = 0;

		for (; threadsCnt < maxthreads;) {

			
			t[threadsCnt] = thread (call_from_thread, 
									domains[domain],
									DNSlist[ip],
									threadsCnt);
			threadsCnt++;


			if (ip == dnsCnt-1) {
				finished = true;
				break;
			}
			ip++;
		}
		
		for (int th = 0; th < threadsCnt; th++) {
			t[th].join ();
		}

		f1.open (filename, ios::app);
		for (int th = 0; th < threadsCnt; th++) {
			f1 << logs[th];
		}
		
		f1.close ();

		
		if (!finished) 
			goto Work;
		
		cout << (domain*100.0 / domainCnt) << endl;
//************************************************************************

	}
	if(D)
		cout << "\n 100 percents!" << endl;
}

void main (int argc, char ** argv) {

	readDomain ();
	readDNS ();
	string filename = "09291644.txt";

	searchCache (filename.c_str ());
	
}

short ngethostbyname (unsigned char *host, string dnsserver, unsigned char recursive) {
	string strbuf="";
	int Nans = 0;

	unsigned char buf[65536], *qname, *reader;
	int i, j, stop;

	SOCKET s;
	struct sockaddr_in a;
	struct sockaddr_in local;

	struct RES_RECORD answers[20], auth[20], addit[20]; //the replies from the DNS server
	struct sockaddr_in dest;

	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;

	 //UDP packet for DNS queries
	if ((s = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf ("Could not create socket : %d", WSAGetLastError ());
	}

	//Configure the sockaddress structure with information of DNS server
	dest.sin_family = AF_INET;
	dest.sin_port = htons (53);
	//Set the dns server
	dest.sin_addr.s_addr = inet_addr (dnsserver.c_str());

	//Set the DNS structure to standard queries
	dns = (struct DNS_HEADER *)&buf;

	dns->id = (unsigned short)htons (GetCurrentThreadId ());
	dns->qr = 0; //This is a query
	dns->opcode = 0; //This is a standard query
	dns->aa = 0; //Not Authoritative
	dns->tc = 0; //This message is not truncated
	dns->rd = recursive; //Recursion Desired
	dns->ra = 0; //Recursion not available! hey we dont have it (lol)
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons (1); //we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//point to the query portion
	qname = (unsigned char*)&buf[sizeof (struct DNS_HEADER)];

	ChangetoDnsNameFormat (qname, host);
	qinfo = (struct QUESTION*)&buf[sizeof (struct DNS_HEADER) + (strlen ((const char*)qname) + 1)]; //fill it

	qinfo->qtype = htons (1); //we are requesting the ipv4 address
	qinfo->qclass = htons (1); //its internet (lol)

	if(D) 
		printf ("\nSending Packet...");


	if (sendto (s, (char*)buf, sizeof (struct DNS_HEADER) + (strlen ((const char*)qname) + 1) + sizeof (struct QUESTION), 0, (struct sockaddr*)&dest, sizeof (dest)) == SOCKET_ERROR) {
			printf ("%d error \n", WSAGetLastError ());
	}
	if(D) 
		printf ("Sent");

	i = sizeof (dest);
	if (D) 
		printf ("\nReceiving answer...");


	int timeout = 1500; // milliseconds 
	setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof (int));
	if (recvfrom (s, (char*)buf, 65536, 0, (struct sockaddr*)&dest, &i) == SOCKET_ERROR) {
		if(D) 
			printf ("Failed. Error Code : %d \n", WSAGetLastError ());
	}

	closesocket (s);
	if (D) printf ("Received.");

	dns = (struct DNS_HEADER*)buf;

	Nans = ntohs (dns->ans_count);

	if (!Nans) return 0;
	return Nans;
}


unsigned char* ReadName (unsigned char* reader, unsigned char* buffer, int* count) {
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char*)malloc (256);

	name[0] = '\0';

	//read the names in 3www6google3com format
	while (*reader != 0) {
		if (*reader >= 192) {
			offset = (*reader) * 256 + *(reader + 1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		}
		else {
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumped == 0) *count = *count + 1; //if we havent jumped to another location then we can count up
	}

	name[p] = '\0'; //string complete
	if (jumped == 1) {
		*count = *count + 1; //number of steps we actually moved forward in the packet
	}

	//now convert 3www6google3com0 to www.google.com
	for (i = 0; i<(int)strlen ((const char*)name); i++) {
		p = name[i];
		for (j = 0; j<(int)p; j++) {
			name[i] = name[i + 1];
			i = i + 1;
		}
		name[i] = '.';
	}

	name[i - 1] = '\0'; //remove the last dot

	return name;
}


//this will convert www.google.com to 3www6google3com ;got it :)
void ChangetoDnsNameFormat (unsigned char* dns, unsigned char* host) {
	int lock = 0, i;

	strcat ((char*)host, ".");

	for (i = 0; i<(int)strlen ((char*)host); i++) {
		if (host[i] == '.') {
			*dns++ = i - lock;
			for (; lock<i; lock++) {
				*dns++ = host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++ = '\0';
}