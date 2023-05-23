#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <random>

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
			if (gameBoard[i][j] == 0) {

				cout << "[ ]";

			//If the current seat is 1, the seat is taken
			} else if (gameBoard[i][j] == 1) {

				cout << "[X]";

			//If the server encounters a seat that is not 0 or 1, prints an error and exits
			} else {

				cout << "An error has occurred. Please try again." << endl;
				exit(-1);

			}

		}

		cout << endl;

	}

	cout << endl;

}

void* reserveSeat(void* seatID) {

	int id = (int)(long)seatID;
	fflush(stdout);

	while (true) {

		int row;
		int col;

		//Requests a row and column for a seat from the client using the id

		if (row > rows || row < 1) {

			//Return seat does not exist error to client
			return (void*)0;

		}
		else if (col > cols || col < 1) {

			//Return seat does not exist error to client
			return (void*)0;

		}

		if (seats[row][col] == 1) {

			//Return seat taken message to client
			return (void*)0;

		}
		else {

			seats[row][col] = 1;
			//Return seat reserved message to client

		}

		pthread_mutex_lock(&myMutex);
		print();
		pthread_mutex_unlock(&myMutex);

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
			}

		}

		//If both for loops have finished with no breaking from the loops, sets gameOver to true
		if (!broken) {

			gameOver = true;

		}

	}

	return (void*)0;

}

int main(int argc, char* argv[]) {

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

	pthread_create(&clients[0], 0, supervisor, (void*)(long)0);

	for (int i = 1; i < (totalPlayers + 1); i++) {

		while (true) {

			//Once socket is found, change wait to false

		}

		if (!filled) {

			pthread_create(&clients[i], 0, reserveSeat, (void*)(long)i);
			break;

		} else {

			//Close program

		}

		if (filled) {

			break;

		}

	}

}