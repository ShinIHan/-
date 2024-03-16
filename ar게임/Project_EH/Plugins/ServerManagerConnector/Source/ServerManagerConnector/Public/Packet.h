#pragma once
#define INET_ADDRSTRLEN 22
#define PORT		9999

constexpr int MAX_ID_LEN = 100;

constexpr int MAX_CLASSROOM_LEN = 17;
constexpr int MAX_LECTUREID_LEN = 11;
constexpr int MAX_CONTENT_LEN = 30;

constexpr int MAX_PLAYERSUB_LEN = 50;

#define S2SM_CONNECT			1
#define S2SM_CONNECTPLAYER		4
#define S2SM_DISCONNECTPLAYER	5
#define S2SM_EVENTREQUEST		6

#define SM2S_CONNECT			11
#define SM2S_DISCONNECT			12
#define SM2S_LECTUREEVENT		13
#define SM2S_INVITEEVENT		14
#define SM2S_EVENTRESPONSE		16
#define SM2S_DISCONNECTPLAYERREQUEST	17

#define C2SM_CONNECT			21
#define C2SM_MATCHREQUEST		23

#define SM2C_CONNECT			31
#define	SM2C_DISCONNECT			32
#define SM2C_MATCHRESPONSE		33

#pragma pack(push, 1)

struct s2sm_connect_packet {
	unsigned char size;
	char packet_type;
	int connected_players;
	int max_acceptable_players;
};

struct s2sm_connect_player_packet {
	unsigned char size;
	char packet_type;
	int connected_players;
};

struct s2sm_disconnect_player_packet {
	unsigned char size;
	char packet_type;
	int connected_players;
};

struct s2sm_eventrequest_packet {
	unsigned char size;
	char packet_type;
	int event_type;
};

////////////////////////////////////

struct sm2s_connect_packet {
	unsigned char size;
	char packet_type;
	wchar_t vivox_channel_name[MAX_ID_LEN];
};

struct sm2s_disconnect_packet {
	unsigned char size;
	char packet_type;
};

struct sm2s_invite_event_packet {
	unsigned char size;
	char packet_type;
	unsigned short start_year;
	unsigned char start_month;
	unsigned char start_day;
	unsigned char start_hour;
	unsigned char start_minute;
	wchar_t classroom[MAX_CLASSROOM_LEN];
	wchar_t lectureId[MAX_LECTUREID_LEN];
	wchar_t content[MAX_CONTENT_LEN];
};

struct sm2s_lecture_event_packet {
	unsigned char size;
	char packet_type;
	unsigned short start_year;
	unsigned char start_month;
	unsigned char start_day;
	unsigned char start_hour;
	unsigned char start_minute;
	unsigned short end_year;
	unsigned char end_month;
	unsigned char end_day;
	unsigned char end_hour;
	unsigned char end_minute;
	wchar_t classroom[MAX_CLASSROOM_LEN];
	wchar_t lectureId[MAX_LECTUREID_LEN];
	wchar_t content[MAX_CONTENT_LEN];
};

struct sm2s_eventresponse_packet {
	unsigned char size;
	char packet_type;
	int event_type;
};

struct sm2s_disconnectplayerrequest_packet {
	unsigned char size;
	char packet_type;
	wchar_t player_sub[MAX_PLAYERSUB_LEN];
};

////////////////////////////////////

struct c2sm_connect_packet {
	unsigned char size;
	char packet_type;
	wchar_t player_sub[MAX_PLAYERSUB_LEN];
};

struct c2sm_matchrequest_packet {
	unsigned char size;
	char packet_type;
	wchar_t classroom[MAX_CLASSROOM_LEN];
};

////////////////////////////////////

struct sm2c_connect_packet {
	unsigned char size;
	char packet_type;
};

struct sm2c_disconnect_packet {
	unsigned char size;
	char packet_type;
};

struct sm2c_matchresponse_packet {
	unsigned char size;
	char packet_type;
	wchar_t ip[INET_ADDRSTRLEN];
};

#pragma pack(pop)
