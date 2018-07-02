#pragma once
#define MAX_MSGCOUNTER 2048

#pragma pack(push, 1)
typedef struct {
	unsigned short len;//패킷의 길이
	unsigned short type;//패킷의 종류
}PACKET_HEADER;

typedef struct {
	PACKET_HEADER ph;
	char msg[MAX_MSGCOUNTER];
}UPACKET;
#pragma pack(pop)

#define PACKET_HEADER_SIZE 4
#define PACKET_CHAT_MSG 1000
#define PACKET_CHAT_NAME_REQ 2000
#define PACKET_CHAT_NAME_ACK 3000