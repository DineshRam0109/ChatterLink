#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define MAX_LEN 200
#define PORT 10000

using namespace std;

void receive_messages(SOCKET client_socket);
void send_messages(SOCKET client_socket);

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use inet_addr instead of inet_pton

    if (connect(client_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cerr << "Connection failed!" << endl;
        return -1;
    }

    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    send(client_socket, name.c_str(), name.size() + 1, 0);

    thread recv_thread(receive_messages, client_socket);
    thread send_thread(send_messages, client_socket);

    send_thread.join();
    recv_thread.detach();

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

void receive_messages(SOCKET client_socket) {
    while (true) {
        char message[MAX_LEN];
        int bytes_received = recv(client_socket, message, MAX_LEN, 0);
        if (bytes_received <= 0) {
            break;
        }
        cout << message << endl;
    }
}

void send_messages(SOCKET client_socket) {
    while (true) {
        string message;
        getline(cin, message);
        send(client_socket, message.c_str(), message.size() + 1, 0);

        if (message == "#exit") {
            break;
        }
    }
}
