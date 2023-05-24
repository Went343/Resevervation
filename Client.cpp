#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

using namespace std;

random_device rd;
mt19937 gen(rd());

int main(int argc, char* argv[]) {

	string ipAd = "127.0.0.1";
	const char* IP = ipAd.c_str();
	int PORT_NUMBER = 5444;
	int TIMEOUT;
	bool automatic = false;

	int rows = 0;
	int cols = 0;

	string cmdl1;
	string cmdl2;

	if (argc == 1) {

		cmdl1 = atoi(argv[1]);

		if (cmdl1 == "manual") {

			cout << "Starting the client in manual mode." << endl;

		} else if (cmdl1 == "automatic") {

			automatic = true;

		} else {

			cout << "Please input a valid starting command. For help, please refer to the README.txt file." << endl;
			exit(1);

		}

	} else if (argc == 2) {

		cmdl1 = atoi(argv[1]);
		cmdl2 = atoi(argv[2]);

		if (cmdl1 == "manual") {

			cout << "Starting the client in manual mode." << endl;

		} else if (cmdl1 == "automatic") {

			automatic = true;

		} else {

			cout << "Please input a valid starting command. For help, please refer to the README.txt file." << endl;
			exit(1);

		}

		//Check for file's existence using cmdl2, else throw error and use default values

	}

	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		cout << "Error creating socket." << endl;
		return -1;
	}

	struct sockaddr_in serverAddress {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(IP);
	serverAddress.sin_port = htons(PORT_NUMBER);

	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		cout << "Error connecting to the server." << endl;
		return -1;
	}

	cout << "Connected to the server." << endl;

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	if (read(clientSocket, buffer, sizeof(buffer) - 1) < 0) {
		cout << "Error receiving message from client." << endl;
		return -1;
	}

	string message(buffer);

	int num;
	istringstream iss(message);
	string cmd;
	iss >> cmd;

	istringstream cmd_iss(cmd);

	if (cmd_iss >> num) {

		rows = num;

	} else if (cmd == "exit") {

		cout << "Closing the client program. Have a nice day." << endl;
		close(clientSocket);
		return 0;

	} else {

		cout << "An error has occured with the Server's data." << endl;
		close(clientSocket);
		return 1;

	}

	string cmd2;
	iss >> cmd2;
	int num2;

	istringstream cmd2_iss(cmd2);

	if (cmd2_iss >> num2) {

		cols = num2;

	} else {

		cout << "An error has occured with the Server's data." << endl;
		close(clientSocket);
		return 1;

	}

	string error;
	iss >> error;

	if (!error.empty()) {

		cout << "An error has occured with the Server's data." << endl;
		close(clientSocket);
		return 1;

	}

	int intER = 0;

	while (true) {

		if (intER > 5) {

			cout << "Too many errors when connecting with the server. Please ensure you and the server have the same IP Address and Port Address." << endl;
			break;

		}

		if (automatic) {

			uniform_int_distribution<> disR(0, rows - 1);
			int randRow = disR(gen);

			uniform_int_distribution<> disC(0, cols - 1);
			int randCol = disC(gen);

			string size = to_string(randRow) + " " + to_string(randCol);

			const char* sizes = size.c_str();

			if (write(clientSocket, sizes, strlen(sizes)) < 0) {
				cout << "Problem sending seating choice information to client." << endl;
				intER += 1;
				continue;
			}

			char buff[BUFFER_SIZE];
			memset(buff, 0, sizeof(buff));

			if (read(clientSocket, buff, sizeof(buff) - 1) < 0) {
				cout << "Error receiving message from client." << endl;
				intER += 1;
				return -1;
			}

			string response(buff);

			cout << response << endl;

		} else {
		
			int row;
			int col;

			string input;

			cout << "What seat would you like to reserve? Seats are selected in an N x M format: ";

			getline(cin, input);

			// Stores the first word in the command
			istringstream iss(input);
			string command1;
			iss >> command1;
			int number;

			istringstream command1_iss(command1);

			if (command1_iss >> number) {

				row = number;

			} else if (command1 == "exit") {

				cout << "Closing the client. Have a nice day!." << endl;
				continue;

			} else {

				cout << "Please input a valid command." << endl;
				continue;

			}

			string command2;
			iss >> command2;
			int number2;

			istringstream command2_iss(command2);

			if (command2_iss >> number2) {

				col = number2;

			} else {

				cout << "Please input a valid command." << endl;
				continue;

			}

			string wrongCommand;
			iss >> wrongCommand;

			if (!wrongCommand.empty()) {

				cout << "No command should be input after the row and column selection. Please input a valid command." << endl;

			}

			string size = to_string(row) + " " + to_string(col);

			const char* sizes = size.c_str();

			if (write(clientSocket, sizes, strlen(sizes)) < 0) {
				cout << "Problem sending seating choice information to client." << endl;
				intER += 1;
				continue;
			}

			char buff[BUFFER_SIZE];
			memset(buff, 0, sizeof(buff));

			if (read(clientSocket, buff, sizeof(buff) - 1) < 0) {
				cout << "Error receiving message from client." << endl;
				intER += 1;
				return -1;
			}

			string response(buff);

			cout << response << endl;

		}

	}

	close(clientSocket);

	return 0;

}