TARGET_MMRM_ENABLE := false
ifeq ($(TARGET_KERNEL_DLKM_DISABLE),true)
	ifeq ($(TARGET_KERNEL_DLKM_MMRM_OVERRIDE),true)
		TARGET_MMRM_ENABLE := true
	endif
else
TARGET_MMRM_ENABLE := true
endif

# Build mmrm kernel driver
ifeq ($(TARGET_MMRM_ENABLE),true)
ifeq ($(call is-board-platform-in-list,$(TARGET_BOARD_PLATFORM)),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm-mmrm.ko
BOARD_VENDOR_RAMDISK_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm-mmrm.ko
BOARD_VENDOR_RAMDISK_RECOVERY_KERNEL_MODULES_LOAD += $(KERNEL_MODULES_OUT)/msm-mmrm.ko

endif
endif
