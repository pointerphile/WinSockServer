#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 10000

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

	std::cout << "Ready..." << std::endl;;

	std::list<USER> userlist;
	
	while(1){
		USER user;
		SOCKET socketClient;
		SOCKADDR_IN saClient;

		int iAddLen = sizeof(saClient);
		socketClient = accept(socketListen, (SOCKADDR*)&saClient, &iAddLen);
		if (socketClient == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				std::cout << inet_ntoa(saClient.sin_addr) << ":" << ntohs(saClient.sin_port)
					<< " Failed." << std::endl;
				closesocket(socketClient);
				break;
			}
		}
		else {
			std::cout << inet_ntoa(saClient.sin_addr) << ":" << ntohs(saClient.sin_port)
				<< " Connected."<<std::endl;

			user.socketClient = socketClient;
			user.saClient = saClient;
			userlist.push_back(user);
		}

		if (userlist.size() > 0) {
			for (auto iter = userlist.begin(); iter != userlist.end(); iter++) {
				std::string strUser;
				strUser.clear();
				strUser.append(inet_ntoa(iter->saClient.sin_addr));
				strUser.append(":");
				strUser.append(std::to_string(ntohs(iter->saClient.sin_port)));

				memset(&iter->buf, 0, sizeof(char) * 256);
				int iRecvSize = recv(iter->socketClient, iter->buf, 256, 0);
				if (iRecvSize == 0) {
					std::cout << strUser << " Disconnected." << std::endl;
					closesocket(iter->socketClient);
					userlist.erase(iter);
					break;
				}
				if (iRecvSize == SOCKET_ERROR) {
					if (WSAGetLastError() != WSAEWOULDBLOCK) {
						std::cout << strUser << " Failed." << std::endl;
						closesocket(iter->socketClient);
						userlist.erase(iter);
						break;
					}
				}
				if (iRecvSize > 0) {
					if (strlen(iter->buf) > 0) {
						std::string strEcho;
						strEcho.clear();
						strEcho.append(strUser);
						strEcho.append(" : ");
						strEcho.append(iter->buf);
						std::cout << strEcho << std::endl;
						for (auto senditer = userlist.begin(); senditer != userlist.end(); senditer++) {
							int iSendSize = send(senditer->socketClient, strEcho.c_str(), strEcho.size(), 0);
							if (iSendSize == SOCKET_ERROR)
							{
								if (WSAGetLastError() != WSAEWOULDBLOCK)
								{
									std::cout << strUser << " Failed." << std::endl;
									closesocket(senditer->socketClient);
									userlist.erase(senditer);
									break;
								}
							}
						}
					}
				}
			}
		}
	}


	std::cout << "Shutting down..." << std::endl;
	closesocket(socketListen);
	WSACleanup();
	system("pause");

	return 0;
}