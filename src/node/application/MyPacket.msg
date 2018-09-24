cplusplus {{
#include "ApplicationPacket_m.h"

extern "C" {
#include "communication/plugin/plugin.h"
#include "util/bytelib.h"
#include "asn1/phd_types.h"
}

}}

class ApplicationPacket;


struct Tmsg {
	string send_str;
	string recv_str;
	uint8_t buff_msg[65535];
	int tam_buff;
}

packet MyPacket extends ApplicationPacket {
	Tmsg extraData;
}

