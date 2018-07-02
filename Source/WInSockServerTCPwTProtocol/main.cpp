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
	int iRecvByte = 0;
	bool m_bAccountAck;
	std::list<UPACKET> m_Packets;
	std::string m_strUsername;
};

std::list<USER> userlist;
bool isShutdown = false;

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

void DisconnectClient(std::list<USER>::iterator iterator) {
	iterator->m_bAccountAck = false;
	shutdown(iterator->socketClient, SD_BOTH);
	closesocket(iterator->socketClient);
	userlist.erase(iterator);
}

int AcceptUser(SOCKET socketListen) {
	while (!isShutdown) {
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
				return -1;
			}
		}
		else {
			std::cout << inet_ntoa(user.saClient.sin_addr) << ":" << ntohs(user.saClient.sin_port)
				<< " Connected." << std::endl;
			user.m_bAccountAck = true;

			UPACKET sendmsg = { 0 };
			std::string strWelcome = "서버 : 이름을 입력 : ";
			strcpy_s(sendmsg.msg, strWelcome.c_str());
			sendmsg.ph.len = (WORD)strlen(sendmsg.msg) + PACKET_HEADER_SIZE;
			sendmsg.ph.type = PACKET_CHAT_NAME_REQ;

			int iSendByte = send(user.socketClient, (char*)&sendmsg, sendmsg.ph.len, 0);
			if (iSendByte == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					std::cout << "Disconnected from server.(Send failed)" << std::endl;
					break;
				}
			}
			userlist.push_back(user);
		}
	}
	return 0;
}

int Receive(std::list<USER>::iterator iter) {
	while ((iter->iRecvByte < PACKET_HEADER_SIZE) && (iter->m_bAccountAck)) {
		iter->iRecvByte += recv(iter->socketClient, &(iter->buf[iter->iRecvByte]), PACKET_HEADER_SIZE - iter->iRecvByte, 0);
		if (iter->iRecvByte == 0 || iter->iRecvByte == SOCKET_ERROR) {
			return iter->iRecvByte;
		}
	}

	//패킷 헤더 다 받았다면
	if ((iter->iRecvByte == PACKET_HEADER_SIZE) && (!isShutdown)) {
		UPACKET rcvmsg = { 0 };
		memcpy(&rcvmsg, iter->buf, PACKET_HEADER_SIZE);
		while (iter->iRecvByte < rcvmsg.ph.len) {
			iter->iRecvByte += recv(iter->socketClient, iter->buf, rcvmsg.ph.len - iter->iRecvByte, 0);
			if (iter->iRecvByte == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					std::cout << "Disconnected from client.(Receiving body failed)" << std::endl;
					err_display();
					return iter->iRecvByte;
				}
			}
		}
		iter->m_Packets.push_back(rcvmsg);
	}

	return iter->iRecvByte;
}

int Broadcast(std::list<USER>::iterator iter) {
	std::string strUsername;
	strUsername.append(iter->m_strUsername);
	strUsername.append(" : ");
	strUsername.append(iter->m_Packets.front().msg);

	UPACKET packet = { 0 };
	packet.ph.type = PACKET_CHAT_MSG;
	packet.ph.len = (WORD)strUsername.size() + PACKET_HEADER_SIZE;
	strcpy_s(packet.msg, strUsername.c_str());

	int iSendByte = 0;
	for (auto sendIter = userlist.begin(); sendIter != userlist.end(); ++sendIter) {
		iSendByte = send(sendIter->socketClient, (char*)&packet, packet.ph.len, 0);
		if (iSendByte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				shutdown(iter->socketClient, SD_BOTH);
				closesocket(sendIter->socketClient);
				userlist.erase(sendIter);
				break;
			}
		}
	}

	return 0;
}

int ReceiveAndBroadcast() {
	while (!isShutdown) {
		if (userlist.size() > 0) {
			for (auto iter = userlist.begin(); iter != userlist.end(); ++iter) {
				std::string strUser;
				strUser.append(inet_ntoa(iter->saClient.sin_addr));
				strUser.append(":");
				strUser.append(std::to_string(ntohs(iter->saClient.sin_port)));

				int iRecvByte = Receive(iter);
				if (iRecvByte == 0) {
					std::cout << "Connection closed." << std::endl;
					DisconnectClient(iter);
					break;
				}
				if (iRecvByte == SOCKET_ERROR) {
					if (WSAGetLastError() != WSAEWOULDBLOCK) {
						std::cout << "Disconnected from client.(Receiving header failed)" << std::endl;
						err_display();
						DisconnectClient(iter);
						break;
					}
				}

				//패킷 전부 받았을 때 작동
				if (iter->m_Packets.size() && iter->m_Packets.front().ph.len && iter->m_bAccountAck) {
					memcpy(&iter->m_Packets.front().msg, iter->buf, iter->m_Packets.front().ph.len - PACKET_HEADER_SIZE);
					switch (iter->m_Packets.front().ph.type) {
					case PACKET_CHAT_MSG: {
						std::cout << iter->m_strUsername << " : " << iter->m_Packets.front().msg << std::endl;
						Broadcast(iter);
						iter->m_Packets.pop_front();
					}break;
					case PACKET_CHAT_NAME_ACK: {
						iter->m_strUsername = iter->m_Packets.front().msg;
						std::cout << strUser << " named " << iter->m_strUsername << std::endl;
						iter->m_Packets.pop_front();
					}break;
					}
				}
				iter->iRecvByte = 0;
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

	std::thread threadGetShutdown(GetServerShutdown, &isShutdown);
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