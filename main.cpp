#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <mpi.h>
#define EMPTY -1

using namespace std;

bool equals(vector<int> a, vector<int> b) {
    if( a.size() != b.size() ) return false;
    for(int i = 0; i < (int) a.size(); i++) {
        if(a[i] != b[i]) return false;
    }
    return true;
}

class Nonogram {
    int dimension;
    vector< vector<int> > matrix, rows, columns;
private:
    bool hasChanged(vector<vector<int>> results) {
        for (int i = 0; i < dimension; i++) {
            if (results[0][i]) return true;
            if (results[1][i]) return true;
        }
        return false;
    }

public:
    Nonogram() {}
    Nonogram(int _dimension) {
        this->dimension = _dimension;

        matrix.resize(dimension);
        for(int i = 0; i < dimension; i++) matrix[i].resize(dimension, EMPTY);

        rows.resize(dimension);
        columns.resize(dimension);
    }

    void setDimension(int d) {
        this->dimension = d;
        matrix.resize(d);
        for(int i = 0; i < d; i++) matrix[i].resize(d, EMPTY);

        rows.resize(d);
        columns.resize(d);
    }

    void showMatrix() {
        cout << "Resulting grid is: " << endl;
        for(int i = 0; i < dimension; i++) {
            for(int j = 0; j < dimension; j++) {
                if(j != 0) cout << " ";
                cout << ((matrix[i][j] == 1) ? 'X' : ' ');
            }
            cout << endl;
        }
    }

    void solveRow(int idx, int& result) {
        int newVal, pos;
        bool hasChanged = false, go;

        vector<int> aux;
        vector< vector<int> > auxLines;

        for(int i = 1; i < (1 << dimension); i++) {
            go = true;
            aux.clear();
            for(int j = 0; j < dimension; j++) {
                if( (i & (1 << j)) != 0 ) newVal = 1;
                else newVal = 0;

                if(matrix[idx][j] != EMPTY and matrix[idx][j] != newVal) go = false;
                aux.push_back(newVal);
            }
            if(go) auxLines.push_back(aux);
        }

        for(int i = auxLines.size() - 1; i >= 0; i--) {
            aux.clear();
            newVal = pos = 0;
            while(pos < dimension) {
                if( auxLines[i][pos] == 0 ) {
                    if(newVal != 0) aux.push_back(newVal), newVal = 0;
                }
                else newVal++;
                pos++;
            }
            if(newVal != 0) aux.push_back(newVal);
            if( not equals(aux, rows[idx]) ) auxLines.erase(auxLines.begin() + i );
        }

        if(auxLines.size() > 0 ) {
            for(int j = 0; j < dimension; j++) {
                if(matrix[idx][j] != EMPTY) continue;
                go = true;
                newVal = auxLines[0][j];
                for(int i = 1; i < (int)auxLines.size(); i++) {
                    if(newVal != auxLines[i][j]) go = false;
                }
                if(go) matrix[idx][j] = newVal, hasChanged = true;
            }
        }

        result = hasChanged;
    }

    void solveColumn(int idx, int& result) {
        int newVal, pos;
        bool hasChanged = false, go;

        vector<int> aux;
        vector< vector<int> > auxLines;

        for(int i = 1; i < (1 << dimension); i++) {
            go = true;
            aux.clear();
            for(int j = 0; j < dimension; j++) {
                if( (i & (1 << j)) != 0 ) newVal = 1;
                else newVal = 0;

                if(matrix[j][idx] != EMPTY and matrix[j][idx] != newVal) go = false;
                aux.push_back(newVal);
            }
            if(go) auxLines.push_back(aux);
        }

        for(int i = auxLines.size() - 1; i >= 0; i--) {
            aux.clear();
            newVal = pos = 0;
            while(pos < dimension) {
                if( auxLines[i][pos] == 0 ) {
                    if(newVal != 0) aux.push_back(newVal), newVal = 0;
                }
                else newVal++;
                pos++;
            }
            if(newVal != 0) aux.push_back(newVal);
            if( not equals(aux, columns[idx]) ) auxLines.erase( auxLines.begin() + i );
        }

        if( auxLines.size() > 0 ) {
            for(int j = 0; j < dimension; j++) {
                if(matrix[j][idx] != EMPTY) continue;
                go = true;
                newVal = auxLines[0][j];
                for(int i = 1; i < (int)auxLines.size(); i++) {
                    if(newVal != auxLines[i][j]) go = false;
                }
                if(go) matrix[j][idx] = newVal, hasChanged = true;
            }
        }

        result = hasChanged;
    }

    void solve() {
        cout << "Solving...\n";
        bool finished = false;
        int loops = 0;
        while(not finished) {
            loops++;
            cout << "Solving loop:" + to_string(loops) + "\n";

//            for(int i = 0;)

            //check if sth hasChanged
            finished = not hasChanged(results);
        }
        showMatrix();
    }

     void solveThread() {
        cout << "Solving...\n";
        bool finished = false;
        int loops = 0;
        while(not finished) {
            loops++;
            cout << "Solving loop:" + to_string(loops) + "\n";
            vector<thread> rowThreads(dimension);
            vector<vector<int>> results(2);
            results[0].resize(dimension);
            results[1].resize(dimension);

            // Create a vector of threads for the rows0
            for (int i = 0; i < rows.size(); i++) {
                rowThreads[i] = thread(&Nonogram::solveRow, this, i, std::ref(results[0][i]));
            }

            // Create a vector of threads for the columns
            std::vector<std::thread> colThreads(dimension);
            for (int i = 0; i < columns.size(); i++) {
                colThreads[i] = thread(&Nonogram::solveColumn, this, i, std::ref(results[1][i]));
            }

            // Wait for the threads to finish
            for (auto &thread : rowThreads) {
                thread.join();
            }
            for (auto &thread : colThreads) {
                thread.join();
            }

            //check if sth hasChanged
            finished = not hasChanged(results);
        }
        showMatrix();
    }

    void solveMPI(int argc, char *argv[]) {
        int size, rank;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        cout << "Solving...\n";
        bool finished = false;
        vector<vector<int>> results(2);
        results[0].resize(dimension);
        results[1].resize(dimension);
        cout << "size:" + to_string(size) + "\n";
        cout << "rank:" + to_string(rank) + "\n";
        int loopsForProc = this->dimension/size;
        int restLoops = this->dimension%size;
        int loops = 0;
        while(not finished) {
            loops++;
            cout << "start loop:" + to_string(loops) + "\n";
            cout << "test1\n";
            for(int i=rank*loopsForProc; i<rank*loopsForProc+loopsForProc;i++) {
                solveRow(i, results[0][i]);
                solveColumn(i, results[1][i]);
                cout << "test2:" + to_string(i);
            }
            cout << "test3\n";
            if(restLoops>rank) {
                int i = dimension - rank;
                solveRow(i, results[0][i]);
                solveColumn(i, results[1][i]);
            }

            // Wait for the threads to finish
            MPI_Barrier(MPI_COMM_WORLD);
            cout << "end loop:" + to_string(loops) + "\n";
            //check if sth hasChanged
            finished = not hasChanged(results);
        }
        MPI_Finalize();
        showMatrix();
    }

    void prepare() {
        int response;
        cout << "Enter manually?[0/1]\n";
        cin >> response;
        getchar();
        cout << "Preparing...\n";
        if (response) { readInput(); } else { readDefault(); }
        cout << "Finished.\n";
    }

    void readDefault() {
        // Define the size of the nonogram
        int d = 10;

        // Define the clues for the rows and columns
        std::vector<std::vector<int>> rowClues = {{1, 5}, {2, 3}, {3, 2}, {1, 5}, {5, 1}, {4, 1}, {3, 3}, {2, 4}, {5, 1}, {1, 3}};
        std::vector<std::vector<int>> colClues = {{3, 2}, {1, 2, 1}, {3, 2}, {2, 2}, {2, 1, 1}, {1, 5}, {5, 1}, {1, 3}, {2, 3}, {1, 2}};
        setDimension(d);
        this->columns = colClues;
        this->rows = rowClues;
    }

    void readInput() {
        int d;
        cout << "Enter size of matrix\n";
        cin >> d;
        getchar();
        setDimension(d);
        cout << "Enter groups by joining size of each group with space e.g. 1[space]5[space]1\n";
        // reading rows
        for(int i = 0; i < dimension; i++) {
            cout << "Enter groups for row " + to_string(i) + ":";
            string line, token;
            getline(cin, line);

            stringstream ss(line);
            while(ss >> token) rows[i].push_back(stoi(token) );
        }

        // reading columns
        for(int i = 0; i < dimension; i++) {
            cout << "Enter groups for column " + to_string(i) + ":";
            string line, token;
            getline(cin, line);

            stringstream ss(line);
            while(ss >> token) columns[i].push_back( stoi(token) );
        }

        if( not cin.eof() ) {
            for(int i = 0; i < dimension; i++) {
                for(int j = 0; j < dimension; j++) cin >> matrix[i][j];
            }
        }
    }
};

    int main(int argc, char *argv[]) {
        Nonogram nonogram = Nonogram();
        nonogram.prepare();
        nonogram.solveMPI(argc, argv);
    }