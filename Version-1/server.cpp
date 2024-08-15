#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>

const int BUFFER_SIZE = 1024;

using namespace std;

// Constants for response types
const int PASS = 0;
const int COMPILER_ERROR = 1;
const int RUNTIME_ERROR = 2;
const int OUTPUT_ERROR = 3;

bool debug(int clientSocket, int& response, string& errorMsg) {
    char sourceCodeFilename[] = "server_test.cpp"; // Use the C++ source code file

    // Receive and write the source code to "p2.cpp"
    FILE* sourceCodeFile = fopen(sourceCodeFilename, "w");
    if (!sourceCodeFile) {
        perror("Error opening source code file");
        errorMsg = "Server Error : opening source code file";
        return false;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytesRead, sourceCodeFile);
        memset(buffer, 0, sizeof(buffer));
       /*if(bytesRead<=0)
        break; */
    }
    printf("ATUL DIDI ");
    fclose(sourceCodeFile);

    // Compile the source code as a C++ program
    int compileStatus = system("g++ -o program server_test.cpp 2> compile_error.txt");

    // Check for compilation errors
    if (compileStatus != 0) {
        response = COMPILER_ERROR;
        errorMsg = "Compilation failed. Error details:\n";
        char compileErrorBuffer[BUFFER_SIZE];
        FILE* compileErrorFile = fopen("compile_error.txt", "r");
        if (compileErrorFile) {
            while (fgets(compileErrorBuffer, sizeof(compileErrorBuffer), compileErrorFile)) {
                errorMsg += compileErrorBuffer;
            }
            fclose(compileErrorFile);
        } else {
            errorMsg += "Server Error : Unable to retrieve compilation error details.";
        }
        remove("compile_error.txt");
        return false;
    }

    // Run the program and capture its output and errors
    int runtimeStatus = system("./program > output.txt 2>&1");

    // Check if a runtime error occurred
    if (runtimeStatus != 0) {
        response = RUNTIME_ERROR;
        errorMsg = "Runtime error occurred\n";

        // Read the contents of "output.txt" to include in the error message
        FILE* programOutputFile = fopen("output.txt", "r");
        if (programOutputFile) {
            char programOutputBuffer[BUFFER_SIZE];
            while (fgets(programOutputBuffer, sizeof(programOutputBuffer), programOutputFile)) {
                errorMsg += programOutputBuffer;
            }
            fclose(programOutputFile);
        } else {
            errorMsg += "Server Error: executing the program";
        }

        return false;
    }

    // Compare the contents of "out.txt" and "output.txt" using the diff command
    int compareStatus = system("diff out.txt output.txt > diff_output.txt");

    if (compareStatus != 0) {
        response = OUTPUT_ERROR;
        errorMsg = "Output does not match the expected output\n";
        
        // Read the contents of "diff_output.txt" to include in the error message
        FILE* diffOutputFile = fopen("diff_output.txt", "r");
        if (diffOutputFile) {
            char diffOutputBuffer[BUFFER_SIZE];
            while (fgets(diffOutputBuffer, sizeof(diffOutputBuffer), diffOutputFile)) {
                errorMsg += diffOutputBuffer;
            }
            fclose(diffOutputFile);
        } else {
            errorMsg += "Server error : Unable to retrieve diff output details.";
        }

        return false;
    }

    response = PASS; // Passed
    errorMsg = "Program executed successfully";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        return 1;
    }
    int counter=0;
    int PORT = atoi(argv[1]); // Get the port from the command line

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error binding");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Error listening");
        exit(1);
    }

    cout << "Server listening on port " << PORT << endl;

    while (true) {
        // Accept a client connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Initialize response type and error message
        int response;
        string errorMsg;

        // Compile, run, and grade the submitted code
        bool result = debug(clientSocket, response, errorMsg);
       

        // Send the result back to the client based on the response type
        const char* responseStr = "";
        switch (response) {
            case PASS:
                responseStr = "PASS";
                break;
            case COMPILER_ERROR:
                responseStr = "COMPILER ERROR";
                break;
            case RUNTIME_ERROR:
                responseStr = "RUNTIME ERROR";
                break;
            case OUTPUT_ERROR:
                responseStr = "OUTPUT ERROR";
                break;
        }

        // Send response type and error message to the client
        string fullResponse = responseStr;
        if (!errorMsg.empty()) {
            fullResponse += "\n" + errorMsg;
        }
        send(clientSocket, fullResponse.c_str(), fullResponse.size(), 0);
        counter+=1;
        cout<<"Server Responded for the client request id :: "<<counter<<endl;
        
        // Close the client socket
        close(clientSocket);
        
    }

    close(serverSocket);

    return 0;
}

