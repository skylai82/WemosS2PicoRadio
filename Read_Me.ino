/* Important information
Board library: esp32 by Espressif Systems: 3.0.0 installed

USB CDC on boot: Enable
USB DFU on boot: Disable
Partition Scheme：RainMalker
PSRAM：Enable

这两个选项出现在 ESP32-S2 的 Arduino IDE 或 ESP-IDF 开发环境中的 “USB配置”选项中，用于控制设备在启动时通过 USB 提供的功能。以下是详细解析，适用于工程应用或开发场景：

✅ 1. USB CDC on boot: ON / OFF
CDC 代表 Communications Device Class，也就是常见的 USB虚拟串口（Serial）接口。

● ON（启用）：
ESP32-S2 上电或复位后，自动通过 USB 暴露一个串口设备（类似 CH340、CP2102 等外接串口芯片的作用）。

可以直接用 USB 线连电脑，通过 Serial.println() 打印调试信息，不再需要 UART-TTL 模块。

用于 调试、日志输出、Arduino Serial Monitor通信等场景。

● OFF（关闭）：
启动时不创建 USB CDC 接口。

若想串口通信，需改用原生 UART 接口（如 GPIO1/3）。

✅ 建议：开发调试阶段保持 ON，部署量产时可关闭以节省资源。

✅ 2. USB DFU on boot: ON / OFF
DFU 是 Device Firmware Upgrade 的缩写，符合 USB DFU 协议。

● ON（启用）：
ESP32-S2 启动时支持 通过 USB 进入 DFU 模式。

可以使用 dfu-util 工具或 ESP-IDF 的 DFU 工具链，通过 USB 直接下载固件。

无需按 BOOT 按钮或使用 UART 烧录工具，适合 OTA、量产烧录、升级维护流程。

● OFF（关闭）：
启动时不暴露 DFU 功能。

需要通过传统方式（比如 esptool.py）进行固件烧录。

✅ 建议：若你的烧录流程用不到 USB DFU，建议关闭，以防止 USB 识别冲突或安全隐患。

⚙️ 实际工程建议（总结）：
选项	建议用途	ON 适合情况	OFF 适合情况
USB CDC on boot	串口调试	开发阶段、使用 USB 调试	成品部署、关闭调试通道
USB DFU on boot	USB 烧录/升级	量产烧录、远程更新	固件已定型、安全性要求高

如您需要构建一个不依赖UART、不焊接引脚的 纯USB设备（如：WebUSB, USB MIDI, USB Audio），这两个选项的配置就会更关键。
*/