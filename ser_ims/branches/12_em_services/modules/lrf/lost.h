#ifndef ECSCF_LOST_H
#define ECSCF_LOST_H

#include "../../str.h"

typedef enum {HTTP_TYPE =1, HTTPS_TYPE} http_type;

typedef struct lost_server_i{
		str host;
		unsigned int port;
		http_type type;
		}lost_server_info;
int LRF_get_psap(struct sip_msg* msg, char* str1, char* str2);
int LRF_has_loc(struct sip_msg* msg, char* str1, char* str2);
int LRF_save_user_loc(struct sip_msg* msg, char* str1, char* str2);


#endif
