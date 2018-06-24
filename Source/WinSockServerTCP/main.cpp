#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 10000

int main(int argc, char* argv[]) {
	std::cout << "PPServer" << std::endl;

	int iRet = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	SOCKET socketListen;
	socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if (socketListen == INVALID_SOCKET) {
		return -1;
	}

	SOCKADDR_IN saServer;
	saServer = { 0 };
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(SERVER_PORT);
	saServer.sin_addr.s_addr = htonl(INADDR_ANY);
	iRet = bind(socketListen, (SOCKADDR*)&saServer, sizeof(saServer));
	if (iRet == SOCKET_ERROR) {
		return -1;
	}
	
	iRet = listen(socketListen, SOMAXCONN);
	if (iRet == SOCKET_ERROR) {
		return -1;
	}

	std::cout << "Ready...";

	SOCKET socketClient;
	SOCKADDR_IN saClient;
	char buf[256] = { 0 };
	int iLen;
	
	while(1){
		int iAddLen = sizeof(saClient);
		socketClient = accept(socketListen, (SOCKADDR*)&saClient, &iAddLen);
		if (socketClient == INVALID_SOCKET) {
			break;
		}
		std::cout << std::endl;
		std::cout << "Á¢¼ÓÀÚ IP : " << inet_ntoa(saClient.sin_addr) 
			<< " : " << ntohs(saClient.sin_port) << std::endl;

		closesocket(socketClient);
	}
	closesocket(socketListen);
	WSACleanup();

	return 0;
}