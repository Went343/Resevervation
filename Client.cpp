#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <ctime>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char* argv[]) {

	string input;
	char* IP = "127.0.0.1"

	while (true) {

		//Wait for connection to server

		getline(cin, input);

		// Stores the first word in the command
		istringstream iss(input);
		string command;
		iss >> command;

	}

}