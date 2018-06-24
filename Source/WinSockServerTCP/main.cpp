#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

int main(char argc, char* argv[]) {
	std::cout << "PPServer" << std::endl;

	int iRet = 0;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	SOCKET socketListen;
	socketListen = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN saServer;
	saServer = { 0 };
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(10000);// host byte 정렬 to network byte 정렬 short
	saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
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
	
	while(1){
		int iAddLen = sizeof(saClient);
		socketClient = accept(socketListen, (SOCKADDR*)&saClient, &iAddLen);
		if (socketClient == INVALID_SOCKET) {
			break;
		}
		std::cout << std::endl;
		std::cout << "접속자 IP : " << inet_ntoa(saClient.sin_addr) 
			<< " : " << ntohs(saClient.sin_port) << std::endl;

		char buf[256] = { 0 };
		int iLen;

		closesocket(socketClient);
	}
	closesocket(socketListen);
	WSACleanup();

	return 0;
}