#pragma once
#include <Windows.h>
#define MAX_MSGCOUNTER 2048

typedef struct {
	WORD len;//패킷의 길이
	WORD type;//패킷의 종류
}PACKET_HEADER;

typedef struct {
	PACKET_HEADER ph;
	char msg[MAX_MSGCOUNTER];
}UPACKET;
#define PACKET_CHAT_MSG 1000
#define PACKET_CHAT_NAME 1000