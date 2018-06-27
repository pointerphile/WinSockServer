#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <functional>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 10000
#define BUFFERSIZE 512

struct USER {
	SOCKET socketClient;
	SOCKADDR_IN saClient;
	char buf[BUFFERSIZE + 0] = { 0 };
};

void err_display() {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, nullptr
	);
	std::cout << (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
}

int SetNonBlockingSocket(SOCKET sock, u_long iMode) {
	int iResult;
	iResult = ioctlsocket(sock, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		return -1;
	}

	return iResult;
}

bool GetServerShutdown(bool* flag) {
	while (1) {
	}
	*flag = true;
	return true;
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

	std::cout << "Ready......" << std::endl;

	bool isServerShutdown = false;
	std::list<USER> userlist;

	while(1){
		USER user;

		int iAddLen = sizeof(user.saClient);
		user.socketClient = accept(socketListen, (SOCKADDR*)&(user.saClient), &iAddLen);
		if (user.socketClient == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				std::cout << inet_ntoa(user.saClient.sin_addr) << ":" << ntohs(user.saClient.sin_port)
					<< " Disconnected." << std::endl;
				closesocket(user.socketClient);
				break;
			}
		}
		else {
			std::cout << inet_ntoa(user.saClient.sin_addr) << ":" << ntohs(user.saClient.sin_port)
				<< " Connected."<<std::endl;

			userlist.push_back(user);
		}

		if (userlist.size() > 0) {
			for (auto iter = userlist.begin(); iter != userlist.end(); iter++) {
				std::string strUser;
				strUser.clear();
				strUser.append(inet_ntoa(iter->saClient.sin_addr));
				strUser.append(":");
				strUser.append(std::to_string(ntohs(iter->saClient.sin_port)));

				int iRecvSize = recv(iter->socketClient, iter->buf, sizeof(char) * BUFFERSIZE, 0);
				if (iRecvSize == 0) {
					std::cout << strUser << " Connection closed." << std::endl;
					closesocket(iter->socketClient);
					userlist.erase(iter);
					break;
				}
				if (iRecvSize == SOCKET_ERROR) {
					if (WSAGetLastError() != WSAEWOULDBLOCK) {
						std::cout << strUser << " Disconnected." << std::endl;
						char msg[] = "recv()";
						err_display();
						closesocket(iter->socketClient);
						userlist.erase(iter);
						break;
					}
				}
				if (iRecvSize > 0) {
					std::string strEcho;
					strEcho.clear();
					iter->buf[iRecvSize] = '\0';
					strEcho.append(strUser);
					strEcho.append(" : ");
					strEcho.append(iter->buf);
					std::cout << strEcho << std::endl;
					for (auto senditer = userlist.begin(); senditer != userlist.end(); senditer++) {
						int iSendSize = send(senditer->socketClient, strEcho.c_str(), (int)strEcho.length(), 0);
						if (iSendSize == SOCKET_ERROR)
						{
							if (WSAGetLastError() != WSAEWOULDBLOCK)
							{
								std::cout << strUser << " Disconnected." << std::endl;
								closesocket(senditer->socketClient);
								userlist.erase(senditer);
								break;
							}
						}
					}
					if (userlist.size() == 0) {
						break;
					}
				}
			}
		}
	}

	std::cout << "Shutting down..." << std::endl;
	while (userlist.size()) {
		closesocket(userlist.back().socketClient);
		userlist.pop_back();
	}

	closesocket(socketListen);
	WSACleanup();
	system("pause");

	return 0;
}