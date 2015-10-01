#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include <iostream>
#include<fstream>
#include <string>
#include <cstdlib>
#include <thread>
#include <mutex>


using namespace std;

mutex mu;

//2182
const int SegmentCnt = 2182;
int progress = 0;
thread p[100];
int processCnt = 65;
ofstream f;
bool finishsoon = false;

//const int segCnt = 2182;
unsigned int segments[SegmentCnt][2];

string path = "C:\\Users\\Pomodori\\OneDrive\\code\\Web\\Debug";
//"C:\\cloud\\Internet\\phoneDNS\\multi";

string workAt (string path) {
	return "cd " + path + " & ";
}

void cmd (string command, string loc = "") {

	system ((workAt (loc) + command).c_str ());
}

void readSegment () {
	ifstream fin ("segment.txt");
	string test;
	int i = 0;
	int j;
	while (!fin.eof ()) {
		for (j = 0; j < 2; j++) {
			fin >> test;
			segments[i][j]=(unsigned int)(atoi(test.c_str()));

		}
		i++;
	}
}

void process (int processID) {
	int myWork;
	bool out=false;

Work:

		
	mu.lock ();
	if (progress + 1 >= SegmentCnt) {
		out = true;
	}
	else {
		cout << ((float)(progress * 100) / (float)(SegmentCnt)) << " %" << endl;
	}
	myWork = progress;


	progress++;

	mu.unlock ();
		
	string C = "web "
		+ to_string (segments[myWork][0]) + " "
		+ to_string (segments[myWork][1]) + " "
		+ to_string (processID + 2);

	cmd (C, path);

	if (out == false)
		goto Work;

	if (!finishsoon) {
		mu.lock ();
		cout << "finish soon..." << endl;
		mu.unlock ();
		finishsoon = true;
	}
	return;
}

void main () {


	
	cout << "Warning: \nsave data - Copy(X).txt first,\n this program will del files first!" << endl;
	cout << "enter processCnt : ";
	cin >> processCnt;

	readSegment ();


	int i = 0;
	for (i = 0; i < processCnt; i++) {
		string filename = "data - Copy (" + to_string (i+2) + ").txt";
		f.open (filename);
		f.close ();


		p[i]= thread(process ,i);

	}
	for (i = 0; i < processCnt; i++) {
		p[i].join ();
	}

	cout << "Final!" << endl;
	getchar ();
}

