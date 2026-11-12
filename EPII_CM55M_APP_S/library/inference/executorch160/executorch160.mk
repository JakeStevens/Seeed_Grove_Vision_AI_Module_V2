# directory declaration
# ExecuTorch 1.6 runtime for Cortex-M55 + Ethos-U55, built in-tree by the
# WE2 Make system (no CMake superbuild, no Ethos-U SDK umbrella fetch:
# the core driver and CMSIS-NN are vendored here, and the kernel
# bindings + schema headers were generated on-host with flatc and
# codegen.gen from the 1.6 tree; see library README).
LIB_INFERENCE_ENGINE_DIR = $(LIBRARIES_ROOT)/inference/executorch160
LIB_INFERENCE_ENGINE_DEFINES = -DEXECUTORCH160
LIB_INFERENCE_ENGINE_ASMSRCDIR	= $(LIB_INFERENCE_ENGINE_DIR)
LIB_INFERENCE_ENGINE_CSRCDIR	= $(LIB_INFERENCE_ENGINE_DIR)
LIB_INFERENCE_ENGINE_CXXSRCSDIR    = $(LIB_INFERENCE_ENGINE_DIR)

LIB_INFERENCE_ENGINE_INCDIR	= $(LIB_INFERENCE_ENGINE_DIR) $(LIB_INFERENCE_ENGINE_DIR)/executorch \
				$(LIB_INFERENCE_ENGINE_DIR)/third_party/ethos-u-core-driver/include \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/arm/runtime \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/backend \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/core/portable_type/c10 \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/kernels \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/schema \
				$(LIB_INFERENCE_ENGINE_DIR)/third_party/flatbuffers/include \
				$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/cortex_m/ops \
				$(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Include/Internal \
				$(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Include \
				$(LIB_INFERENCE_ENGINE_DIR)/generated

LIB_INFERENCE_ENGINE_CSRCS += \
$(LIB_INFERENCE_ENGINE_DIR)/third_party/ethos-u-core-driver/src/ethosu_pmu.c \
$(LIB_INFERENCE_ENGINE_DIR)/third_party/ethos-u-core-driver/src/ethosu_driver.c \
$(LIB_INFERENCE_ENGINE_DIR)/third_party/ethos-u-core-driver/src/ethosu_device_u55_u65.c \

LIB_INFERENCE_ENGINE_CSRCS += \
$(filter-out $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/BasicMathFunctions/arm_elementwise_add_f16.c \
             $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/BasicMathFunctions/arm_elementwise_add_f32.c, \
             $(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/BasicMathFunctions/*c))) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/ActivationFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/ConcatenationFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/ConvolutionFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/DepthwiseConvFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/FullyConnectedFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/NNSupportFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/PoolingFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/ReshapeFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/SoftmaxFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/SVDFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/PadFunctions/*c)) \
$(filter-out %_f16.c %_f32.c, $(wildcard $(LIB_INFERENCE_ENGINE_DIR)/third_party/cmsis-nn/Source/TransposeFunctions/*c)) \

LIB_INFERENCE_ENGINE_CXXSRCS += \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/method_meta.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/method.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/program.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/program_validation.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/pte_data_map.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/tensor_parser_portable.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/core/exec_aten/util/tensor_util_portable.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/core/exec_aten/util/tensor_shape_to_c_string.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/executor/tensor_parser_exec_aten.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/arm/runtime/EthosUBackend.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/arm/runtime/EthosUBackend_CoreDriver.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/arm/runtime/EthosUBackend_IoMemcpy.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/arm/runtime/VelaBinStream.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/kernels/portable/cpu/util/broadcast_util.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/executorch/kernels/portable/cpu/util/repeat_util.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/generated/RegisterCodegenUnboxedKernelsEverything.cpp \
$(LIB_INFERENCE_ENGINE_DIR)/generated/RegisterCodegenUnboxedKernels_0.cpp \

LIB_INFERENCE_ENGINE_CXXSRCS += \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/backend/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/core/portable_type/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/core/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/kernel/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/runtime/platform/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/schema/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/kernels/prim_ops/*.cpp) \
$(wildcard $(LIB_INFERENCE_ENGINE_DIR)/executorch/backends/cortex_m/ops/*.cpp) \

LIB_INFERENCE_ENGINE_ASMSRCS =

LIB_INFERENCE_ENGINE_COBJS = $(call get_relobjs, $(LIB_INFERENCE_ENGINE_CSRCS))
LIB_INFERENCE_ENGINE_CXXOBJS = $(call get_relobjs, $(LIB_INFERENCE_ENGINE_CXXSRCS))
LIB_INFERENCE_ENGINE_ASMOBJS = $(call get_relobjs, $(LIB_INFERENCE_ENGINE_ASMSRCS))

LIB_INFERENCE_ENGINE_OBJS = $(LIB_INFERENCE_ENGINE_COBJS) $(LIB_INFERENCE_ENGINE_ASMOBJS) $(LIB_INFERENCE_ENGINE_CXXOBJS)

LIB_INFERENCE_ENGINE_DEPS = $(call get_deps, $(LIB_INFERENCE_ENGINE_OBJS))

LIB_INFERENCE_ENGINE_DEFINES += -DC10_USING_CUSTOM_GENERATED_MACROS -D__XTENSA__
LIB_INFERENCE_ENGINE_DEFINES += -DETHOSU_ARCH=u55 -DETHOSU55 -DETHOSU_LOG_SEVERITY=ETHOSU_LOG_WARN -DETHOS_U
LIB_INFERENCE_ENGINE_DEFINES += -DFalse=false
# Kernel registry is a static array sized by MAX_KERNEL_NUM (23
# kernels in the generated bindings + prim ops). 256 overflowed the
# 256 KiB DATA region once the heap/stack reservations are added;
# 64 leaves headroom and still fits every registered kernel.
LIB_INFERENCE_ENGINE_DEFINES += -DMAX_KERNEL_NUM=64

INFERENCE_ENGINE_LIB_NAME = libexecutorch160_$(TOOLCHAIN).a

INFERENCE_ENGINE_LIB_CSRCDIR += $(LIB_INFERENCE_ENGINE_CSRCDIR)
INFERENCE_ENGINE_LIB_CXXSRCDIR += $(LIB_INFERENCE_ENGINE_CXXSRCSDIR)
INFERENCE_ENGINE_LIB_ASMSRCDIR += $(LIB_INFERENCE_ENGINE_ASMSRCDIR)

INFERENCE_ENGINE_LIB_COBJS += $(LIB_INFERENCE_ENGINE_COBJS)
INFERENCE_ENGINE_LIB_CXXOBJS += $(LIB_INFERENCE_ENGINE_CXXOBJS)
INFERENCE_ENGINE_LIB_ASMOBJS += $(LIB_INFERENCE_ENGINE_ASMOBJS)
INFERENCE_ENGINE_LIB_ALLOBJS += $(LIB_INFERENCE_ENGINE_OBJS)

LIB_DEFINES += $(LIB_INFERENCE_ENGINE_DEFINES)
LIB_DEPS += $(LIB_INFERENCE_ENGINE_DEPS)

ifeq ($(INFERENCE_FORCE_PREBUILT), y)
override LIB_INFERENCE_ENGINE_OBJS:=
endif

INFERENCE_ENGINE_LIB := $(subst /,$(PS), $(strip $(OUT_DIR)/$(INFERENCE_ENGINE_LIB_NAME)))

$(INFERENCE_ENGINE_LIB): $(LIB_INFERENCE_ENGINE_OBJS)
	$(TRACE_ARCHIVE)
ifeq "$(strip $(LIB_INFERENCE_ENGINE_OBJS))" ""
	$(CP) $(PREBUILT_LIB)$(INFERENCE_ENGINE_LIB_NAME) $(INFERENCE_ENGINE_LIB)
else
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_INFERENCE_ENGINE_OBJS)
	$(CP) $(INFERENCE_ENGINE_LIB) $(PREBUILT_LIB)$(INFERENCE_ENGINE_LIB_NAME)
endif
