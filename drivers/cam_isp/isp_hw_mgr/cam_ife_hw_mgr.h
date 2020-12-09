/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _CAM_IFE_HW_MGR_H_
#define _CAM_IFE_HW_MGR_H_

#include <linux/completion.h>
#include <linux/time.h>
#include "cam_isp_hw_mgr.h"
#include "cam_vfe_hw_intf.h"
#include "cam_ife_csid_hw_intf.h"
#include "cam_tasklet_util.h"

/* enum cam_ife_hw_mgr_res_type - manager resource node type */
enum cam_ife_hw_mgr_res_type {
	CAM_IFE_HW_MGR_RES_UNINIT,
	CAM_IFE_HW_MGR_RES_ROOT,
	CAM_IFE_HW_MGR_RES_CID,
	CAM_IFE_HW_MGR_RES_CSID,
	CAM_IFE_HW_MGR_RES_IFE_SRC,
	CAM_IFE_HW_MGR_RES_IFE_IN_RD,
	CAM_IFE_HW_MGR_RES_IFE_OUT,
};

/* IFE resource constants */
#define CAM_IFE_HW_IN_RES_MAX            (CAM_ISP_IFE_IN_RES_MAX & 0xFF)
#define CAM_IFE_HW_OUT_RES_MAX           (CAM_ISP_IFE_OUT_RES_MAX & 0xFF)
#define CAM_IFE_HW_RES_POOL_MAX          64

#define CAM_IFE_HW_STATE_STOPPED  0
#define CAM_IFE_HW_STATE_STARTING 1
#define CAM_IFE_HW_STATE_STARTED  2
#define CAM_IFE_HW_STATE_BUSY     3
#define CAM_IFE_HW_STATE_STOPPING 4

/**
 * struct cam_vfe_hw_mgr_res- HW resources for the VFE manager
 *
 * @list:                used by the resource list
 * @res_type:            IFE manager resource type
 * @res_id:              resource id based on the resource type for root or
 *                       leaf resource, it matches the KMD interface port id.
 *                       For branch resrouce, it is defined by the ISP HW
 *                       layer
 * @hw_res:              hw layer resource array. For single VFE, only one VFE
 *                       hw resrouce will be acquired. For dual VFE, two hw
 *                       resources from different VFE HW device will be
 *                       acquired
 * @parent:              point to the parent resource node.
 * @children:            point to the children resource nodes
 * @child_num:           numbe of the child resource node.
 * @is_secure            informs whether the resource is in secure mode or not
 *
 */
struct cam_ife_hw_mgr_res {
	struct list_head                 list;
	enum cam_ife_hw_mgr_res_type     res_type;
	uint32_t                         res_id;
	uint32_t                         is_dual_vfe;
	struct cam_isp_resource_node    *hw_res[CAM_ISP_HW_SPLIT_MAX];

	/* graph */
	struct cam_ife_hw_mgr_res       *parent;
	struct cam_ife_hw_mgr_res       *child[CAM_IFE_HW_OUT_RES_MAX];
	uint32_t                         num_children;
	uint32_t                         is_secure;
};


/**
 * struct ctx_base_info - Base hardware information for the context
 *
 * @idx:                 Base resource index
 * @split_id:            Split info for the base resource
 *
 */
struct ctx_base_info {
	uint32_t                       idx;
	enum cam_isp_hw_split_id       split_id;
};

/**
 * struct cam_ife_hw_mgr_debug - contain the debug information
 *
 * @dentry:                    Debugfs entry
 * @csid_debug:                csid debug information
 * @enable_recovery:           enable recovery
 * @enable_csid_recovery:      enable csid recovery
 * @enable_diag_sensor_status: enable sensor diagnosis status
 * @enable_req_dump:           Enable request dump on HW errors
 * @per_req_reg_dump:          Enable per request reg dump
 *
 */
struct cam_ife_hw_mgr_debug {
	struct dentry  *dentry;
	uint64_t       csid_debug;
	uint32_t       enable_recovery;
	uint32_t       enable_csid_recovery;
	uint32_t       camif_debug;
	bool           enable_req_dump;
	bool           per_req_reg_dump;
};

/**
 * struct cam_ife_mgr_bw_data - contain data to calc bandwidth for context
 *
 * @format:                    image format
 * @width:                     image width
 * @height:                    image height
 * @framerate:                 framerate
 *
 */
struct cam_ife_mgr_bw_data {
	uint32_t format;
	uint32_t width;
	uint32_t height;
	uint32_t framerate;
};


/**
 * struct cam_vfe_hw_mgr_ctx - IFE HW manager Context object
 *
 * concr_ctx:             HW Context currently used from this manager context
 * hw_mgr:                IFE hw mgr which owns this context
 * event_cb:              event callbacks
 * cb_priv:               event callbacks data
 * ctx_in_use:            indicates if context is active
 * is_offline:            indicates if context is used for offline processing
 * ctx_idx:               index of this context
 * num_in_ports:          number of context input ports
 * in_ports:              context input ports
 * bw_data:               contains data for BW usage calculation
 *
 */
struct cam_ife_hw_mgr_ctx {
	struct cam_ife_hw_concrete_ctx       *concr_ctx;
	struct cam_ife_hw_mgr                *hw_mgr;
	cam_hw_event_cb_func                  event_cb[CAM_ISP_HW_EVENT_MAX];
	void                                 *cb_priv;
	uint32_t                              ctx_in_use;
	bool                                  is_offline;
	uint32_t                              ctx_idx;
	uint32_t                              num_in_ports;
	struct cam_isp_in_port_generic_info **in_ports;
	struct cam_ife_mgr_bw_data            bw_data;
};

/**
 * struct cam_ife_hw_concrete_ctx - IFE HW Context object
 *
 * @list:                   used by the ctx list.
 * @common:                 common acquired context data
 * @ctx_index:              acquired context id.
 * @master_hw_idx:          hw index for master core
 * @slave_hw_idx:           hw index for slave core
 * @acquired_hw_id:         hw id that this context acquired
 * @served_ctx_id:          contains currently and next served context ids
 * @served_ctx_r:           index pointing to currently served context id
 * @served_ctx_w:           index pointing to next served context id
 * @hw_mgr:                 IFE hw mgr which owns this context
 * @ctx_in_use:             flag to tell whether context is active
 * @res_list_ife_in:        Starting resource(TPG,PHY0, PHY1...) Can only be
 *                          one.
 * @res_list_ife_cid:       CID resource list
 * @res_list_ife_csid:      CSID resource list
 * @res_list_ife_src:       IFE input resource list
 * @res_list_ife_in_rd      IFE input resource list for read path
 * @res_list_ife_out:       IFE output resoruces array
 * @free_res_list:          Free resources list for the branch node
 * @res_pool:               memory storage for the free resource list
 * @irq_status0_mask:       irq_status0_mask for the context
 * @irq_status1_mask:       irq_status1_mask for the context
 * @base                    device base index array contain the all IFE HW
 *                          instance associated with this context.
 * @num_base                number of valid base data in the base array
 * @cdm_handle              cdm hw acquire handle
 * @cdm_ops                 cdm util operation pointer for building
 *                          cdm commands
 * @cdm_cmd                 cdm base and length request pointer
 * @sof_cnt                 sof count value per core, used for dual VFE
 * @epoch_cnt               epoch count value per core, used for dual VFE
 * @eof_cnt                 eof count value per core, used for dual VFE
 * @overflow_pending        flat to specify the overflow is pending for the
 *                          context
 * @cdm_done                flag to indicate cdm has finished writing shadow
 *                          registers
 * @is_rdi_only_context     flag to specify the context has only rdi resource
 * @config_done_complete    indicator for configuration complete
 * @reg_dump_buf_desc:      cmd buffer descriptors for reg dump
 * @num_reg_dump_buf:       Count of descriptors in reg_dump_buf_desc
 * @applied_req_id:         Last request id to be applied
 * @last_dump_flush_req_id  Last req id for which reg dump on flush was called
 * @last_dump_err_req_id    Last req id for which reg dump on error was called
 * @init_done               indicate whether init hw is done
 * @is_fe_enabled           Indicate whether fetch engine\read path is enabled
 * @is_dual                 indicate whether context is in dual VFE mode
 * @custom_enabled          update the flag if context is connected to custom HW
 * @use_frame_header_ts     obtain qtimer ts using frame header
 * @ts                      captured timestamp when the ctx is acquired
 * @is_offline              Indicate whether context is for offline IFE
 * @dsp_enabled             Indicate whether dsp is enabled in this context
 * @dual_ife_irq_mismatch_cnt   irq mismatch count value per core, used for
 *                              dual VFE
 * @ctx_state               Indicates context state
 * @offline_clk             Clock value to be configured for offline processing
 */
struct cam_ife_hw_concrete_ctx {
	struct list_head                list;

	void                           *tasklet_info;

	uint32_t                        ctx_index;
	uint32_t                        master_hw_idx;
	uint32_t                        slave_hw_idx;
	uint32_t                        acquired_hw_id;
	uint32_t                        served_ctx_id[2];
	uint32_t                        served_ctx_r;
	uint32_t                        served_ctx_w;
	struct cam_ife_hw_mgr          *hw_mgr;
	uint32_t                        ctx_in_use;

	struct cam_ife_hw_mgr_res       res_list_ife_in;
	struct list_head                res_list_ife_cid;
	struct list_head                res_list_ife_csid;
	struct list_head                res_list_ife_src;
	struct list_head                res_list_ife_in_rd;
	struct cam_ife_hw_mgr_res       res_list_ife_out[
						CAM_IFE_HW_OUT_RES_MAX];

	struct list_head                free_res_list;
	struct cam_ife_hw_mgr_res       res_pool[CAM_IFE_HW_RES_POOL_MAX];

	uint32_t                        irq_status0_mask[CAM_IFE_HW_NUM_MAX];
	uint32_t                        irq_status1_mask[CAM_IFE_HW_NUM_MAX];
	struct ctx_base_info            base[CAM_IFE_HW_NUM_MAX];
	uint32_t                        num_base;
	uint32_t                        cdm_handle;
	struct cam_cdm_utils_ops       *cdm_ops;
	struct cam_cdm_bl_request      *cdm_cmd;

	uint32_t                        sof_cnt[CAM_IFE_HW_NUM_MAX];
	uint32_t                        epoch_cnt[CAM_IFE_HW_NUM_MAX];
	uint32_t                        eof_cnt[CAM_IFE_HW_NUM_MAX];
	atomic_t                        overflow_pending;
	atomic_t                        cdm_done;
	uint32_t                        is_rdi_only_context;
	struct completion               config_done_complete;
	struct cam_cmd_buf_desc         reg_dump_buf_desc[
						CAM_REG_DUMP_MAX_BUF_ENTRIES];
	uint32_t                        num_reg_dump_buf;
	uint64_t                        applied_req_id;
	uint64_t                        last_dump_flush_req_id;
	uint64_t                        last_dump_err_req_id;
	bool                            init_done;
	bool                            is_fe_enabled;
	bool                            is_dual;
	bool                            custom_enabled;
	bool                            use_frame_header_ts;
	struct timespec64               ts;
	bool                            is_offline;
	bool                            dsp_enabled;
	uint32_t                        dual_ife_irq_mismatch_cnt;
	atomic_t                        ctx_state;
	uint32_t                        offline_clk;
};

/**
 * struct cam_ife_offline_hw - Offline ife context allocation information
 *
 * @ctx_idx:                context index
 * @custom_enabled:         update the flag if context is connected to custom HW
 * @use_frame_header_ts     obtain qtimer ts using frame header
 * @acquired_hw_id:         Acquired hardware mask
 * @acquired_hw_path:       Acquired path mask for an input
 *                          if input splits into multiple paths,
 *                          its updated per hardware
 * valid_acquired_hw:       Valid num of acquired hardware
 * @ife_ctx:                HW context connected to that HW
 *
 */
struct cam_ife_offline_hw {
	uint32_t                        ctx_idx;
	bool                            custom_enabled;
	bool                            use_frame_header_ts;
	uint32_t                        acquired_hw_id[CAM_MAX_ACQ_RES];
	uint32_t                        acquired_hw_path[CAM_MAX_ACQ_RES][
						CAM_MAX_HW_SPLIT];
	uint32_t                        valid_acquired_hw;
	struct cam_ife_hw_concrete_ctx *ife_ctx;
};


/**
 * struct cam_ife_mgr_offline_in_queue - Offline IFE input request queue element
 *
 * @list:                   list private data;
 * @request_id:             request id
 * @ctx_idx:                context owning this request
 * @hw_id:                  ID of hardware core servicing this request
 * @prepare:                prepare requset data
 * @cfg:                    config request data
 * @ready:                  indicates if request is ready to be processed
 *
 */
struct cam_ife_mgr_offline_in_queue {
	struct list_head                  list;
	uint64_t                          request_id;
	uint32_t                          ctx_idx;
	uint32_t                          hw_id;
	struct cam_hw_prepare_update_args prepare;
	struct cam_hw_config_args         cfg;
	bool                              ready;
};

/**
 * struct cam_ife_hw_mgr - IFE HW Manager
 *
 * @mgr_common:                common data for all HW managers
 * @csid_devices;              csid device instances array. This will be filled
 *                             by HW manager during the initialization.
 * @ife_devices:               IFE device instances array. This will be filled
 *                             by HW layer during initialization
 * @ctx_mutex:                 mutex for the hw context pool
 * @active_ctx_cnt:            number of active contexts
 * @free_ctx_list:             free hw context list
 * @used_ctx_list:             used hw context list
 * @ctx_pool:                  HW context storage
 * @virt_ctx_pool:             HW manager context storage
 * @ife_csid_dev_caps          csid device capability stored per core
 * @ife_dev_caps               ife device capability per core
 * @work q                     work queue for IFE hw manager
 * @debug_cfg                  debug configuration
 * @ctx_lock:                  lock for context common data protection
 * @num_acquired_offline_ctx:  number of acquired offline contexts
 * @acquired_hw_pool:          offline contexts acquire data
 * @input_queue:               input request queue for offline processing
 * @in_proc_queue:             currently processed requests queuse
 * @starting_offline_cnt:      number of offline HWs that are currently starting
 * @offline_clk:               last set clock for offline processing
 * @max_clk_threshold:         min clock threshold
 * @nom_clk_threshold:         nom clock threshold
 * @min_clk_threshold:         max clock threshold
 * @bytes_per_clk:             bytes per clock processed
 */
struct cam_ife_hw_mgr {
	struct cam_isp_hw_mgr          mgr_common;
	struct cam_hw_intf            *csid_devices[CAM_IFE_CSID_HW_NUM_MAX];
	struct cam_hw_intf            *ife_devices[CAM_IFE_HW_NUM_MAX];
	struct cam_soc_reg_map        *cdm_reg_map[CAM_IFE_HW_NUM_MAX];
	struct mutex                   ctx_mutex;
	atomic_t                       active_ctx_cnt;
	struct list_head               free_ctx_list;
	struct list_head               used_ctx_list;
	struct cam_ife_hw_concrete_ctx ctx_pool[CAM_CTX_MAX];
	struct cam_ife_hw_mgr_ctx      virt_ctx_pool[CAM_CTX_MAX];
	struct cam_ife_csid_hw_caps    ife_csid_dev_caps[
						CAM_IFE_CSID_HW_NUM_MAX];
	struct cam_vfe_hw_get_hw_cap   ife_dev_caps[CAM_IFE_HW_NUM_MAX];
	struct cam_req_mgr_core_workq *workq;
	struct cam_ife_hw_mgr_debug    debug_cfg;
	spinlock_t                     ctx_lock;
	atomic_t                       num_acquired_offline_ctx;
	struct cam_ife_offline_hw      acquired_hw_pool[CAM_CTX_MAX];
	struct cam_ife_mgr_offline_in_queue   input_queue;
	struct cam_ife_mgr_offline_in_queue   in_proc_queue;
	uint32_t                       starting_offline_cnt;
	uint32_t                       offline_clk;
	uint32_t                       max_clk_threshold;
	uint32_t                       nom_clk_threshold;
	uint32_t                       min_clk_threshold;
	uint32_t                       bytes_per_clk;
};

/**
 * cam_ife_hw_mgr_init()
 *
 * @brief:              Initialize the IFE hardware manger. This is the
 *                      etnry functinon for the IFE HW manager.
 *
 * @hw_mgr_intf:        IFE hardware manager object returned
 * @iommu_hdl:          Iommu handle to be returned
 *
 */
int cam_ife_hw_mgr_init(struct cam_hw_mgr_intf *hw_mgr_intf, int *iommu_hdl);

#endif /* _CAM_IFE_HW_MGR_H_ */
