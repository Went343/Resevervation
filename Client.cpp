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

#define BUFFER_SIZE 1024

using namespace std;

random_device rd;
mt19937 gen(rd());

//Function for checking if the IP address is a valid input from the ini file
	//Code based on code given by Chatgpt
bool isValidIPAddress(const std::string& ipAddress) {

	const regex pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

	return regex_match(ipAddress, pattern);

}

//Function for checking if the Port address number is a valid input from the ini file
	//Code based on code given by Chatgpt
bool isValidPortNumber(const std::string& portNumber) {
	
	if (portNumber.empty()) {
		return false;
	}

	
	for (char c : portNumber) {
		if (!std::isdigit(c)) {
			return false;
		}
	}

	
	int port = std::stoi(portNumber);
	if (port < 0 || port > 65535) {
		return false;
	}

	return true;
}

//Main function, takes in arguments for whether the user will use 'automatic' or 'manual' mode, as well as a second argument for an ini file of type .txt
int main(int argc, char* argv[]) {

	//Default values for IP Address, Port Number, Timeout, and a boolean for if the user is using automatic mode
	string ipAd = "127.0.0.1";
	const char* IP = ipAd.c_str();
	int PORT_NUMBER = 5444;
	int timed = 2;
	bool automatic = false;

	//Ints to store the rows and columns of the seating arrangement
	int rows = 0;
	int cols = 0;

	//Strings to hold the first command and the filename, if there is any.
	string cmdl1;
	string filename;

	//If the user only specified automatic or manual:
	if (argc == 2) {

		cmdl1 = argv[1];

		//If the user chose manual mode, system notifies the user and keeps the 'automatic' variable at false
		if (cmdl1 == "manual") {

			cout << "Starting the client in manual mode." << endl;

		//If the user chose automatic mode, system notifies the user and sets the 'automatic' variable to true
		} else if (cmdl1 == "automatic") {

			automatic = true;
			cout << "Starting the client in automatic mode." << endl;

		//If the user input an invalid command, system notifies the user and closes hte program
		} else {

			cout << "Please input a valid starting command. For help, please refer to the README.txt file." << endl;
			exit(1);

		}

	//If the user specified the mode and an ini file:
	} else if (argc == 3) {

		cmdl1 = argv[1];
		filename = argv[2];

		//If the user chose manual mode, system notifies the user and keeps the 'automatic' variable at false
		if (cmdl1 == "manual") {

			cout << "Starting the client in manual mode." << endl;

		//If the user chose automatic mode, system notifies the user and sets the 'automatic' variable to true
		} else if (cmdl1 == "automatic") {

			automatic = true;
			cout << "Starting the client in automatic mode." << endl;

		//If the user input an invalid command, system notifies the user and closes hte program
		} else {

			cout << "Please input a valid starting command. For help, please refer to the README.txt file." << endl;
			exit(1);

		}

		//Tries to open the file specified by the user
		ifstream file(filename);

		//If the file is found:
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

		//Error message for if no file could be found
		} else {

			cout << "No ini file could be found. Using default values." << endl;
			cout << "Make sure that the ini file is inside the same directory as the client file." << endl;
			cout << "Make sure to also check the README for information on how to correctly layout the ini file and how to type the name into the client program." << endl;

		}

	}

	//Creates a client socket
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0) {
		cout << "Error creating socket." << endl;
		return -1;
	}

	cout << "Socket created" << endl;

	//Creates a server address
	struct sockaddr_in serverAddress {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(IP);
	serverAddress.sin_port = htons(PORT_NUMBER);

	cout << "Server address created" << endl;

	//Connects to the server via the client socket and server address
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		cout << "Error connecting to the server." << endl;
		return -1;
	}

	cout << "Connected to the server." << endl;

	//Recieves a message from the server of the seating arrangement's size (in N x M)
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	if (read(clientSocket, buffer, sizeof(buffer) - 1) < 0) {
		cout << "Error receiving message from server." << endl;
		return -1;
	}

	string message(buffer);

	int num;
	istringstream iss(message);
	string cmd;
	iss >> cmd;

	istringstream cmd_iss(cmd);

	//If the first number sent is an int, stores it in 'rows'
	if (cmd_iss >> num) {

		rows = num;

	//If not, sends an error message and ends the program
	} else {

		cout << "An error has occured with the Server's data." << endl;
		return 1;

	}

	string cmd2;
	iss >> cmd2;
	int num2;

	istringstream cmd2_iss(cmd2);

	//If the second number sent is an int, stores it in 'cols'
	if (cmd2_iss >> num2) {

		cols = num2;

	//If not, sends an error message and ends the program
	} else {

		cout << "An error has occured with the Server's data." << endl;
		return 1;

	}

	string error;
	iss >> error;

	//If the server somehow accidentally sent over more than two commands, sends an error message and ends the program
	if (!error.empty()) {

		cout << "An error has occured with the Server's data." << endl;
		return 1;

	}

	//Int for keeping track of how many connection errors the client has with the server in case it needs to close
	int intER = 0;

	while (true) {

		//If too many connection errors occur, ends while loop
		if (intER > 5) {

			cout << "Too many errors when connecting with the server. Please ensure you and the server have the same IP Address and Port Address." << endl;
			break;

		}

		//Code for if the client program is in Automatic mode
		if (automatic) {

			//Randomly genereated ints to store the row and column chosen
			uniform_int_distribution<> disR(1, rows);
			int randRow = disR(gen);
			uniform_int_distribution<> disC(1, cols);
			int randCol = disC(gen);

			//Notifies the user what seat is about to be reserved
			cout << "Reserving Seat: " << randRow << " - " << randCol << endl;

			//Sends the two ints as a char array separated by a " " char
			string size = to_string(randRow) + " " + to_string(randCol);
			const char* sizes = size.c_str();
			if (write(clientSocket, sizes, strlen(sizes)) < 0) {
				cout << "Problem sending seating choice information to client." << endl;
				intER += 1;
				continue;
			}

			//Recieves a response from the server on how the seat registration went
			char buff[BUFFER_SIZE];
			memset(buff, 0, sizeof(buff));
			if (read(clientSocket, buff, sizeof(buff) - 1) < 0) {
				cout << "Error receiving message from client." << endl;
				intER += 1;
				return -1;
			}

			string response(buff);

			//If the client recieves 'break1', that means that the Server's seats are full, and notifies the user before ending the program
			if (response == "break1") {

				cout << "The show has sold out!Closing the server now, hope you all enjoy the show!" << endl;
				return 0;

			//If the client recieves 'break2', that means that the Server had too many problems recieving messages from the client, and notifies the user before ending the program
			} else if (response == "break2") {

				cout << "You have ran into too many errors when trying to connect to the server." << endl;
				cout << "We are going to disconnect you, but feel free to reconnect and try again." << endl;
				return 0;

			//If the seat registration is successfully checked, then it display's the server's response for the user and sleeps for two seconds to not overload the server with requests
			} else {

				cout << response << endl;
				this_thread::sleep_for(chrono::seconds(2));

			}
			
		//Code for if the client program is in manual mode
		} else {
		
			int row;
			int col;

			string newIn;

			//Obtains the user's request seat in N x M format
			cout << "What seat would you like to reserve? Seats are selected in an N x M format (do not type the x inbetween): ";

			getline(cin, newIn);

			// Stores the first word in the command
			istringstream iss2(newIn);
			string command1;
			iss2 >> command1;
			int number;

			istringstream command1_iss(command1);

			//If the first set of chars is an int, stores it in row
			if (command1_iss >> number) {

				row = number;

			//If the user wants to exit instead, they can type exit as the first command which will close the client
			} else if (command1 == "exit") {

				cout << "Closing the client. Have a nice day!." << endl;
				break;

			//If the user didn't type an int or 'exit', they are notified that their command was incorrect and has them try again
			} else {

				cout << "Please input a valid command." << endl;
				continue;

			}

			string command2;
			iss2 >> command2;
			int number2;

			istringstream command2_iss(command2);

			//If the second set of chars is an int, stores it in col
			if (command2_iss >> number2) {

				col = number2;

			//If the user didn't type an int, they are notified that their command was incorrect and has them try again
			} else {

				cout << "Please input a valid command." << endl;
				continue;

			}

			string wrongCommand;
			iss2 >> wrongCommand;

			//If they type anything after the two ints, they are asked to try again with a correct input
			if (!wrongCommand.empty()) {

				cout << "No command should be input after the row and column selection. Please input a valid command." << endl;
				continue;

			}

			//Sends the user-selected seat over to the server
			string size = to_string(row) + " " + to_string(col);
			const char* sizes = size.c_str();
			if (write(clientSocket, sizes, strlen(sizes)) < 0) {
				cout << "Problem sending seating choice information to client." << endl;
				intER += 1;
				continue;
			}

			//Recieves a response from the server
			char buff[BUFFER_SIZE];
			memset(buff, 0, sizeof(buff));
			if (read(clientSocket, buff, sizeof(buff) - 1) < 0) {
				cout << "Error receiving message from client." << endl;
				intER += 1;
				return -1;
			}

			string response(buff);

			//If the client recieves 'break1', that means that the Server's seats are full, and notifies the user before ending the program
			if (response == "break1") {

				cout << "The show has sold out!Closing the server now, hope you all enjoy the show!" << endl;
				break;

			//If the client recieves 'break2', that means that the Server had too many problems recieving messages from the client, and notifies the user before ending the program
			} else if (response == "break2") {

				cout << "You have ran into too many errors when trying to connect to the server." << endl;
				cout << "We are going to disconnect you, but feel free to reconnect and try again." << endl;
				break;
			
			//If the seat registration is successfully checked, then it display's the server's response for the user
			} else {

				cout << response << endl;

			}
		}

	}

	return 0;

}