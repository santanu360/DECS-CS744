#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

const int BUFFER_SIZE = 1024;

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded>" << endl;
        return 1;
    }

    const char* serverIPPort = argv[1]; // Server's IP address and port in "serverIP:port" format
    const char* sourceCodeFile = argv[2];

    // Parse the serverIPPort to separate IP address and port
    char* serverIP = strtok(strdup(serverIPPort), ":");
    int PORT = atoi(strtok(NULL, ":"));

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        close(clientSocket);
        return 1;
    }

    // Open the source code file for reading
    int sourceCodeFileDescriptor = open(sourceCodeFile, O_RDONLY);
    if (sourceCodeFileDescriptor < 0) {
        cerr << "Error opening source code file" << endl;
        close(clientSocket);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(sourceCodeFileDescriptor, buffer, sizeof(buffer))) > 0) {
        send(clientSocket, buffer, bytesRead, 0);
        memset(buffer, 0, sizeof(buffer));

    }
    close(sourceCodeFileDescriptor);

    // Send an EOF to signal the end of the file
    shutdown(clientSocket, SHUT_WR);

    // Receive and print the result from the server
    char responseBuffer[BUFFER_SIZE];
    ssize_t receivedBytes;
    while ((receivedBytes = recv(clientSocket, responseBuffer, sizeof(responseBuffer), 0)) > 0) {
        fwrite(responseBuffer, 1, receivedBytes, stdout);
    }
    cout << endl;

    // Close the client socket
    close(clientSocket);

    return 0;
}
