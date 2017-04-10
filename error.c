#include "error.h"

bool or_check_error(int64_t code) {
	switch(code) {
	 case ORSOQS_ERR_UCR:
	 case ORSOQS_ERR_BSO:
		return true;
	
	 default:
		return false;
	}
}

char const* or_error_str(const int64_t code) {
	switch(code) {
	 case ORSOQS_ERR_UCR:
		return "Unreachable code reached.";
	 case ORSOQS_ERR_BSO:
		return "Operation not supported on socket.";
	 default:
		printf("Unhandled case in or_error_str()\n");
		abort();
	}
}
