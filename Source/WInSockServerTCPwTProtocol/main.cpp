#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <WinSock2.h>
#include "TProtocol.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 10000
#define BUFFERSIZE 2048

struct USER {
	SOCKET socketClient;
	SOCKADDR_IN saClient;
	char buf[BUFFERSIZE + 0] = { 0 };
	int iRecvSize;
	bool m_bAccountAck;
	std::list<UPACKET> m_Packets;
	//std::string strBuffer;
};

std::list<USER> userlist;
bool isServerShutdown = false;

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
		std::string buf;
		std::getline(std::cin, buf);

		if (buf.length() == 0) {
			break;
		}
	}
	*flag = true;
	return true;
}

int AcceptUser(SOCKET socketListen) {
	while (!isServerShutdown) {
		USER user;
		int iAddLen = sizeof(user.saClient);
		user.socketClient = accept(socketListen, (SOCKADDR*)&(user.saClient), &iAddLen);
		if (user.socketClient == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				std::cout << inet_ntoa(user.saClient.sin_addr) << ":" << ntohs(user.saClient.sin_port)
					<< " Disconnected." << std::endl;
				err_display();
				shutdown(user.socketClient, SD_BOTH);
				closesocket(user.socketClient);
				break;
				return -1;
			}
		}
		else {
			std::cout << inet_ntoa(user.saClient.sin_addr) << ":" << ntohs(user.saClient.sin_port)
				<< " Connected." << std::endl;

			userlist.push_back(user);
		}
	}
	return 0;
}

int ReceiveAndBroadcast() {
	while (!isServerShutdown) {
		if (userlist.size() > 0) {
			for (auto iter = userlist.begin(); iter != userlist.end(); ++iter) {
				std::string strUser;
				strUser.append(inet_ntoa(iter->saClient.sin_addr));
				strUser.append(":");
				strUser.append(std::to_string(ntohs(iter->saClient.sin_port)));

				if (!isServerShutdown) {
					iter->iRecvSize = recv(iter->socketClient, iter->buf, sizeof(char) * BUFFERSIZE, 0);
				}
				if (iter->iRecvSize == 0) {
					std::cout << strUser << " Connection closed." << std::endl;
					shutdown(iter->socketClient, SD_BOTH);
					closesocket(iter->socketClient);
					userlist.erase(iter);
					break;
				}
				if (iter->iRecvSize == SOCKET_ERROR) {
					if (WSAGetLastError() != WSAEWOULDBLOCK) {
						std::cout << strUser << " Disconnected." << std::endl;
						err_display();
						shutdown(iter->socketClient, SD_BOTH);
						closesocket(iter->socketClient);
						userlist.erase(iter);
						break;
					}
				}
				if (iter->iRecvSize > 0) {
					std::cout << iter->buf << std::endl;

					for (auto sendIter = userlist.begin(); sendIter != userlist.end(); ++sendIter) {
						int iSendSize = send(sendIter->socketClient, iter->buf, iter->iRecvSize, 0);
						if (iSendSize == SOCKET_ERROR)
						{
							if (WSAGetLastError() != WSAEWOULDBLOCK) {
								shutdown(iter->socketClient, SD_BOTH);
								closesocket(sendIter->socketClient);
								userlist.erase(sendIter);
								break;
							}
						}
					}
				}
			}
		}
	}

	return 0;
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

	SetNonBlockingSocket(socketListen, true);

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

	std::thread threadGetShutdown(GetServerShutdown, &isServerShutdown);
	threadGetShutdown.detach();

	std::thread threadAccept(AcceptUser, socketListen);
	std::thread threadReceiveAndBroadcast(ReceiveAndBroadcast);
	threadAccept.join();
	threadReceiveAndBroadcast.join();

	std::cout << "Shutting down..." << std::endl;
	while (userlist.size()) {
		shutdown(userlist.back().socketClient, SD_BOTH);
		closesocket(userlist.back().socketClient);
		userlist.pop_back();
	}

	shutdown(socketListen, SD_BOTH);
	closesocket(socketListen);
	WSACleanup();
	system("pause");

	return 0;
}