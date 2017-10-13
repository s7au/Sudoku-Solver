#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <vector>
#include <queue>
#include <set>

using namespace std;

// [rows][col]
static int puzzle[9][9];

// arrays to keep track of uniqueness within rows, columns, and boxes
// I denote these three structures as Rules for naming other functions
static int rows[9][9];
static int columns[9][9];
// boxes go from left to right top to bottom
static int boxes[3][3][9];

// arrays to keep track of the sum of possible numbers at each structure
// used to choose value based on LCV
static int lcvRows[9][9];
static int lcvColumns[9][9];
static int lcvBoxes[3][3][9];

void init() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			rows[i][j] = 0;
			columns[i][j] = 0;
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 9; k++) {
				boxes[i][j][k] = 0;
			}
		}
	}
}

// false when the inserted value is invalid
bool checkNewInsert(int row, int col, int num) {
	if (rows[row][num-1] || columns[col][num-1] || boxes[row/3][col/3][num-1]) {
		return false;
	}
	else {
		return true;
	}
}

// update the Rule after a single insert
void updateRules(int row, int col, int num) {
	int updateValue = 1;
	rows[row][num-1] = updateValue;
	columns[col][num-1] = updateValue;
	boxes[row/3][col/3][num-1] = updateValue;
}

// function to handle updating the puzzle
void insertIntoPuzzle(int row, int col, int num) {
	puzzle[row][col] = num;
	updateRules(row, col, num);
}

// wipes the Rule arrays and updates them from fresh
void updateAllRules() {
	init();
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (puzzle[i][j] != 0) updateRules(i,j, puzzle[i][j]);
		}
	}
}

void printPuzzle() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			cout << puzzle[i][j] << " ";
		}
		cout << endl;
	}
}

struct sudokuEntry {
	set<int> possibleNum;
	int row;
	int col;

	bool isEmpty() {
		return possibleNum.empty();
	}
	bool clear() {
		possibleNum.clear();
	}
	// remove numbers that have already been traversed
	void eraseOldNum(int num) {
		possibleNum.erase(num);
	}

	void generatePossibleNum() {
		clear();
		for (int i = 9; i > 0; i--) { // reversed so as to compare with version a
			if (checkNewInsert(row, col, i)) {
				possibleNum.insert(i);
				lcvRows[row][i-1]++;
				lcvColumns[col][i-1]++;
				lcvBoxes[row/3][col/3][i-1]++;
			}
		}
	}
	// returns the number of variables that will be given a constraint as a result of assigning this variable a value
	int varsConstraining() const {
		int vars = 0;
		for (int i = 0; i < 9; i++) {
			vars += rows[row][i] + columns[col][i] + boxes[row/3][col/3][i];
		}
		return 24 - vars;
	}

	// generates a number from possibleNum based on LCV
	int getNumber() {
		int chosenNum;
		int effect = 0;
		for (set<int>::iterator it = possibleNum.begin(); it != possibleNum.end(); it++) {
			int sum = lcvRows[row][*it-1] + lcvColumns[col][*it-1] + lcvBoxes[row/3][col/3][*it-1];
			if (sum > effect) {
				effect = sum;
				chosenNum = *it;
			}
		}
		return chosenNum;
	}
	sudokuEntry(int row, int col) : row(row), col(col) {}
};

// comparison function for priority queue
struct LessConstrained {
	bool operator()(sudokuEntry const *se1 , sudokuEntry const *se2) {
		int se1Size = se1->possibleNum.size();
		int se2Size = se2->possibleNum.size();
		if (se1Size > se2Size) {
			return true;
		} else if (se1Size == se2Size) {
			int se1Effects = se1->varsConstraining();
			int se2Effects = se2->varsConstraining();
			if (se1Effects > se2Effects) {
				return true;
			} else if (se1Effects == se2Effects) {
				return se1->row*9+se1->col > se2->row*9+se2->col;
			} else return false;
		} else return false;
	}

};

void clearLCVArrays() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			lcvRows[i][j] = 0;
			lcvColumns[i][j] = 0;
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 9; k++) {
				lcvBoxes[i][j][k] = 0;
			}
		}
	}
}

// recalculates the possibleNum field of all sudokuEntry's still in the priority queue
void recalculateAllPossibleNumbers (priority_queue<sudokuEntry*, vector<sudokuEntry*>, LessConstrained> &heap) {
	vector<sudokuEntry*> tempContainer;
	while (!heap.empty()) {
		tempContainer.push_back(heap.top());
		heap.pop();
	}
	while(!tempContainer.empty()) {
		tempContainer.back()->generatePossibleNum();
		heap.push(tempContainer.back());
		tempContainer.pop_back();
	}

}

int assignments = 0;

bool solve(priority_queue<sudokuEntry*, vector<sudokuEntry*>, LessConstrained> &heap, vector<sudokuEntry*> &btQueue) {
	// sudokuEntry *varOfInterest;
	btQueue.push_back(heap.top());
	heap.pop();
	// node of interest will always be top of btQueue
	while (true) {
		int number = btQueue.back()->getNumber(); 
		if (number == 0) { // implies no numbers to choose from
			heap.push(btQueue.back());
			btQueue.pop_back();
			if (btQueue.empty()) return false;
			puzzle[btQueue.back()->row][btQueue.back()->col] = 0;
			assignments++;
			if (assignments > 10000) return false;
			updateAllRules();
			clearLCVArrays();
			recalculateAllPossibleNumbers(heap);
		} else {
			insertIntoPuzzle(btQueue.back()->row, btQueue.back()->col, number);
			btQueue.back()->eraseOldNum(number);
			assignments++;
			if (assignments > 10000) return false;
			if (heap.empty()) return true;
			clearLCVArrays();
			recalculateAllPossibleNumbers(heap);
			btQueue.push_back(heap.top());
			heap.pop();
		}
	}
}


int main() {
	init();
	clearLCVArrays();
	vector<sudokuEntry*> backTrackCells;
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			cin >> puzzle[i][j];
			if (puzzle[i][j] != 0) {
				updateRules(i, j, puzzle[i][j]);
			} else {
				backTrackCells.push_back(new sudokuEntry(i, j));
			}
		}
	}

	for (vector<sudokuEntry*>::iterator it = backTrackCells.begin(); it < backTrackCells.end(); it++) {
		(*it)->generatePossibleNum();
	}

	priority_queue<sudokuEntry*, vector<sudokuEntry*>, LessConstrained> cellHeap;
	while (!backTrackCells.empty()) {
		cellHeap.push(backTrackCells.back());
		backTrackCells.pop_back();
	}

	bool result = solve(cellHeap, backTrackCells);
	for (vector<sudokuEntry*>::iterator it = backTrackCells.begin(); it < backTrackCells.end(); it++) {
		delete *it;
	}
	
	
	if (!result) {
		cout << "Puzzle is unsolvable or takes too long" << endl;
		return 0;
	}

	printPuzzle();
	cout << "Assignments made: " << assignments << endl;
	// cout << assignments << endl;
}