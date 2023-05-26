#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <regex>
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
#include <chrono>
#include <thread>
#include <fcntl.h>

#define BUFFER_SIZE 1024

using namespace std;



//Global variables for storing and accessing different parts of the game between threads
int** seats;
int rows = 0;
int cols = 0;
int totalSeats = 0;
int timed = 5;
int serverSocket;
bool filled = false;
random_device rd;
mt19937 gen(rd());
vector<pthread_t> threadIds;
pthread_t threadId;

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

//Function for checking if the IP address is a valid input from the ini file
	//Code based on code given by Chatgpt
bool isValidIPAddress(const std::string& ipAddress) {
	
	const regex pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

	return regex_match(ipAddress, pattern);

}

//Function for checking if the Port address number is a valid input from the ini file
	//Code based on code given by Chatgpt
bool isValidPortNumber(const std::string& portNumber) {
	// Check if the string is not empty
	if (portNumber.empty()) {
		return false;
	}

	// Check if every character is a digit
	for (char c : portNumber) {
		if (!std::isdigit(c)) {
			return false;
		}
	}

	// Convert the string to an integer and check the range
	int port = std::stoi(portNumber);
	if (port < 0 || port > 65535) {
		return false;
	}

	return true;
}

//Client function for handling a client's actions
void* client(void* cID) {
	
	//Stores the client socket
	int clientSocket = *((int*)cID);
	free(cID);

	//Ints for storing the row and column selected by the user
	int row = 0;
	int col = 0;

	//Sends over the size of the seating arrangement to the client
	string size = to_string(rows) + " " + to_string(cols);
	const char* sizes = size.c_str();
	if (write(clientSocket, sizes, strlen(sizes)) < 0) {
		cout << "Problem sending seating size information to client." << endl;
		return (void*)1;
	}

	//Variable for storing the number of encountered connection errors
	int erTest = 0;

	//While the seats are not full:
	while (!filled) {

		//If more than 5 errors occur, breaks the loop
		if (erTest> 5) {

			break;

		}

		//Recieves message from client
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));
		if (read(clientSocket, buffer, sizeof(buffer) - 1) < 0) {
			cout << "Error receiving message from client." << endl;
			erTest += 1;

			//Attempts to send an error message to the client, just in case the error occurred from the user taking too long to choose a seat
				//KNOWN BUG: Timeout currently doesn't work as intended. I tried to implement the timout, but it was breaking the basic logic, so I had to scrap it due to time
			string err = "You might have taken too long or typed your selection wrong. Please try again.";
			const char* errors = err.c_str();
			if (write(clientSocket, errors, strlen(errors)) < 0) {
				cout << "Problem sending error message to client." << endl;
				return (void*)1;
			}

			continue;
		}

		string message(buffer);

		int number;
		istringstream iss(message);
		string command;
		iss >> command;

		istringstream c1_iss(command);

		//Sees if the first command is an int, if so it stores the int in 'row'
		if (c1_iss >> number) {

			row = number;

		}

		string command2;
		iss >> command2;
		int number2;

		istringstream c2_iss(command2);

		//Sees if the second command is an int, if so it stores the int in 'col'
		if (c2_iss >> number2) {

			col = number2;

		}

		//If the row is outside of the seating arrangement, notifies the client
		if (row > rows || row < 1) {

			string er = "I am sorry, that seat does not exist. Please try again.";
			const char* error = er.c_str();
			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << endl;
				return (void*)1;
			}

			continue;

		//If the column is outside of the seating arrangment, notifies the client
		} else if (col > cols || col < 1) {

			string er = "I am sorry, that seat does not exist. Please try again.";
			const char* error = er.c_str();
			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << endl;
				return (void*)1;
			}

			continue;

		}

		//If the seat is taken, there will be a 1 in the corresponding seat on the 2D array, and the server will notify the client
		if (seats[row-1][col-1] == 1) {

			string er = "I am sorry, that seat is taken. Please try again.";
			const char* error = er.c_str();
			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << std::endl;
				return (void*)1;
			}

			continue;

		//If the seat is not taken, there will be a 0 in the corresponding seat on the 2D array, and the server will reserve the seat and notify the client
		} else {

			seats[row-1][col-1] = 1;

			string er = "Your seat has been reserved.";
			const char* error = er.c_str();
			if (write(clientSocket, error, strlen(error)) < 0) {
				cout << "Problem sending error message to client." << std::endl;
				return (void*)1;
			}

			//Prints off the board for every successful seat registration to show the new seating
			pthread_mutex_lock(&myMutex);
			print();
			pthread_mutex_unlock(&myMutex);
			continue;

		}

	}

	//If Else If statement to choose which kind of "break" response to send to the user, break1 being for if the seats are full, and break2 being for too many connection errors
	if (erTest <= 5) {

		string er = "break1";
		const char* error = er.c_str();
		if (write(clientSocket, error, strlen(error)) < 0) {
			cout << "Problem sending message to client." << endl;
			return (void*)1;
		}

	} else {

		string er = "break2";
		const char* error = er.c_str();
		if (write(clientSocket, error, strlen(error)) < 0) {
			cout << "Problem sending error message to client." << endl;
			return (void*)1;
		}

	}

	return (void*)0;
}

//Supervisor thread for updating the seating status to filled or not filled
void* supervisor(void* superID) {

	//Obtains the thread ID
	int id = (int)(long)superID;
	fflush(stdout);

	//While the seats are not full
	while (!filled) {

		//Boolean for if the nested for loops have broken due to an empty seat
		bool broken = false;

		for (int i = 0; i < rows; i++) {

			//If the innermost for loop breaks, this one breaks as well
			if (broken) {

				break;

			}

			//Innermost for-loop
			for (int j = 0; j < cols; j++) {

				//If the seat is empty (== 0), then broken is set to true and the innermost for loop is broken
				if (seats[i][j] == 0) {

					broken = true;
					break;

				//If the seat is occupied, moves on to the next iteration of the for loop
				} else if (seats[i][j] == 1) {

					continue;
				
				//Server has ran into an error
				} else {

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

	//If for whatever reason the while loop breaks, sets filled to true as well so that the server wont stay open
	filled = true;

	return (void*)0;

}

//Function to end the game so that the server does not keep the port open looking for clients
void* gameEnder(void* id) {

	//Infinite loop that prevents the gameEnder function from moving on while the board is not full
	while (!filled) {

	}

	//Closes every thread
	for (pthread_t threadId : threadIds) {
		pthread_join(threadId, NULL);
	}

	//Notifies the server user that the server is closed
	cout << "The show has sold out! Closing the server now, hope you all enjoy the show!" << endl;

	//Sleeps for 2 seconds to allow the clients to disconnect
	this_thread::sleep_for(chrono::seconds(2));

	//Closes the socket
	close(serverSocket);

	//Ends the program
	exit(0);

	return (void*)0;
}

//Main function
int main(int argc, char* argv[]) {

	//Variables for storing the default IP, Port Number, and a boolean for if the user wants to close the serverSocket
	string ipAd = "127.0.0.1";
	int PORT_NUMBER = 5444;
	bool closing = false;

	string closed = argv[1];

	//If there is 2 commands, then tries to assign them numbers
	if (argc == 3) {

		rows = atoi(argv[1]);
		cols = atoi(argv[2]);

	//If the first command is "close", sets closing to true so that the program can close the serverSocket
	} else if (closed == "close") {

		closing = true;

	//IF the user incorrectly input the commands, displays an error and closes the server
	} else {

		cout << "Please input the file command as './[exec_name] [rows] [columns]', with rows and columns being an integer." << endl;
		return 1;

	}

	//Sets totalSeats to the amount of rows * columns
	totalSeats = rows * cols;

	//Creates the array
	seats = new int* [rows];
	for (int i = 0; i < rows; i++) {
		seats[i] = new int[cols];
	}

	//Makes a pthread_t of clients
	pthread_t clients[totalSeats + 1];

	//Populates the array with empty seats (aka 0s)
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			seats[i][j] = 0;
		}
	}

	//Asks the user to input the filename
	cout << "Please input the name of the .txt file you would like to use as an ini file. Make sure to include '.txt' at the end of the name." << endl;
	cout << "If you would like to not use a file, please type 'cont' or anything without .txt at the end to use the default IP and Port." << endl;
	
	string input;
	getline(cin, input);

	// Stores the first word in the command
	istringstream iss(input);
	string filename;
	iss >> filename;

	//Attempts to open the file
	ifstream file(filename);
	
	//If the file exists
	if (file) {

		string line;
		
		getline(file, line);

		//Checks to see if the IP address given is correct by calling 'isValidIPAddress()', notifies the user of the result
		if (isValidIPAddress(line)) {

			cout << "The IP address from " << filename << " is valid." << endl;

		} else {

			cout << "The IP address from " << filename << " is invalid. Using default IP address." << endl;

		}

		getline(file, line);

		//Checks to see if the Port Number give is correct by calling 'isValidPortNumber()', notifies the user of the result
		if (isValidPortNumber(line)) {

			cout << "The Port Number from " << filename << " is valid." << endl;
			PORT_NUMBER = stoi(line);

		} else {

			cout << "The Port Number from " << filename << " is invalid. Using default Port Number." << endl;

		}

		getline(file, line);

		//If the third line (or the timeout value), can be converted to an int, it is see as valid and the timeout value is changed to it
		try {
				
			timed = stoi(line);
			cout << "The timeout value from " << filename << " is valid." << endl;

		} catch (exception e) {

			cout << "The timeout value from " << filename << " is invalid. Using default timeout value." << endl;
			timed = 5;

		}

	} else if (filename == "cont") {

		cout << "No file chosen. Using default values." << endl;

	} else {

		cout << "No ini file could be found. Using default values." << endl;
		cout << "Make sure that the ini file is inside the same directory as the client file." << endl;

	}

	const char* IP = ipAd.c_str();

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		cout << "Error creating socket." << endl;
		return -1;
	}

	if (closing) {

		close(serverSocket);

		return 0;

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

	if (closing) {

		close(serverSocket);

		return 0;

	}

	cout << "Server listening on port " << PORT_NUMBER << endl;

	
	pthread_create(&threadId, NULL, supervisor, (void*)0);
	pthread_create(&threadId, NULL, gameEnder, (void*)1);
	threadIds.push_back(threadId);

	while (true) {

		if (!filled) {

			struct sockaddr_in clientAddress {};
			socklen_t clientAddressLength = sizeof(clientAddress);

			// Accept an incoming client connection and create a client socket
			int* clientSocket = new int;
			*clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
			if (clientSocket < 0) {
				cout << "Error establishing client socket connection." << endl;
				continue;
			}

			pthread_create(&threadId, NULL, client, (void*)clientSocket);
			threadIds.push_back(threadId);

			continue;

		} else {

			cout << "The show has sold out! Closing the server now, hope you all enjoy the show!" << endl;

			break;

		}

	}

	cout << "Ended." << endl;

	close(serverSocket);

	return 0;

}