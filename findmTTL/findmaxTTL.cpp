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

int errordnscount = 0;
int errorsent = 0;
int serverGot = 0;
int ii, jj;
int threadsCnt;
int malurlCnt = 0;
string malurl[1100] = { " " };
unsigned long long maxTTL[1100];

//const int segCnt = 2182;
//unsigned int segments[segCnt][2];
const int num_threads = 2000;
//const int maxthreads = 800;

string logs[maxthreads];
//int ips[segCnt][2];


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
void ngethostbyname (unsigned char *host,
					 string dnsserver,
					 unsigned char recursive,
					 unsigned short qtype,
					 string* logsInfo,
					struct DNS_HEADER *dh,
					struct RES_RECORD an[20],
					struct RES_RECORD au[20],
					struct RES_RECORD ad[20]);
void ChangetoDnsNameFormat(unsigned char*, unsigned char*);
unsigned char* ReadName(unsigned char*, unsigned char*, int*);

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


unsigned long long findTTL (string host) {
	struct DNS_HEADER head;
//	*head = NULL;
	struct RES_RECORD answer[20], auth[20], addit[20];
	string ttllog;
	sockaddr_in a;
	long *p;
	unsigned long long Mttl = 0;
	string localDNS = "114.114.114.114";

	ngethostbyname ((unsigned char*)(host.substr (host.find_first_of ('.')+1).c_str()),
					localDNS,
					1,
					2,
					&ttllog,
					&head,
					answer,
					auth,
					addit);

	if (ntohs (head.ans_count) == 0) {
		goto Ret;
	}
	

	p = (long*)addit[0].rdata;
	a.sin_addr.s_addr = (*p); //working without ntohl
	cout << inet_ntoa (a.sin_addr) << endl;


	ngethostbyname ((unsigned char*)host.c_str(),
					inet_ntoa (a.sin_addr),
					1,
					1,
					&ttllog,
					&head,
					answer,
					auth,
					addit);

	if (ntohs (head.ans_count) == 0) {

		//if (ntohs (head.auth_count) != 0) {

		//	ngethostbyname (auth[0].rdata,
		//					localDNS,
		//					1,
		//					1,
		//					&ttllog,
		//					&head,
		//					answer,
		//					auth,
		//					addit);
		//	if (ntohs (head.ans_count) == 0) {
		//		goto Ret;
		//	}

		//	p = (long*)answer[0].rdata;
		//	a.sin_addr.s_addr = (*p); //working without ntohl
		//	cout << inet_ntoa (a.sin_addr) << endl;

		//	ngethostbyname ((unsigned char*)host.c_str (),
		//					inet_ntoa (a.sin_addr),
		//					1,
		//					1,
		//					&ttllog,
		//					&head,
		//					answer,
		//					auth,
		//					addit);
		//}
		//else
			goto Ret;
	}

	Mttl = ntohl (answer[0].resource->ttl);

Ret:
	return Mttl;
}

void readURLlist () {
	ifstream fin ("malurl.txt");
	string test;
	int j;
	while (!fin.eof ()) {
			fin >> test;
			malurl[malurlCnt]=test;
			malurlCnt++;
	}
}
void writeTTLlist () {
	ofstream fout ("maxTTL.txt");
	for (int i = 0; i < malurlCnt; i++) {

		fout << maxTTL[i] << endl;
	}
	fout.close ();
}

void main () {
	WSADATA firstsock;
	WSAStartup (MAKEWORD (2, 2), &firstsock);
	string log;

	//string host = "138118.com";
	////oldLookup (host, 2);
	//findTTL (host);
	//return;

	readURLlist ();
	

	for (int i = 0; i < malurlCnt; i++) {
		cout << "**********************************" << endl;
		cout << malurl[i] << endl;
		maxTTL[i]= findTTL (malurl[i]);
		cout << maxTTL[i]<<endl;
	}
	writeTTLlist ();
	
	cout << "search Complete" << endl;
}

void ngethostbyname(unsigned char *host,
					string dnsserver, 
					unsigned char recursive,
					unsigned short qtype, 
					string* logsInfo,
                    struct DNS_HEADER *hd,
					struct RES_RECORD answers[20],
					struct RES_RECORD auth[20],
					struct RES_RECORD addit[20]) {
	*logsInfo = "";
	string strbuf = "";
	int Nans = 0;

	unsigned char buf[65536], *qname, *reader;
	int i, j, stop;

	SOCKET s;
	struct sockaddr_in a;
	struct sockaddr_in local;

	struct sockaddr_in dest;

	struct DNS_HEADER* dns = NULL;
	struct QUESTION *qinfo = NULL;

	//UDP packet for DNS queries
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
	}

	//Configure the sockaddress structure with information of DNS server
	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	//Set the dns server
	dest.sin_addr.s_addr = inet_addr(dnsserver.c_str());

	//Set the DNS structure to standard queries
	dns = (struct DNS_HEADER *)&buf;

	dns->id = (unsigned short)htons(GetCurrentThreadId());
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
	dns->q_count = htons(1); //we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	//point to the query portion
	qname = (unsigned char*)&buf[sizeof(struct DNS_HEADER)];

	ChangetoDnsNameFormat(qname, host);
	qinfo = (struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it

	qinfo->qtype = htons(qtype); //we are requesting NS record
	qinfo->qclass = htons(1); //its internet (lol)


	if (D) printf("\nSending Packet...");

	if (sendto(s, (char*)buf, sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR) {
		printf("%d error \n", WSAGetLastError());
		errorsent++;
	}
	if (D) printf("Sent");

	i = sizeof(dest);
	if (D) printf("\nReceiving answer...");


	int timeout = 10000; // milliseconds 
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	if (recvfrom(s, (char*)buf, 65536, 0, (struct sockaddr*)&dest, &i) == SOCKET_ERROR) {
		if (D) printf("Failed. Error Code : %d \n", WSAGetLastError());
	}

	closesocket(s);
	if (D) printf("Received.");

	dns = (struct DNS_HEADER*)buf;

	//move ahead of the dns header and the query field
	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION)];

	//reading answers
	stop = 0;

	for (i = 0; i<ntohs (dns->ans_count); i++) {
		answers[i].name = ReadName (reader, buf, &stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA*)(reader);
		reader = reader + sizeof (struct R_DATA);

		if (ntohs (answers[i].resource->type) == 1) //if its an ipv4 address
		{
			answers[i].rdata = (unsigned char*)malloc (ntohs (answers[i].resource->data_len));

			for (j = 0; j<ntohs (answers[i].resource->data_len); j++)
				answers[i].rdata[j] = reader[j];

			answers[i].rdata[ntohs (answers[i].resource->data_len)] = '\0';

			reader = reader + ntohs (answers[i].resource->data_len);

		}
		else {
			answers[i].rdata = ReadName (reader, buf, &stop);
			reader = reader + stop;
		}

	}

	//read authorities
	for (i = 0; i<ntohs (dns->auth_count); i++) {
		auth[i].name = ReadName (reader, buf, &stop);
		reader += stop;

		auth[i].resource = (struct R_DATA*)(reader);
		reader += sizeof (struct R_DATA);

		auth[i].rdata = ReadName (reader, buf, &stop);
		reader += stop;
	}

	//read additional
	for (i = 0; i<ntohs (dns->add_count); i++) {
		addit[i].name = ReadName (reader, buf, &stop);
		reader += stop;

		addit[i].resource = (struct R_DATA*)(reader);
		reader += sizeof (struct R_DATA);

		if (ntohs (addit[i].resource->type) == 1) {
			addit[i].rdata = (unsigned char*)malloc (ntohs (addit[i].resource->data_len));
			for (j = 0; j<ntohs (addit[i].resource->data_len); j++)
				addit[i].rdata[j] = reader[j];

			addit[i].rdata[ntohs (addit[i].resource->data_len)] = '\0';
			reader += ntohs (addit[i].resource->data_len);

		}
		else {
			addit[i].rdata = ReadName (reader, buf, &stop);
			reader += stop;
		}
	}

	(*hd).add_count = dns->add_count;
	(*hd).auth_count = dns->auth_count;
	(*hd).ans_count = dns->ans_count;
}


unsigned char* ReadName(unsigned char* reader, unsigned char* buffer, int* count) {
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char*)malloc(256);

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
	for (i = 0; i<(int)strlen((const char*)name); i++) {
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
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
	int lock = 0, i;

	strcat((char*)host, ".");

	for (i = 0; i<(int)strlen((char*)host); i++) {
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