#pragma once
#include <Windows.h>
#define MAX_MSGCOUNTER 2048

typedef struct {
	WORD len;//��Ŷ�� ����
	WORD type;//��Ŷ�� ����
}PACKET_HEADER;

typedef struct {
	PACKET_HEADER ph;
	char msg[MAX_MSGCOUNTER];
}UPACKET;
#define PACKET_CHAT_MSG 1000
#define PACKET_CHAT_NAME 1000