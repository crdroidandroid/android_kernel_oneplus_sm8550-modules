/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef CAM_ICP_HW_MGR_H
#define CAM_ICP_HW_MGR_H

#include <linux/types.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <media/cam_icp.h>
#include "cam_icp_hw_intf.h"
#include "cam_hw_mgr_intf.h"
#include "cam_hw_intf.h"
#include "hfi_session_defs.h"
#include "hfi_intf.h"
#include "cam_req_mgr_workq.h"
#include "cam_mem_mgr.h"
#include "cam_smmu_api.h"
#include "cam_soc_util.h"
#include "cam_req_mgr_timer.h"

#define CAM_ICP_ROLE_PARENT     1
#define CAM_ICP_ROLE_CHILD      2

#define CAM_FRAME_CMD_MAX       40

#define CAM_MAX_OUT_RES         14
#define CAM_MAX_IN_RES          16

#define ICP_WORKQ_NUM_TASK      100
#define ICP_WORKQ_TASK_CMD_TYPE 1
#define ICP_WORKQ_TASK_MSG_TYPE 2

#define ICP_PACKET_SIZE         0
#define ICP_PACKET_TYPE         1
#define ICP_PACKET_OPCODE       2
#define ICP_MAX_OUTPUT_SUPPORTED 6

#define ICP_FRAME_PROCESS_SUCCESS 0
#define ICP_FRAME_PROCESS_FAILURE 1
#define ICP_MSG_BUF_SIZE        256
#define ICP_DBG_BUF_SIZE        102400

#define ICP_CLK_HW_IPE          0x0
#define ICP_CLK_HW_BPS          0x1
#define ICP_CLK_HW_MAX          0x2

#define ICP_OVER_CLK_THRESHOLD  5

#define CPAS_IPE0_BIT           0x1000
#define CPAS_IPE1_BIT           0x2000
#define CPAS_BPS_BIT            0x400

/* Used for targets >= 480 and its variants */
#define CPAS_TITAN_IPE0_CAP_BIT 0x800

#define ICP_PWR_CLP_BPS         0x00000001
#define ICP_PWR_CLP_IPE0        0x00010000
#define ICP_PWR_CLP_IPE1        0x00020000

#define CAM_ICP_CTX_STATE_FREE      0x0
#define CAM_ICP_CTX_STATE_IN_USE    0x1
#define CAM_ICP_CTX_STATE_ACQUIRED  0x2
#define CAM_ICP_CTX_STATE_RELEASE   0x3

#define CAM_ICP_CTX_MAX_CMD_BUFFERS 0x2

/* Current appliacble vote paths, based on number of UAPI definitions */
#define CAM_ICP_MAX_PER_PATH_VOTES 6

/*
 * Response time threshold in ms beyond which a request is not expected
 * to be with ICP hw
 */
#define CAM_ICP_CTX_RESPONSE_TIME_THRESHOLD   300000

struct hfi_mini_dump_info;

/**
 * struct icp_hfi_mem_info
 * @qtbl: Memory info of queue table
 * @cmd_q: Memory info of command queue
 * @msg_q: Memory info of message queue
 * @dbg_q: Memory info of debug queue
 * @sec_heap: Memory info of secondary heap
 * @fw_buf: Memory info of firmware
 * @qdss_buf: Memory info of qdss
 * @sfr_buf: Memory info for sfr buffer
 * @fw_uncached: Memory info for fw uncached region
 * @shmem: Memory info for shared region
 * @io_mem: Memory info for io region
 */
struct icp_hfi_mem_info {
	struct cam_mem_mgr_memory_desc qtbl;
	struct cam_mem_mgr_memory_desc cmd_q;
	struct cam_mem_mgr_memory_desc msg_q;
	struct cam_mem_mgr_memory_desc dbg_q;
	struct cam_mem_mgr_memory_desc sec_heap;
	struct cam_mem_mgr_memory_desc fw_buf;
	struct cam_mem_mgr_memory_desc qdss_buf;
	struct cam_mem_mgr_memory_desc sfr_buf;
	struct cam_mem_mgr_memory_desc fw_uncached;
	struct cam_smmu_region_info shmem;
	struct cam_smmu_region_info io_mem;
};

/**
 * struct hfi_cmd_work_data
 * @type: Task type
 * @data: Pointer to command data
 * @request_id: Request id
 */
struct hfi_cmd_work_data {
	uint32_t type;
	void *data;
	int32_t request_id;
};

/**
 * struct hfi_msg_work_data
 * @type: Task type
 * @data: Pointer to message data
 * @recover: Device needs recovery
 */
struct hfi_msg_work_data {
	uint32_t type;
	void *data;
	bool recover;
};

/**
 * struct clk_work_data
 * @type: Task type
 * @data: Pointer to clock info
 */
struct clk_work_data {
	uint32_t type;
	void *data;
};

/*
 * struct icp_frame_info
 * @request_id: request id
 * @io_config: the address of io config
 * @hfi_cfg_io_cmd: command struct to be sent to hfi
 * @pkt: pointer to the packet header of current request
 */
struct icp_frame_info {
	uint64_t request_id;
	dma_addr_t io_config;
	struct hfi_cmd_ipebps_async hfi_cfg_io_cmd;
	struct cam_packet *pkt;
};

/**
 * struct cam_icp_clk_bw_request_v2
 *
 * @budget_ns: Time required to process frame
 * @frame_cycles: Frame cycles needed to process the frame
 * @rt_flag: Flag to indicate real time stream
 * @reserved: Reserved filed.
 * @num_paths: Number of paths for per path bw vote
 * @axi_path: Per path vote info for IPE/BPS
 */
struct cam_icp_clk_bw_req_internal_v2 {
	uint64_t budget_ns;
	uint32_t frame_cycles;
	uint32_t rt_flag;
	uint32_t reserved;
	uint32_t num_paths;
	struct cam_cpas_axi_per_path_bw_vote axi_path[CAM_ICP_MAX_PER_PATH_VOTES];
};

#define HANG_DUMP_REGIONS_MAX 10

/**
 * struct cam_hangdump_mem_regions -
 *        List of multiple memory descriptors of different
 *        regions
 *
 * @num_regions    : Number of regions
 * @map_info_array : Array of all the regions
 */
struct cam_hangdump_mem_regions {
	uint32_t num_mem_regions;
	struct cam_cmd_mem_region_info mem_info_array[HANG_DUMP_REGIONS_MAX];
};

/**
 * struct cam_icp_ctx_perf_stats -
 *        ICP general Perf stats per ctx
 *
 * @total_resp_time: accumulative FW response time
 * @total_requests : accumulative submitted requests
 */
struct cam_icp_ctx_perf_stats {
	uint64_t total_resp_time;
	uint64_t total_requests;
};

/**
 * struct hfi_frame_process_info
 * @hfi_frame_cmd: Frame process command info
 * @bitmap: Bitmap for hfi_frame_cmd
 * @bits: Used in hfi_frame_cmd bitmap
 * @lock: Lock for hfi_frame_cmd
 * @request_id: Request id list
 * @num_out_resources: Number of out syncs
 * @out_resource: Out sync info
 * @fw_process_flag: Frame process flag
 * @clk_info: Clock information for a request
 * @clk_info_v2: Clock info for AXI bw voting v2
 * @frame_info: information needed to process request
 * @submit_timestamp: Submit timestamp to hw
 * @hangdump_mem_regions: Mem regions for hangdump
 */
struct hfi_frame_process_info {
	struct hfi_cmd_ipebps_async hfi_frame_cmd[CAM_FRAME_CMD_MAX];
	void *bitmap;
	size_t bits;
	struct mutex lock;
	uint64_t request_id[CAM_FRAME_CMD_MAX];
	uint32_t num_out_resources[CAM_FRAME_CMD_MAX];
	uint32_t out_resource[CAM_FRAME_CMD_MAX][CAM_MAX_OUT_RES];
	uint32_t in_resource[CAM_FRAME_CMD_MAX];
	uint32_t in_free_resource[CAM_FRAME_CMD_MAX];
	uint32_t fw_process_flag[CAM_FRAME_CMD_MAX];
	struct cam_icp_clk_bw_request clk_info[CAM_FRAME_CMD_MAX];
	struct cam_icp_clk_bw_req_internal_v2 clk_info_v2[CAM_FRAME_CMD_MAX];
	struct icp_frame_info frame_info[CAM_FRAME_CMD_MAX];
	ktime_t submit_timestamp[CAM_FRAME_CMD_MAX];
	struct cam_hangdump_mem_regions *hangdump_mem_regions;
};

/**
 * struct cam_ctx_clk_info
 * @curr_fc: Context latest request frame cycles
 * @rt_flag: Flag to indicate real time request
 * @base_clk: Base clock to process the request
 * @reserved: Reserved field
 * #uncompressed_bw: Current bandwidth voting
 * @compressed_bw: Current compressed bandwidth voting
 * @clk_rate: Supported clock rates for the context
 * @num_paths: Number of valid AXI paths
 * @axi_path: ctx based per path bw vote
 * @bw_included: Whether bw of this context is included in overal voting
 * @max_supported_clk_level: max supported clock level
 */
struct cam_ctx_clk_info {
	uint32_t curr_fc;
	uint32_t rt_flag;
	uint32_t base_clk;
	uint32_t reserved;
	uint64_t uncompressed_bw;
	uint64_t compressed_bw;
	int32_t clk_rate[CAM_MAX_VOTE];
	uint32_t num_paths;
	struct cam_cpas_axi_per_path_bw_vote axi_path[CAM_ICP_MAX_PER_PATH_VOTES];
	bool bw_included;
	uint32_t max_supported_clk_level;
};

/**
 * struct cam_icp_hw_ctx_data
 * @context_priv: Context private data
 * @ctx_mutex: Mutex for context
 * @fw_handle: Firmware handle
 * @scratch_mem_size: Scratch memory size
 * @acquire_dev_cmd: Acquire command
 * @icp_dev_acquire_info: Acquire device info
 * @ctxt_event_cb: Context callback function
 * @state: context state
 * @role: Role of a context in case of chaining
 * @chain_ctx: Peer context
 * @hfi_frame_process: Frame process command
 * @wait_complete: Completion info
 * @temp_payload: Payload for destroy handle data
 * @ctx_id: Context Id
 * @bw_config_version: BW config version indicator
 * @clk_info: Current clock info of a context
 * @watch_dog: watchdog timer handle
 * @watch_dog_reset_counter: Counter for watch dog reset
 * @icp_dev_io_info: io config resource
 * @last_flush_req: last flush req for this ctx
 * @err_inject_params: Error injection data for hw_mgr_ctx
 * @perf_stats: performance statistics info
 * @unified_dev_type: Unified dev type which does not hold any priority info.
 *                    It's either IPE/BPS
 * @abort_timed_out: Indicates if abort timed out
 */
struct cam_icp_hw_ctx_data {
	void *context_priv;
	struct mutex ctx_mutex;
	uint32_t fw_handle;
	uint32_t scratch_mem_size;
	struct cam_acquire_dev_cmd acquire_dev_cmd;
	struct cam_icp_acquire_dev_info *icp_dev_acquire_info;
	cam_hw_event_cb_func ctxt_event_cb;
	uint32_t state;
	uint32_t role;
	struct cam_icp_hw_ctx_data *chain_ctx;
	struct hfi_frame_process_info hfi_frame_process;
	struct completion wait_complete;
	struct ipe_bps_destroy temp_payload;
	uint32_t ctx_id;
	uint32_t bw_config_version;
	struct cam_ctx_clk_info clk_info;
	struct cam_req_mgr_timer *watch_dog;
	uint32_t watch_dog_reset_counter;
	struct cam_icp_acquire_dev_info icp_dev_io_info;
	uint64_t last_flush_req;
	char ctx_id_string[128];
	struct cam_hw_err_param err_inject_params;
	struct cam_icp_ctx_perf_stats perf_stats;
	uint32_t unified_dev_type;
	bool abort_timed_out;
};

/**
 * struct icp_cmd_generic_blob
 * @ctx: Current context info
 * @frame_info_idx: Index used for frame process info
 * @io_buf_addr: pointer to io buffer address
 */
struct icp_cmd_generic_blob {
	struct cam_icp_hw_ctx_data *ctx;
	uint32_t frame_info_idx;
	dma_addr_t *io_buf_addr;
};

/**
 * struct cam_icp_clk_info
 * @base_clk: Base clock to process request
 * @curr_clk: Current clock of hadrware
 * @prev_clk: Previous clock of hadrware
 * @threshold: Threshold for overclk count
 * @over_clked: Over clock count
 * @uncompressed_bw: Current bandwidth voting
 * @compressed_bw: Current compressed bandwidth voting
 * @num_paths: Number of AXI vote paths
 * @axi_path: Current per path bw vote info
 * @hw_type: IPE/BPS device type
 * @watch_dog: watchdog timer handle
 * @watch_dog_reset_counter: Counter for watch dog reset
 */
struct cam_icp_clk_info {
	uint32_t base_clk;
	uint32_t curr_clk;
	uint32_t prev_clk;
	uint32_t threshold;
	uint32_t over_clked;
	uint64_t uncompressed_bw;
	uint64_t compressed_bw;
	uint32_t num_paths;
	struct cam_cpas_axi_per_path_bw_vote axi_path[CAM_ICP_MAX_PER_PATH_VOTES];
	uint32_t hw_type;
	struct cam_req_mgr_timer *watch_dog;
	uint32_t watch_dog_reset_counter;
};

/**
 * struct cam_icp_hw_mgr
 * @hw_mgr_mutex: Mutex for ICP hardware manager
 * @hw_mgr_lock: Spinlock for ICP hardware manager
 * @devices: Devices of ICP hardware manager
 * @ctx_data: Context data
 * @icp_caps: ICP capabilities
 * @mini_dump_cb: Mini dump cb
 * @icp_booted: Processor is booted i.e. firmware loaded
 * @icp_resumed: Processor is powered on
 * @iommu_hdl: Non secure IOMMU handle
 * @iommu_sec_hdl: Secure IOMMU handle
 * @hfi_mem: Memory for hfi
 * @cmd_work: Work queue for hfi commands
 * @msg_work: Work queue for hfi messages
 * @timer_work: Work queue for timer watchdog
 * @msg_buf: Buffer for message data from firmware
 * @dbg_buf: Buffer for debug data from firmware
 * @icp_complete: Completion info
 * @cmd_work_data: Pointer to command work queue task
 * @msg_work_data: Pointer to message work queue task
 * @timer_work_data: Pointer to timer work queue task
 * @ctxt_cnt: Active context count
 * @ipe_ctxt_cnt: IPE Active context count
 * @bps_ctxt_cnt: BPS Active context count
 * @dentry: Debugfs entry
 * @icp_pc_flag: Flag to enable/disable power collapse
 * @ipe_bps_pc_flag: Flag to enable/disable
 *                   power collapse for ipe & bps
 * @icp_use_pil: Flag to indicate usage of PIL framework
 * @icp_debug_clk: Set clock based on debug value
 * @icp_default_clk: Set this clok if user doesn't supply
 * @clk_info: Clock info of hardware
 * @secure_mode: Flag to enable/disable secure camera
 * @icp_jtag_debug: entry to enable ICP JTAG debugging
 * @icp_debug_type : entry to enable FW debug message/qdss
 * @icp_dbg_lvl : debug level set to FW.
 * @icp_fw_dump_lvl : level set for dumping the FW data
 * @ipe0_enable: Flag for IPE0
 * @ipe1_enable: Flag for IPE1
 * @bps_enable: Flag for BPS
 * @icp_dev_intf : Device interface for ICP
 * @ipe0_dev_intf: Device interface for IPE0
 * @ipe1_dev_intf: Device interface for IPE1
 * @bps_dev_intf: Device interface for BPS
 * @ipe_clk_state: IPE clock state flag
 * @bps_clk_state: BPS clock state flag
 * @disable_ubwc_comp: Disable UBWC compression
 * @recovery: Flag to validate if in previous session FW
 *            reported a fatal error or wdt. If set FW is
 *            re-downloaded for new camera session.
 * @frame_in_process: Counter for frames in process
 * @frame_in_process_ctx_id: Contxt id processing frame
 */
struct cam_icp_hw_mgr {
	struct mutex hw_mgr_mutex;
	spinlock_t hw_mgr_lock;

	struct cam_hw_intf **devices[CAM_ICP_DEV_MAX];
	struct cam_icp_hw_ctx_data ctx_data[CAM_ICP_CTX_MAX];
	struct cam_icp_query_cap_cmd icp_caps;
	cam_icp_mini_dump_cb mini_dump_cb;

	bool icp_booted;
	bool icp_resumed;
	int32_t iommu_hdl;
	int32_t iommu_sec_hdl;
	struct icp_hfi_mem_info hfi_mem;
	struct cam_req_mgr_core_workq *cmd_work;
	struct cam_req_mgr_core_workq *msg_work;
	struct cam_req_mgr_core_workq *timer_work;
	uint32_t msg_buf[ICP_MSG_BUF_SIZE];
	uint32_t dbg_buf[ICP_DBG_BUF_SIZE];
	struct completion icp_complete;
	struct hfi_cmd_work_data *cmd_work_data;
	struct hfi_msg_work_data *msg_work_data;
	struct hfi_msg_work_data *timer_work_data;
	uint32_t ctxt_cnt;
	uint32_t ipe_ctxt_cnt;
	uint32_t bps_ctxt_cnt;
	struct dentry *dentry;
	bool icp_pc_flag;
	bool ipe_bps_pc_flag;
	bool icp_use_pil;
	uint64_t icp_debug_clk;
	uint64_t icp_default_clk;
	struct cam_icp_clk_info clk_info[ICP_CLK_HW_MAX];
	bool secure_mode;
	bool icp_jtag_debug;
	u64 icp_debug_type;
	u64 icp_dbg_lvl;
	u64 icp_fw_dump_lvl;
	bool ipe0_enable;
	bool ipe1_enable;
	bool bps_enable;
	struct cam_hw_intf *icp_dev_intf;
	struct cam_hw_intf *ipe0_dev_intf;
	struct cam_hw_intf *ipe1_dev_intf;
	struct cam_hw_intf *bps_dev_intf;
	bool ipe_clk_state;
	bool bps_clk_state;
	bool disable_ubwc_comp;
	atomic_t recovery;
	uint64_t icp_svs_clk;
	atomic_t frame_in_process;
	int frame_in_process_ctx_id;
};

/**
 * struct cam_icp_mini_dump_acquire_info - ICP mini dump device info
 *
 * @in_res: resource info used for clock and bandwidth calculation
 * @out_res: output resource
 * @num_out_res: number of output resources
 * @dev_type: device type (IPE_RT/IPE_NON_RT/BPS)
 * @secure_mode: camera mode (secure/non secure)
 */
struct cam_icp_mini_dump_acquire_info {
	struct cam_icp_res_info out_res[ICP_MAX_OUTPUT_SUPPORTED];
	struct cam_icp_res_info in_res;
	uint16_t                num_out_res;
	uint8_t                 dev_type;
	uint8_t                 secure_mode;
};

/**
 * struct hfi_frame_process_info
 * @request_id: Request id list
 * @num_out_res: Number of out syncs
 * @out_res: Out sync info
 * @in_resource: In sync info
 * @submit_timestamp: Submit timestamp to hw
 * @fw_process_flag: Frame process flag
 */
struct  hfi_frame_mini_dump_info {
	uint64_t                              request_id[CAM_FRAME_CMD_MAX];
	uint32_t                              num_out_res[CAM_FRAME_CMD_MAX];
	uint32_t                              out_res[CAM_FRAME_CMD_MAX][CAM_MAX_OUT_RES];
	uint32_t                              in_resource[CAM_FRAME_CMD_MAX];
	ktime_t                               submit_timestamp[CAM_FRAME_CMD_MAX];
	uint8_t                               fw_process_flag[CAM_FRAME_CMD_MAX];
};

/**
 * struct cam_icp_hw_ctx_mini_dump
 * @acquire: Acquire device info
 * @hfi_frame: Frame  command
 * @hw_ctx: Context private data
 * @state: context state
 * @ctx_id_string: Context id string
 * @ctx_id: Context Id
 */
struct cam_icp_hw_ctx_mini_dump {
	struct cam_icp_mini_dump_acquire_info acquire;
	struct hfi_frame_mini_dump_info       hfi_frame;
	void                                 *hw_ctx;
	char                                  ctx_id_string[128];
	uint8_t                               state;
	uint8_t                               ctx_id;
};

/**
 * struct cam_icp_hw_mini_dump_info
 *
 * @ctx: Context for minidump
 * @hfi_info: hfi info
 * @hfi_mem_info: hfi mem info
 * @fw_img: FW image
 * @recovery: To indicate if recovery is on
 * @icp_booted: Indicate if ICP is booted
 * @icp_resumed: Indicate if ICP is resumed
 * @ipe_clk_state: IPE Clk state
 * @bps_clk_state: BPS clk state
 * @disable_ubwc_comp: Indicate if ubws comp is disabled
 * @ipe0_enable: Is IPE0 enabled
 * @ipe1_enable: Is IPE1 enabled
 * @bps_enable:  Is BPS enabled
 * @icp_pc_flag: Is ICP PC enabled
 * @ipe_bps_pc_flag: Is IPE BPS PC enabled
 * @icp_use_pil: Is PIL used
 */
struct cam_icp_hw_mini_dump_info {
	struct cam_icp_hw_ctx_mini_dump   *ctx[CAM_ICP_CTX_MAX];
	struct hfi_mini_dump_info          hfi_info;
	struct icp_hfi_mem_info            hfi_mem_info;
	void                              *fw_img;
	uint32_t                           recovery;
	uint32_t                           num_context;
	bool                               icp_booted;
	bool                               icp_resumed;
	bool                               ipe_clk_state;
	bool                               bps_clk_state;
	bool                               disable_ubwc_comp;
	bool                               ipe0_enable;
	bool                               ipe1_enable;
	bool                               bps_enable;
	bool                               icp_pc_flag;
	bool                               ipe_bps_pc_flag;
	bool                               icp_use_pil;
};

static int cam_icp_mgr_hw_close(void *hw_priv, void *hw_close_args);
static int cam_icp_mgr_hw_open(void *hw_mgr_priv, void *download_fw_args);
static int cam_icp_mgr_icp_resume(struct cam_icp_hw_mgr *hw_mgr);
static int cam_icp_mgr_icp_power_collapse(struct cam_icp_hw_mgr *hw_mgr);
#endif /* CAM_ICP_HW_MGR_H */
