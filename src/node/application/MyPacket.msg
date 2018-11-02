cplusplus {{
#include "ApplicationPacket_m.h"
#include <string>
#include <queue>
#include <stack>

typedef std::queue<const char*> TypeMsg;

extern "C" {
#include "communication/plugin/plugin.h"
#include "util/bytelib.h"
#include "asn1/phd_types.h"
}

}}

class ApplicationPacket;
class noncobject TypeMsg;

struct Tmsg {
	TypeMsg fila;
	string send_str;
	string recv_str;
	uint8_t buff_msg[65535];
	int tam_buff;
}

packet MyPacket extends ApplicationPacket {
	Tmsg extraData;
}

