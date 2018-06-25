#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <list>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 10000
const int g_MaxUser = 3;

struct USER {
	SOCKET socketClient;
	SOCKADDR_IN saClient;
	char buf[256] = { 0 };
};

int SetNonBlockingSocket(SOCKET sock, u_long iMode) {
	int iResult;
	iResult = ioctlsocket(sock, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		return -1;
	}

	return iResult;
}

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

	SetNonBlockingSocket(socketListen, TRUE);

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

	
	
	int iUserCount = 0;
	std::list<USER> userlist;
	
	while(iUserCount < g_MaxUser){
		USER user;
		SOCKET socketClient;
		SOCKADDR_IN saClient;

		int iAddLen = sizeof(saClient);
		socketClient = accept(socketListen, (SOCKADDR*)&saClient, &iAddLen);
		if (socketClient == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				std::cout << "立加 角菩" << std::endl;
				break;
			}
		}
		else {
			std::cout << std::endl;
			std::cout << "立加磊 IP : " << inet_ntoa(saClient.sin_addr)
				<< " : " << ntohs(saClient.sin_port) << std::endl;

			user.socketClient = socketClient;
			user.saClient = saClient;
			userlist.push_back(user);
			iUserCount++;
		}
	}

	std::cout << "test" << std::endl;

	while (userlist.size() > 0) {
		for (auto iter = userlist.begin(); iter != userlist.end(); ++iter) {
			memset(&iter->buf, 0, sizeof(char) * 256);
			int iRecvSize = recv(iter->socketClient, iter->buf, 256, 0);
			if (iRecvSize == 0) {
				std::cout << "立加 辆丰 IP : " << inet_ntoa(iter->saClient.sin_addr)
					<< " : " << ntohs(iter->saClient.sin_port) << std::endl;
				closesocket(iter->socketClient);
				userlist.erase(iter);
				break;
			}
			if (iRecvSize == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					std::cout << "立加 角菩" << std::endl;
					break;
				}
			}
			if (iRecvSize > 0) {
				break;
			}

		}
	}

	std::cout << "Shutting down..." << std::endl;
	closesocket(socketListen);
	WSACleanup();
	system("pause");

	return 0;
}