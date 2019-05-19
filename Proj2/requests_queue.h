#pragma once

#include "constants.h"
#include "types.h"
#include "sope.h"

#include "bank_operations.h"

void put_request(tlv_request_t tlv_request);
void get_request(tlv_request_t *tlv_request, int bank_office_id);
void destroy_buffer_lock();
