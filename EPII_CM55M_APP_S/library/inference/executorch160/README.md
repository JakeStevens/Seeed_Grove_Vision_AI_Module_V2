# ExecuTorch 1.6 runtime for Cortex-M55 + Ethos-U55 (HX6538)

Vendored subset of ExecuTorch 1.6.0 (commit
`ebdb936da1136d9dec9cb6128cab29ed5b44bde2`) compiled in-tree by the
WE2 Make system via `executorch160.mk`. No CMake superbuild and no
Ethos-U SDK umbrella fetch are involved: the core driver and CMSIS-NN
are vendored here, and the kernel bindings + schema headers were
generated on-host (see below).

## Contents

- `executorch/` — runtime (executor, core, backend, kernel,
  platform), prim ops, portable CPU utils, Arm delegate runtime,
  Cortex-M ops, schema, data-loader header. Test/example files are
  excluded. The `torch`/`__init__.py` Python files in
  `backends/cortex_m` are not compiled.
- `generated/` — kernel bindings produced by `codegen.gen` from the
  Cortex-M custom-ops manifest only (`operators.yaml`): the model
  executes delegate-only, so no portable Aten bindings are needed.
  `RegisterCodegenUnboxedKernels_0.cpp` is the sharded half of the
  registration and must be compiled alongside `...Everything.cpp`.
  `RegisterSchema.cpp` / `RegisterCPUCustomOps.cpp` (ATen host
  registration) are present for reference and are NOT compiled for
  bare metal.
- `third_party/ethos-u-core-driver/` — Ethos-U core driver sources
  vendored from the Himax 1.0 torchtag tree. The 1.6 delegate's weak
  fallbacks delegate `ethosu_reserve_driver_ex` to the vendored
  `ethosu_reserve_driver()`; `ethosu_init` / `ethosu_invoke_v3`
  signatures match.
- `third_party/cmsis-nn/` — CMSIS-NN v8.0.0 (GitHub, tag `v8.0.0`),
  `Include/` + `Source/`. The `*_f16.c` / `*_f32.c` sources are
  excluded in the `.mk`: they do not compile under this toolchain's
  flags and the int8 model does not use them.
- `third_party/flatbuffers/include/` — flatbuffers 24 headers.

Force-link stubs (appended to the copies, not upstream): `et16_kernels_force_link`
in the generated bindings, `executorch_ethosu_force_link` in
`EthosUBackend.cpp`. The app calls both so the linker pulls the
registration objects out of the archive.

## Regeneration

Bindings (project venv, from the ExecuTorch checkout):

```sh
python -m codegen.gen --source-path=<executorch>/codegen \
  --install-dir=<generated> \
  --tags-path=<torchgen>/packaged/ATen/native/tags.yaml \
  --aten-yaml-path=<torchgen>/packaged/ATen/native/native_functions.yaml \
  --custom-ops-yaml-path=<executorch>/backends/cortex_m/ops/operators.yaml
```

Schema headers:

```sh
cd <executorch>/schema
flatc --cpp --cpp-std c++11 --gen-mutable --scoped-enums \
  -o <schema include dir>/executorch/schema program.fbs scalar_type.fbs
```

## Memory notes

- `MAX_KERNEL_NUM=64` in the `.mk`: the kernel registry is a static
  array in the 256 KiB DATA region (23 bindings kernels + prim ops).
  256 overflowed the region once heap/stack reservations are added.
- The app linker script heap is 112 KiB (`__HEAP_SIZE = 0x1C000`,
  matching the ARM-compiler script); the firmware's real heap is the
  SRAM mm heap. ROM usage is ~94% of 256 KiB — watch it when adding
  ops.
