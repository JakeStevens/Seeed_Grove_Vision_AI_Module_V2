/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

// clang-format off
#include <torch/library.h>
#include <ATen/Tensor.h>



namespace torch {
namespace executor {
namespace function {


at::Tensor & wrapper_CPU_out_quantize_per_tensor_out(const at::Tensor & input, double scale, int64_t zero_point, int64_t quant_min, int64_t quant_max, at::ScalarType dtype, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_dequantize_per_tensor_out(const at::Tensor & input, double scale, int64_t zero_point, int64_t quant_min, int64_t quant_max, at::ScalarType dtype, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_add_out(const at::Tensor & self, int64_t self_zero_point, int64_t self_multiplier, int64_t self_shift, const at::Tensor & other, int64_t other_zero_point, int64_t other_multiplier, int64_t other_shift, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_mul_out(const at::Tensor & self, int64_t self_zero_point, const at::Tensor & other, int64_t other_zero_point, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_div_out(const at::Tensor & self, int64_t self_zero_point, const at::Tensor & other, int64_t other_zero_point, int64_t output_zero_point, double output_scale, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_activation_out(const at::Tensor & input, const at::Tensor & lut, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_minimum_out(const at::Tensor & self, const at::Tensor & other, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_maximum_out(const at::Tensor & self, const at::Tensor & other, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_linear_out(const at::Tensor & input, const at::Tensor & weights, const ::std::optional<at::Tensor> & bias, const ::std::optional<at::Tensor> & kernel_sum, int64_t input_offset, int64_t filter_offset, int64_t output_offset, at::IntArrayRef requantize_multipliers, at::IntArrayRef requantize_shifts, int64_t activation_max, int64_t activation_min, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_softmax_out(const at::Tensor & input, int64_t dim, int64_t input_zero_point, int64_t output_zero_point, int64_t input_multiplier, int64_t input_shift, int64_t diff_min, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_transpose_out(const at::Tensor & input, at::IntArrayRef perm, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_pad_out(const at::Tensor & input, at::IntArrayRef pre_pad, at::IntArrayRef post_pad, int64_t pad_value, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_depthwise_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t depth_multiplier, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_depthwise_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t depth_multiplier, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_transpose_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef output_padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, const at::Tensor & output_scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_transpose_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef output_padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, const at::Tensor & output_scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_avg_pool2d_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, bool ceil_mode, int64_t zero_point, int64_t multiplier, int64_t shift, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_avg_pool2d_nhwc_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, bool ceil_mode, int64_t zero_point, int64_t multiplier, int64_t shift, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_max_pool2d_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, bool ceil_mode, int64_t input_zero_point, int64_t output_zero_point, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_max_pool2d_nhwc_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, bool ceil_mode, int64_t input_zero_point, int64_t output_zero_point, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    return out;
}


at::Tensor & wrapper_CPU_out_quantized_batch_matmul_out(const at::Tensor & lhs, int64_t lhs_zero_point, const at::Tensor & rhs_transposed, int64_t rhs_zero_point, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, const at::Tensor & scratch, at::Tensor & out) {
    return out;
}

// All out variants ops

TORCH_LIBRARY_IMPL(cortex_m, CPU, m) {
m.impl("quantize_per_tensor.out",
TORCH_FN(wrapper_CPU_out_quantize_per_tensor_out));

m.impl("dequantize_per_tensor.out",
TORCH_FN(wrapper_CPU_out_dequantize_per_tensor_out));

m.impl("quantized_add.out",
TORCH_FN(wrapper_CPU_out_quantized_add_out));

m.impl("quantized_mul.out",
TORCH_FN(wrapper_CPU_out_quantized_mul_out));

m.impl("quantized_div.out",
TORCH_FN(wrapper_CPU_out_quantized_div_out));

m.impl("quantized_activation.out",
TORCH_FN(wrapper_CPU_out_quantized_activation_out));

m.impl("minimum.out",
TORCH_FN(wrapper_CPU_out_minimum_out));

m.impl("maximum.out",
TORCH_FN(wrapper_CPU_out_maximum_out));

m.impl("quantized_linear.out",
TORCH_FN(wrapper_CPU_out_quantized_linear_out));

m.impl("softmax.out",
TORCH_FN(wrapper_CPU_out_softmax_out));

m.impl("transpose.out",
TORCH_FN(wrapper_CPU_out_transpose_out));

m.impl("pad.out",
TORCH_FN(wrapper_CPU_out_pad_out));

m.impl("quantized_conv2d.out",
TORCH_FN(wrapper_CPU_out_quantized_conv2d_out));

m.impl("quantized_conv2d_nhwc.out",
TORCH_FN(wrapper_CPU_out_quantized_conv2d_nhwc_out));

m.impl("quantized_depthwise_conv2d.out",
TORCH_FN(wrapper_CPU_out_quantized_depthwise_conv2d_out));

m.impl("quantized_depthwise_conv2d_nhwc.out",
TORCH_FN(wrapper_CPU_out_quantized_depthwise_conv2d_nhwc_out));

m.impl("quantized_transpose_conv2d.out",
TORCH_FN(wrapper_CPU_out_quantized_transpose_conv2d_out));

m.impl("quantized_transpose_conv2d_nhwc.out",
TORCH_FN(wrapper_CPU_out_quantized_transpose_conv2d_nhwc_out));

m.impl("quantized_avg_pool2d.out",
TORCH_FN(wrapper_CPU_out_quantized_avg_pool2d_out));

m.impl("quantized_avg_pool2d_nhwc.out",
TORCH_FN(wrapper_CPU_out_quantized_avg_pool2d_nhwc_out));

m.impl("quantized_max_pool2d.out",
TORCH_FN(wrapper_CPU_out_quantized_max_pool2d_out));

m.impl("quantized_max_pool2d_nhwc.out",
TORCH_FN(wrapper_CPU_out_quantized_max_pool2d_nhwc_out));

m.impl("quantized_batch_matmul.out",
TORCH_FN(wrapper_CPU_out_quantized_batch_matmul_out));

}

namespace cpu {



} // namespace cpu

} // namespace function
} // namespace executor
} // namespace torch
