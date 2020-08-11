ccflags-y += -I$(srctree)/techpack/ais/drivers/cam_core/
ccflags-y += -I$(srctree)/techpack/ais/drivers/cam_req_mgr/
ccflags-y += -I$(srctree)/techpack/ais/drivers/cam_smmu/
ccflags-y += -I$(srctree)/techpack/ais/drivers/cam_utils

obj-$(CONFIG_MSM_AIS) += cam_soc_util.o cam_io_util.o cam_packet_util.o cam_debug_util.o cam_trace.o cam_common_util.o cam_compat.o

ifdef CONFIG_QCOM_CX_IPEAK
obj-$(CONFIG_MSM_AIS) += cam_cx_ipeak.o
endif

ifneq (,$(filter $(CONFIG_QCOM_BUS_SCALING),y m))
obj-y += cam_soc_bus.o
endif

ifneq (,$(filter $(CONFIG_INTERCONNECT_QCOM),y m))
obj-y += cam_soc_icc.o
endif