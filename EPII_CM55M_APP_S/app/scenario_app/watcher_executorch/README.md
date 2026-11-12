## SenseCAP Watcher coffee mug detection (watcher_executorch) using ExecuTorch and Ethos-U55 on WE2

- This is the HX6538 firmware for the Seeed SenseCAP Watcher. It
  runs ImageNet classification under the ExecuTorch 1.6 runtime
  on the Ethos-U55 NPU and delivers each captured image plus the
  top-5 class results to the ESP32-S3 host over the internal SPI
  link, using the SSCMA AT/JSON wire format. The ESP32 side arms
  it with the Muse gadget `watcher.coffee_mug` command: it fires
  only when a coffee mug (ImageNet target 504) is the top result
  above 75% confidence. The app is based on `torch_mb_cls`.
- Model: MobileNetV2, ImageNet 1000 classes, 224x224 int8 input,
  output (1, 1000) int8. Himax model zoo file
  `mv2_quant_himax_ini_opt_size_vela_4_5_0_inout_in8.pte`
  (3,516,944 B, SHA-256
  `70d2e36f0c0c12d972230dc3ab51a327f46fa1da112d6e9a7a346f0a045f11e9`),
  already quantized for Ethos-U (Vela 4.5.0); no export is
  needed. It is not in this repo: flash the model-zoo file with
  the firmware (see below).
- Runtime: `library/inference/executorch160` (vendored ExecuTorch
  1.6.0 subset + CMSIS-NN + Ethos-U core driver 0.16.0, built
  in-tree via `executorch160.mk`; no CMake superbuild).
- How it runs: OV5647 320x240 -> resize 224x224 -> int8
  (value-128) -> Ethos-U delegate -> top 5 of the 1000 int8
  logits, percents from the linear map `(raw + 128) * 100 / 255`
  (not softmax). Input size comes from the loaded method, so one
  binary runs either input size. Classification logic is in
  `cvapp_mb_cls.cpp` (entry points kept from `torch_mb_cls`);
  frame gating and the AT server are in `torch_mb_cls.c` /
  `watcher_at_server.cpp`; the SPI slave is
  `sscma_spi_slave.cpp` (mode 0, 12 MHz, SYNC GPIO2). A bounded
  NPU wait (`npu_diag.c`, ~2 s + register dump) makes a stalled
  inference fail loudly instead of freezing the event loop.
- Memory: read this before changing pool sizes. The mm pool is
  shared: if the ExecuTorch pools grow, the sensor WDMA
  allocation can fail and leave the raw frame buffer at
  address 0 (`WD3_RAW[0]` in the boot log), freezing inference
  on constant input while the JPEG keeps changing. Current
  sizing: temp pool 897 KiB, method pool 384 KiB. The 224x224
  delegate scratch does not fit a 640 KiB temp pool (scratch
  allocation fails on every execute), and the 736 KiB method
  pool with it exhausts the mm pool first (WD3_RAW[0]). A null
  WDMA address fails datapath init loudly (see
  `cisdp_sensor.c`).

### Linux Environment
- Prerequisites: GNU Make 4.x (Make 3.81 fails this build), Arm GNU
  Toolchain 13.2, Python 3 with `pyserial` and `xmodem` for
  flashing (see
  [xmodem/requirements.txt](https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2/tree/main/xmodem/requirements.txt)).
- Build the firmware (from `EPII_CM55M_APP_S/`):
    ```
    make APP_TYPE=watcher_executorch
    ```
  The image is `we2_image_gen_local/output_case1_sec_wlcsp/output.img`
  (maximum size is 1MB).
- Flash firmware and model in ONE full XMODEM transfer (never
  split the model: split writes corrupt flash):
    ```
    python3 xmodem/xmodem_send.py --port=[your COM number] --baudrate=921600 --protocol=xmodem --file=we2_image_gen_local/output_case1_sec_wlcsp/output.img --model="mv2_quant_himax_ini_opt_size_vela_4_5_0_inout_in8.pte 0xB7B000 0x00000"

    # example:
    # python3 xmodem/xmodem_send.py --port=/dev/ttyACM0 --baudrate=921600 --protocol=xmodem --file=we2_image_gen_local/output_case1_sec_wlcsp/output.img --model="mv2_quant_himax_ini_opt_size_vela_4_5_0_inout_in8.pte 0xB7B000 0x00000"
    ```
  - port: the Himax UART COM number, baudrate 921600.
  - Model position on flash is `0xB7B000` (runtime address
    `0x3AB7B000`, see `common_config.h`).
  - The sender waits for the 2nd-stage bootloader prompt. Reset
    the HX by hard-resetting the ESP32-S3 (which pulses HX RST)
    while the sender waits. The Watcher has a backup battery, so
    unplugging USB does NOT reset it. The transfer pauses near
    72% for ~20 s (slow flash-erase window): wait it out and use
    a 60 s ACK timeout; do not reset or unplug mid-transfer.

[Back to Outline](https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2?tab=readme-ov-file#outline)

### Windows Environment
- Same as Linux: set `APP_TYPE = watcher_executorch`, build with
  the same toolchain, disconnect `Tera Term`, and flash from `CMD`
  with backslash paths:
    ```
    python xmodem\xmodem_send.py --port=[your COM number] --baudrate=921600 --protocol=xmodem --file=we2_image_gen_local\output_case1_sec_wlcsp\output.img --model="mv2_quant_himax_ini_opt_size_vela_4_5_0_inout_in8.pte 0xB7B000 0x00000"
    ```
  All other notes above (one full transfer, ESP reset to reach
  the bootloader prompt, battery, erase pause) apply unchanged.

[Back to Outline](https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2?tab=readme-ov-file#outline)

### Coffee mug detection result
- On the HX UART (921600 baud): `Classifier Top 5` plus
  `Coffee mug (target 504): N%` or `absent` per inference. At
  boot, check `WD1[...], WD2_J[...], WD3_RAW[...]` — `WD3_RAW`
  must be nonzero; `0` means the raw frame allocation failed
  (see Memory above).
- Over SPI: `AT+INVOKE` events carry the JPEG plus `data.classes`
  with up to five entries `[[percent, target]]` best-first. The
  ESP32 host arms/disarms with the Muse gadget
  `watcher.coffee_mug` command and pushes result + image to
  Muse only when target 504 is the top entry above 75%
  confidence. No mug present reads `mug=0`; the loop keeps
  running. Scores are linear-map percents, not softmax.

[Back to Outline](https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2?tab=readme-ov-file#outline)

### Model source link
- Himax model zoo: `model_zoo/torch_mb_cls/mv2_quant_himax_ini_opt_size_vela_4_5_0_inout_in8.pte`
  (MobileNetV2, ImageNet, already quantized for Ethos-U; no
  export needed).
- [Model converted example by executorch](https://github.com/HimaxWiseEyePlus/Executorch_Convert_Example)

[Back to Outline](https://github.com/HimaxWiseEyePlus/Seeed_Grove_Vision_AI_Module_V2?tab=readme-ov-file#outline)
