#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm> // Added this for remove_if

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

#define MAX_LEN 200
#define PORT 10000

using namespace std;

struct Client {
    SOCKET socket;
    string name;
};

vector<Client> clients;
mutex client_mutex;

void broadcast_message(const string& message, SOCKET sender_socket);
void handle_client(SOCKET client_socket);
void remove_client(SOCKET client_socket);

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cerr << "Bind failed!" << endl;
        return -1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        cerr << "Listen failed!" << endl;
        return -1;
    }

    cout << "Server started on port " << PORT << endl;

    while (true) {
        sockaddr_in client;
        int client_size = sizeof(client);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client, &client_size);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Accept failed!" << endl;
            continue;
        }

        thread t(handle_client, client_socket);
        t.detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void handle_client(SOCKET client_socket) {
    char name[MAX_LEN];
    recv(client_socket, name, MAX_LEN, 0);

    {
        lock_guard<mutex> lock(client_mutex);
        clients.push_back({client_socket, name});
    }

    string welcome = string(name) + " has joined the chat!";
    broadcast_message(welcome, client_socket);

    while (true) {
        char message[MAX_LEN];
        int bytes_received = recv(client_socket, message, MAX_LEN, 0);
        if (bytes_received <= 0) {
            break;
        }

        string full_message = string(name) + ": " + string(message);
        broadcast_message(full_message, client_socket);
    }

    remove_client(client_socket);
    closesocket(client_socket);
}

void broadcast_message(const string& message, SOCKET sender_socket) {
    lock_guard<mutex> lock(client_mutex);
    for (const auto& client : clients) {
        if (client.socket != sender_socket) {
            send(client.socket, message.c_str(), message.size() + 1, 0);
        }
    }
}

void remove_client(SOCKET client_socket) {
    lock_guard<mutex> lock(client_mutex);
    clients.erase(remove_if(clients.begin(), clients.end(),
                            [client_socket](const Client& c) { return c.socket == client_socket; }),
                  clients.end());
}
