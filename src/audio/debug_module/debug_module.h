/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright(c) 2024 Intel Corporation. All rights reserved.
 *
 * Author: Fabiola Jasinska <fabiola.jasinska@intel.com>
 */

#ifndef __SOF_AUDIO_DEBUG_MODULE_H__
#define __SOF_AUDIO_DEBUG_MODULE_H__
#include <sof/audio/ipc-config.h>
#include <ipc/stream.h>
#include <stddef.h>
#include <stdint.h>
#include <ipc/stream.h>
#include <ipc4/base-config.h>
#include "debug_module_ipc4.h"

/** forward declaration */
struct debug_module_data;

enum debug_module_sha_algorithm {
	SHA1_ALGORITHM   = 0,
	SHA224_ALGORITHM = 1,
	SHA256_ALGORITHM = 2,
	SHA384_ALGORITHM = 3,
	SHA512_ALGORITHM = 4,
};

#define SHA384_API_MAX_FRAG_LEN (64 * 1024 - 256)

uint8_t processing_buffer[SHA384_API_MAX_FRAG_LEN];

struct debug_module_sha_payload {
	enum debug_module_sha_algorithm algorithm;
	size_t data_size;
	uint8_t data[0];
};

struct debug_module_watchdog_enable_payload {
	uint8_t instance_id;
	bool enable;
};

static inline uint8_t get_channel_location(const channel_map map,
					   const enum ipc4_channel_index channel)
{
	uint8_t offset = 0xF;
	uint8_t i;

	/* Search through all 4 bits of each byte in the integer for the channel. */
	for (i = 0; i < 8; i++) {
		if (((map >> (i * 4)) & 0xF) == (uint8_t)channel) {
			offset = i;
			break;
		}
	}

	return offset;
}

static inline enum ipc4_channel_index get_channel_index(const channel_map map,
							const uint8_t location)
{
	return (enum ipc4_channel_index)((map >> (location * 4)) & 0xF);
}

/**
 * \brief debug_module component private data.
 */
struct debug_module_data {
	struct ipc4_debug_module_cfg config;
};


#endif /* __SOF_AUDIO_DEBUG_MODULE_H__ */
