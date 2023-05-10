
// Windows-specific socket implementation

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)

// Adapted from:
//   https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <functional>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

static SOCKET ClientSocket = INVALID_SOCKET;

int createServer(const char *port, std::function<void()> callback) {
  WSADATA wsaData;
  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;

  struct addrinfo *result = NULL;
  struct addrinfo hints;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return 1;
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  iResult = getaddrinfo(NULL, port, &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    WSACleanup();
    return 1;
  }

  // Create a SOCKET for connecting to server
  ListenSocket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }

  // Setup the TCP listening socket
  iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  freeaddrinfo(result);

  iResult = listen(ListenSocket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    printf("listen failed with error: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  while (1) {
    printf("Waiting for connection...\n");

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
      printf("accept failed with error: %d\n", WSAGetLastError());
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
    }

    printf("invoking callback...\n");
    callback();
  }

  // No longer need server socket
  closesocket(ListenSocket);
  WSACleanup();
}

int readSocket(char *recvbuf, int recvbuflen) {
  int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
  if (iResult < 0) {
    printf("recv failed with error: %d\n", WSAGetLastError());
  }
  return iResult;
}

int writeSocket(const char *sendbuf, int sendbuflen) {
  int iSendResult = send(ClientSocket, sendbuf, sendbuflen, 0);
  if (iSendResult == SOCKET_ERROR) {
    printf("send failed with error: %d\n", WSAGetLastError());
    closesocket(ClientSocket);
  }
  return iSendResult;
}

void closeSocket() {
  int iResult = shutdown(ClientSocket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    printf("shutdown failed with error: %d\n", WSAGetLastError());
  }
  printf("Connection closing...\n");
  closesocket(ClientSocket);
}

#endif
