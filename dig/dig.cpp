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

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;

#define D 1

string digLoc = "C:\\cloud\\Internet\\phoneDNS\\BIND9.9.6.x86";
string pjLoc = "C:\\Users\\Pomodori\\OneDrive\\code\\Web\\dig";
string logName = "l.log";

string workAt (string path) {
	return "cd " + path + " & ";
}

void cmd (string command,string loc="") {
	static bool log = false;
	if (!log) {
		system ((workAt(loc) + command + " > "+logName).c_str ());
		log = true;
	}
	system ((workAt(loc) + command + " >> " + logName).c_str ());
}

void mv (string src, string dst,string filename) {
	cmd ("move " + src + "\\" + filename + " " + dst + "\\" + filename);
}

void dig (string url="www.xjtu.edu.cn",string server="202.117.0.20") {
	string Dig = "dig " + url + " @" + server;

	cmd (Dig,digLoc);
	cout << "dig succeeded" << endl;
//	mv (digLoc, pjLoc, logName);
}


void main () {
	dig ();
}
