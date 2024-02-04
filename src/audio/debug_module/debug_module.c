// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2022 Intel Corporation. All rights reserved.
//
// Author: Bartosz Kokoszko <bartoszx.kokoszko@intel.com>
// Author: Adrian Bonislawski <adrian.bonislawski@intel.com>

//#include <sof/audio/module_adapter/module/generic.h>
#include <module/module/base.h>
#include <module/module/api_ver.h>
#include <rimage/sof/user/manifest.h>
#include <module/audio/source_api.h>
#include <module/audio/sink_api.h>

#include "debug_module.h"
#include <errno.h>
#include <sof/audio/module_adapter/library/native_system_service.h>
#include <stdlib.h>
#include <stdint.h>
#include <rtos/string.h>
#include <sof/lib/cpu.h>
#include <sof/lib/watchdog.h>

/* Logging is temporary disabled */
#define comp_err(...)
#define comp_dbg(...)

static struct native_system_service_api *system_service;
uint32_t heap_mem[2048] __attribute__((section(".heap_mem"))) __attribute__((aligned(4096)));

int32_t custom_coeffs[DEBUG_MODULE_COEFFS_LENGTH];

static int debug_module_free(struct processing_module *mod)
{
	struct debug_module_data *cd = module_get_private_data(mod);

	free(cd);

	return 0;
}

static int debug_module_init(struct processing_module *mod)
{
	struct module_config *dst = &mod->priv.cfg;
	const struct ipc4_debug_module_cfg *debug_module = dst->init_data;
	struct module_data *mod_data = &mod->priv;
	struct comp_dev *dev = mod->dev;
	struct debug_module_data *cd;
	int ret;

	cd = malloc(sizeof(struct debug_module_data));
	if (!cd) {
		free(dev);
		return -ENOMEM;
	}

	mod_data->private = cd;

	if (memcpy_s(&cd->config, sizeof(cd->config), debug_module, sizeof(*debug_module)) < 0) {
		ret = -EINVAL;
		goto err;
	}


	return 0;

err:
	debug_module_free(mod);
	return ret;
}

static int
debug_module_process(struct processing_module *mod,
		      struct sof_source **input_buffers, int num_input_buffers,
		      struct sof_sink **output_buffers, int num_output_buffers)
{
	struct debug_module_data *cd = module_get_private_data(mod);
	struct comp_dev *dev = mod->dev;

	size_t output_frames, input_frames, ret, input_cirbuf_size, output_cirbuf_size;
	const uint8_t *input0_pos, *input0_start;
	uint8_t *output_pos, *output_start;

	comp_dbg(dev, "debug_module_process()");

	output_frames = sink_get_free_frames(output_buffers[0]);
	input_frames = source_get_data_frames_available(input_buffers[0]);

	const size_t output_frame_bytes = sink_get_frame_bytes(output_buffers[0]);

	ret = sink_get_buffer(output_buffers[0], output_frames * output_frame_bytes,
			      (void **)&output_pos, (void **)&output_start, &output_cirbuf_size);
	if (ret)
		return -ENODATA;

	const size_t input0_frame_bytes = source_get_frame_bytes(input_buffers[0]);

	ret = source_get_data(input_buffers[0], input_frames * input0_frame_bytes,
			      (const void **)&input0_pos, (const void **)&input0_start,
			      &input_cirbuf_size);
	if (ret) {
		sink_commit_buffer(output_buffers[0], 0);
		return -ENODATA;
	}

	//cd->mix_routine(cd, (const void *)input0_start, input_cirbuf_size, (void *)output_start);

	ret = sink_commit_buffer(output_buffers[0], output_frames * output_frame_bytes);
	if (ret)
		return ret;

	ret = source_release_data(input_buffers[0], input_frames * input0_frame_bytes);
	if (ret)
		return ret;
	return 0;
}

static int debug_module_fw_exception_test(void)
{
	int a = 0;
	int exception = 1 / a;

	return 0;
}

static int debug_module_watchdog_enable_set(struct processing_module *mod,
					    const uint8_t *data, int data_size)
{
	struct debug_module_watchdog_enable_payload payload;
	const bool enable = payload.enable;

	payload = *(const struct debug_module_watchdog_enable_payload *)data;
	if (enable)
		watchdog_enable(payload.instance_id);
	else
		watchdog_disable(payload.instance_id);
	return 0;
}

static int debug_module_sha384_test(uint8_t *input, size_t input_size,
				    uint8_t *output, size_t output_size)
{
	struct debug_module_sha_payload payload;
	int ret;

	payload = *(const struct debug_module_sha_payload *)input;
	if (memcpy_s(&processing_buffer, sizeof(processing_buffer),
		     payload.data, payload.data_size) < 0) {
		ret = -EINVAL;
		return ret;
	}
	hash_context sha384hash;
	int ec = hash_sha384_init(sha384hash);
	size_t remaining_data_size = payload.data_size;

	do {
		size_t chunk_size = remaining_data_size > sizeof(processing_buffer) ?
		sizeof(processing_buffer) : remaining_data_size;

		remaining_data_size -= chunk_size;
		ec = hash_update(sha384hash, processing_buffer, sizeof(processing_buffer));
	} while (remaining_data_size);

	size_t hash_get_size = hash_get_digest(sha384hash, output, output_size);

	if (memcpy_s(&output, output_size, sha384hash, hash_get_size) < 0) {
		ret = -EINVAL;
		return ret;
	}
	return 0;
}

static int debug_module_hang_fw(void)
{
	struct comp_driver_list *drivers = comp_drivers_get();
	k_spinlock_key_t key;
	int gate = 0;

	key = k_spin_lock(&drivers->lock);

	while (gate == 0) {
		for (size_t idx = 0; idx < 12; ++idx)
			asm volatile("nop");
	}
	k_spin_unlock(&drivers->lock, key);
	return 0;
}

static int debug_module_set_configuration(struct processing_module *mod,
					  uint32_t config_id,
					  enum module_cfg_fragment_position pos,
					  uint32_t data_offset_size,
					  const uint8_t *fragment, size_t fragment_size,
					  uint8_t *response,
					  size_t response_size)
{
	struct comp_dev *dev = mod->dev;

	comp_dbg(dev, "debug_module_set_config()");

	switch (config_id) {
	case IPC4_DEBUG_MODULE_FW_EXCEPTION_TEST:
		return debug_module_fw_exception_test();
	case IPC4_DEBUG_MODULE_WDT_ENABLE_SET:
		return debug_module_watchdog_enable_set(mod, fragment, fragment_size);
	case IPC4_DEBUG_MODULE_HANG_FW_TO_TEST_WATCHDOG:
		return debug_module_hang_fw();
	case IPC4_DEBUG_MODULE_SHA384_TEST:
		return debug_module_sha384_test(fragment, fragment_size, response, response_size);
	default:
		return -EINVAL;
	}
}

static const struct module_interface debug_module_interface = {
	.init = debug_module_init,
	.process = debug_module_process,
	.free = debug_module_free,
	.set_configuration = debug_module_set_configuration,
};

DECLARE_LOADABLE_MODULE_API_VERSION(udm);

static void *entry_point(void *mod_cfg, void *parent_ppl, void **mod_ptr)
{
	system_service = *(const struct native_system_agent **)mod_ptr;

	return &debug_module_interface;
}

/* 5fa8976c-581b-4ea6-8fd3-e94dd39a3669 */
__attribute__((section(".module")))
const struct sof_man_module_manifest udm_manifest = {
	.module = {
		.name = "DEBUG",
		.uuid = {0x6c, 0x97, 0xa8, 0x5f,
			 0x1b, 0x58,
			 0xa6, 0x4e,
			 0x8f, 0xd3,
			 0xe9, 0x4d, 0xd3, 0x9a, 0x36, 0x69},
		.entry_point = (uint32_t)entry_point,
		.type = {.load_type = SOF_MAN_MOD_TYPE_MODULE,
		.domain_ll = 1 },
		.affinity_mask = 1,
	}
};
