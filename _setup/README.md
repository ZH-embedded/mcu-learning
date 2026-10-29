# _setup · 环境证据


## 需要的 5 张（10-29 完成）

| 文件名 | 内容 | 完成 |
|---|---|---|
| `d:\mcu-learning\_setup\屏幕截图 2026-10-29 234518.png` | 终端里 `gcc --version` + hello world 运行结果 | ☐ |
| `02-ide-build.png` | CubeIDE 编译成功，`0 Error(s), 0 Warning(s)` | ☐ |
| `c:\Users\zhouh\Pictures\03-downloader.jpg` | 设备管理器里的下载器（ST-Link / DAP-Link / CH340） | ☐ |
| `c:\Users\zhouh\Pictures\Screenshots\04-serial.png` | 串口工具（Tera Term）打开界面 | ☐ |
| `05-pulseview.png` | PulseView 启动界面 | ☐ |




| 功能 | 板载丝印 | GPIO | 备注 |
|---|---|---|---|
| LED0 | LED4 | PG11 | 电源灯 |
| LED1 | LED1 | PA0 | 低电平亮 |
| LED2 | LED2 | PA1 | 低电平亮 |
| LED3 | LED3 | PA8 | 低电平亮 |
| KEY0 | SW2 | NRST | 复位 |
| KEY1 | SW3 | PF8 | 按下为高，板上已拉低 |
| KEY2 | SW4 | PF9 | 按下为高，板上已拉低 |
| KEY3 | SW5 | PF10 | 按下为低，板上已上拉 |
| KEY4 | SW6 | PF11 | 按下为低，板上已上拉 |
| 串口 | USART1 | PA9/PA10 | MCU调试口8针排坐 |
| 串口 | USART2 | PA2/PA3 | 接排针预留口 |
| 串口 | USART3 | PB10/PB11 | 接I2C |
| 串口 | USART4 | PC10/PC11 | 接排针预留口 |
| 串口 | USART5 | PC12/PD2 | 接排针预留口 |
| SWD | — | PA13/PA14 | 4 针排针：3V3/GND/SWDIO/SWCLK |

- `c:\Users\zhouh\Pictures\chip-silkscreen.jpg`　主控芯片丝印特写（确认型号，建工程要用）


