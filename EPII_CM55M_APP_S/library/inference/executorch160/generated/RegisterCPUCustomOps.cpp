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

#include "CustomOpsNativeFunctions.h"

namespace torch {
namespace executor {
namespace function {

namespace {

at::Tensor & wrapper_CPU_out_quantize_per_tensor_out(const at::Tensor & input, double scale, int64_t zero_point, int64_t quant_min, int64_t quant_max, at::ScalarType dtype, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantize_per_tensor_out(input, scale, zero_point, quant_min, quant_max, dtype, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_dequantize_per_tensor_out(const at::Tensor & input, double scale, int64_t zero_point, int64_t quant_min, int64_t quant_max, at::ScalarType dtype, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::dequantize_per_tensor_out(input, scale, zero_point, quant_min, quant_max, dtype, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_add_out(const at::Tensor & self, int64_t self_zero_point, int64_t self_multiplier, int64_t self_shift, const at::Tensor & other, int64_t other_zero_point, int64_t other_multiplier, int64_t other_shift, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_add_out(self, self_zero_point, self_multiplier, self_shift, other, other_zero_point, other_multiplier, other_shift, output_zero_point, output_multiplier, output_shift, activation_min, activation_max, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_mul_out(const at::Tensor & self, int64_t self_zero_point, const at::Tensor & other, int64_t other_zero_point, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_mul_out(self, self_zero_point, other, other_zero_point, output_zero_point, output_multiplier, output_shift, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_div_out(const at::Tensor & self, int64_t self_zero_point, const at::Tensor & other, int64_t other_zero_point, int64_t output_zero_point, double output_scale, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_div_out(self, self_zero_point, other, other_zero_point, output_zero_point, output_scale, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_activation_out(const at::Tensor & input, const at::Tensor & lut, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_activation_out(input, lut, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_minimum_out(const at::Tensor & self, const at::Tensor & other, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::minimum_out(self, other, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_maximum_out(const at::Tensor & self, const at::Tensor & other, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::maximum_out(self, other, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_linear_out(const at::Tensor & input, const at::Tensor & weights, const ::std::optional<at::Tensor> & bias, const ::std::optional<at::Tensor> & kernel_sum, int64_t input_offset, int64_t filter_offset, int64_t output_offset, at::IntArrayRef requantize_multipliers, at::IntArrayRef requantize_shifts, int64_t activation_max, int64_t activation_min, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_linear_out(input, weights, bias, kernel_sum, input_offset, filter_offset, output_offset, requantize_multipliers, requantize_shifts, activation_max, activation_min, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_softmax_out(const at::Tensor & input, int64_t dim, int64_t input_zero_point, int64_t output_zero_point, int64_t input_multiplier, int64_t input_shift, int64_t diff_min, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::softmax_out(input, dim, input_zero_point, output_zero_point, input_multiplier, input_shift, diff_min, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_transpose_out(const at::Tensor & input, at::IntArrayRef perm, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::transpose_out(input, perm, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_pad_out(const at::Tensor & input, at::IntArrayRef pre_pad, at::IntArrayRef post_pad, int64_t pad_value, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::pad_out(input, pre_pad, post_pad, pad_value, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_conv2d_out(input, weight, bias, stride, padding, dilation, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_conv2d_nhwc_out(input, weight, bias, stride, padding, dilation, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_depthwise_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t depth_multiplier, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_depthwise_conv2d_out(input, weight, bias, stride, padding, dilation, depth_multiplier, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_depthwise_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, int64_t depth_multiplier, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_depthwise_conv2d_nhwc_out(input, weight, bias, stride, padding, dilation, depth_multiplier, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_transpose_conv2d_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef output_padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, const at::Tensor & output_scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_transpose_conv2d_out(input, weight, bias, stride, padding, output_padding, dilation, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, output_scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_transpose_conv2d_nhwc_out(const at::Tensor & input, const at::Tensor & weight, const ::std::optional<at::Tensor> & bias, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef output_padding, at::IntArrayRef dilation, int64_t input_offset, int64_t output_offset, const at::Tensor & requantize_multipliers, const at::Tensor & requantize_shifts, int64_t activation_min, int64_t activation_max, const at::Tensor & scratch, const at::Tensor & output_scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_transpose_conv2d_nhwc_out(input, weight, bias, stride, padding, output_padding, dilation, input_offset, output_offset, requantize_multipliers, requantize_shifts, activation_min, activation_max, scratch, output_scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_avg_pool2d_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, bool ceil_mode, int64_t zero_point, int64_t multiplier, int64_t shift, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_avg_pool2d_out(input, kernel_size, stride, padding, ceil_mode, zero_point, multiplier, shift, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_avg_pool2d_nhwc_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, bool ceil_mode, int64_t zero_point, int64_t multiplier, int64_t shift, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_avg_pool2d_nhwc_out(input, kernel_size, stride, padding, ceil_mode, zero_point, multiplier, shift, scratch, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_max_pool2d_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, bool ceil_mode, int64_t input_zero_point, int64_t output_zero_point, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_max_pool2d_out(input, kernel_size, stride, padding, dilation, ceil_mode, input_zero_point, output_zero_point, activation_min, activation_max, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_max_pool2d_nhwc_out(const at::Tensor & input, at::IntArrayRef kernel_size, at::IntArrayRef stride, at::IntArrayRef padding, at::IntArrayRef dilation, bool ceil_mode, int64_t input_zero_point, int64_t output_zero_point, int64_t activation_min, int64_t activation_max, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_max_pool2d_nhwc_out(input, kernel_size, stride, padding, dilation, ceil_mode, input_zero_point, output_zero_point, activation_min, activation_max, out);
}

} // anonymous namespace

namespace {

at::Tensor & wrapper_CPU_out_quantized_batch_matmul_out(const at::Tensor & lhs, int64_t lhs_zero_point, const at::Tensor & rhs_transposed, int64_t rhs_zero_point, int64_t output_zero_point, int64_t output_multiplier, int64_t output_shift, const at::Tensor & scratch, at::Tensor & out) {
    // No device check


  // DeviceGuard omitted
  return cortex_m::native::quantized_batch_matmul_out(lhs, lhs_zero_point, rhs_transposed, rhs_zero_point, output_zero_point, output_multiplier, output_shift, scratch, out);
}

} // anonymous namespace

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
