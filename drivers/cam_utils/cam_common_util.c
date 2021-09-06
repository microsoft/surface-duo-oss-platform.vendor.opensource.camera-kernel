// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019,2021 The Linux Foundation. All rights reserved.
 */

#include <linux/string.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "cam_common_util.h"
#include "cam_debug_util.h"

int cam_common_util_get_string_index(const char **strings,
	uint32_t num_strings, const char *matching_string, uint32_t *index)
{
	int i;

	for (i = 0; i < num_strings; i++) {
		if (strnstr(strings[i], matching_string, strlen(strings[i]))) {
			CAM_DBG(CAM_UTIL, "matched %s : %d\n",
				matching_string, i);
			*index = i;
			return 0;
		}
	}

	return -EINVAL;
}

uint32_t cam_common_util_remove_duplicate_arr(int32_t *arr, uint32_t num)
{
	int i, j;
	uint32_t wr_idx = 1;

	if (!arr) {
		CAM_ERR(CAM_UTIL, "Null input array");
		return 0;
	}

	for (i = 1; i < num; i++) {
		for (j = 0; j < wr_idx ; j++) {
			if (arr[i] == arr[j])
				break;
		}
		if (j == wr_idx)
			arr[wr_idx++] = arr[i];
	}

	return wr_idx;
}

uint64_t cam_common_util_get_time_diff(struct timeval *t1, struct timeval *t2)
{
	uint64_t diff = 0;

	diff = (t1->tv_sec - t2->tv_sec) * 1000000 +
		    (t1->tv_usec - t2->tv_usec);
	return diff;
}

void cam_common_util_get_curr_timestamp(struct timeval *time_stamp)
{
	struct timespec ts;

	get_monotonic_boottime(&ts);
	time_stamp->tv_sec    = ts.tv_sec;
	time_stamp->tv_usec   = ts.tv_nsec/1000;
}

