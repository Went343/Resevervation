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

#define PORT_NUMBER 5444
#define BUFFER_SIZE 1024

using namespace std;

//Global variables for storing and accessing different parts of the game between threads
int** seats;
int rows = 0;
int cols = 0;
int totalSeats = 0;
bool filled = false;
random_device rd;
mt19937 gen(rd());

//Mutex for preventing the board prints to overlap
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

void print() {

	cout << endl;
	cout << "Current seating:" << endl;

	//Mested for loops for displaying the board
	for (int i = 0; i < rows; i++) {

		for (int j = 0; j < cols; j++) {

			//If the current seat is 0, the seat is empty
			if (seats[i][j] == 0) {

				cout << "[ ]";

			//If the current seat is 1, the seat is taken
			} else if (seats[i][j] == 1) {

				cout << "[X]";

			} 

		}

		cout << endl;

	}

	cout << endl;

}

void* client(void* cID) {
	
	int clientSocket = *((int*)cID);
	free(cID);

	int row = 0;
	int col = 0;

	cout << "Client connected. Socket: " << clientSocket << endl;
	
	string size = to_string(rows) + " " + to_string(cols);

	const char* sizes = size.c_str();

	if (write(clientSocket, sizes, strlen(sizes)) < 0) {
		cout << "Problem sending seating size information to client." << endl;
		return (void*)1;
	}

	while (!filled) {

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));

		if (read(clientSocket, buffer, sizeof(buffer) - 1) < 0) {
			cout << "Error receiving message from client." << endl;
			return (void*)1;
		}

		string message(buffer);

		int number;
		istringstream iss(message);
		string command;
		iss >> command;

		istringstream c1_iss(command);

		if (c1_iss >> number) {

			row = number;

		} else if (command == "exit") {

			break;

		}

		string command2;
		iss >> command2;
		int number2;

		istringstream c2_iss(command2);

		if (c2_iss >> number2) {

			col = number2;

		}

		if (row > rows || row < 1) {

			string er = "I am sorry, that seat does not exist. Please try again.";

			const char* error = er.c_str();

			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << endl;
				return (void*)1;
			}

			continue;

		}
		else if (col > cols || col < 1) {

			string er = "I am sorry, that seat does not exist. Please try again.";

			const char* error = er.c_str();

			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << endl;
				return (void*)1;
			}

			continue;

		}

		if (seats[row][col] == 1) {

			string er = "I am sorry, that seat is taken exist. Please try again.";

			const char* error = er.c_str();

			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << std::endl;
				return (void*)1;
			}

			continue;

		} else {

			seats[row][col] = 1;

			string er = "Your seat has been reserved.";

			const char* error = er.c_str();

			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << std::endl;
				return (void*)1;
			}

			pthread_mutex_lock(&myMutex);
			print();
			pthread_mutex_unlock(&myMutex);
			continue;

		}

	}

	return (void*)0;
}

void* supervisor(void* superID) {

	//Obtains the thread ID
	int id = (int)(long)superID;
	fflush(stdout);

	//While the game is not ended
	while (!filled) {

		//Boolean for if the nested for loops have broken due to a blank space
		bool broken = false;

		for (int i = 0; i < rows; i++) {

			//If the innermost for loop breaks, this one breaks as well
			if (broken) {

				break;

			}

			//Innermost for-loop
			for (int j = 0; j < cols; j++) {

				//If the space is empty (== 0), then broken is set to true and the innermost for loop is broken
				if (seats[i][j] == 0) {

					broken = true;
					break;

					//If the space is occupied, moves on to the next iteration of the for loop
				}
				else if (seats[i][j] == 1) {

					continue;

				}
				else {

					//Server has ran into an error
					cout << "The server has encountered an error." << endl;
					return (void*)0;

				}
			}

		}

		//If both for loops have finished with no breaking from the loops, sets filled to true
		if (!broken) {

			filled = true;

		}

	}

	return (void*)0;

}

int main(int argc, char* argv[]) {

	string ipAd = "127.0.0.1";

	int timeout = 5;

	if (argc == 2) {

		rows = atoi(argv[1]);
		cols = atoi(argv[2]);
		
	} else {

		cout << "Please input the file command as './[exec_name] [rows] [columns]', with rows and columns being an integer." << endl;
		return 1;

	}

	totalSeats = rows * cols;

	seats = new int* [rows];
	for (int i = 0; i < rows; i++) {
		seats[i] = new int[cols];
	}

	pthread_t clients[totalSeats + 1];

	for (int i = 0; i < rows; i++) {

		for (int j = 0; j < cols; j++) {

			seats[i][j] = 0;

		}

	}

	//Prompt users to include an ini file name. Change ipAd as needed

	const char* IP = ipAd.c_str();

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		cout << "Error creating socket." << endl;
		return -1;
	}

	struct sockaddr_in serverAddress {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(IP);
	serverAddress.sin_port = htons(PORT_NUMBER);

	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		cout << "Error binding socket." << endl;
		return -1;
	}

	if (listen(serverSocket, 1) < 0) {
		cout << "Error listening on socket." << endl;
		return -1;
	}

	cout << "Server listening on port " << PORT_NUMBER << endl;

	vector<pthread_t> threadIds;

	pthread_t threadId;
	pthread_create(&threadId, NULL, client, (void*)0);
	threadIds.push_back(threadId);

	while (true) {

		struct sockaddr_in clientAddress {};
		socklen_t clientAddressLength = sizeof(clientAddress);

		// Accept an incoming client connection and create a client socket
		int* clientSocket = (int*)malloc(sizeof(int));
		*clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
		if (*clientSocket < 0) {
			std::cerr << "Error accepting connection." << std::endl;
			break;
		}

		if (!filled) {

			pthread_create(&threadId, NULL, client, (void*)clientSocket);
			threadIds.push_back(threadId);
			continue;

		} else {

			break;

		}

	}

	for (pthread_t threadId : threadIds) {
		pthread_join(threadId, NULL);
	}

	close(serverSocket);

	return 0;

}