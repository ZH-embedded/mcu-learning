# _setup · 环境证据与引脚表

这个目录会随仓库公开：**能证明工具链真的跑起来了**，面试官点开就能看到。

## 一、跑通截图

| 文件名 | 内容 | 完成 |
|---|---|---|
| `01-gcc.png` | `gcc --version` + hello world 运行结果 | ☑ |
| `02-ide-build.png` | CubeIDE 编译成功 `0 Error(s), 0 Warning(s)` | ☐ |
| `03-downloader.jpg` | 设备管理器里的下载器（ST-Link 虚拟串口 COM4） | ☑ |
| `04-serial.png` | 串口工具（Tera Term）界面 | ☑ |
| `05-pulseview.png` | PulseView 启动界面 | ☐ |

## 二、板子照片

| 文件名 | 内容 |
|---|---|
| `board-pinout.jpg` | 整板照片（**待标注**：LED / 按键 / 串口 / SWD） |
| `chip-silkscreen.jpg` | 主控芯片丝印特写，可读 `STM32F103ZET6` |

## 三、引脚对照表

| 功能 | 板载丝印 | GPIO | 备注 |
|---|---|---|---|
| 电源灯 | LED4 | PG11 | 上电即亮，不可控 |
| LED1 | LED1 | PA0 | 低电平亮 |
| LED2 | LED2 | PA1 | 低电平亮 |
| LED3 | LED3 | PA8 | 低电平亮 |
| 复位键 | SW2 | NRST | 硬件复位 |
| KEY1 | SW3 | PF8 | 按下为高（板上已下拉） |
| KEY2 | SW4 | PF9 | 按下为高（板上已下拉） |
| KEY3 | SW5 | PF10 | 按下为低（板上已上拉） |
| KEY4 | SW6 | PF11 | 按下为低（板上已上拉） |
| 串口 1 | USART1 | PA9(TX) / PA10(RX) | MCU 调试口 8 针排座，板载 CH340 |
| 串口 2 | USART2 | PA2 / PA3 | 排针预留 |
| 串口 3 | USART3 | PB10 / PB11 | 与 I2C 复用 |
| 串口 4 | UART4 | PC10 / PC11 | 排针预留 |
| 串口 5 | UART5 | PC12 / PD2 | 排针预留 |
| SWD | — | PA13(SWDIO) / PA14(SWCLK) | 4 针：3V3 / GND / SWDIO / SWCLK |

> 建工程选型用 **STM32F103ZETx**（144 脚）。按键极性在代码里对应 `GPIO_PULLDOWN`（PF8/PF9）与 `GPIO_PULLUP`（PF10/PF11），11-10 消抖那天要用。

命名规则：`日期_序号-主题.png`，例：`2026-11-17_uart-waveform.png`
