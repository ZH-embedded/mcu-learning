# mcu-learning

嵌入式 MCU 学习与项目日志（2026-10-29 起，102 天计划）。
目标：2026-12-20 前完成两个可演示项目 + 简历；2027 春节前拿到 offer。

## 目录

| 目录 | 内容 |
|---|---|
| `01-c/` | C 语言练习（指针、结构体、链表、位操作） |
| `02-stm32/` | 裸机外设练习（GPIO / 中断 / 串口 / 定时器 / ADC / I2C / SPI） |
| `03-rtos/` | FreeRTOS 练习（任务、队列、互斥、优先级反转） |
| `04-comm/` | Modbus RTU、RS485、IAP 升级 |
| `notes/` | 故障记录、知识点卡片、踩坑总结 |
| `_setup/` | 环境搭建截图（工具链证据） |

## 开发板

![board](_setup/board-pinout.png)

- 主控：**待填**（看芯片丝印，如 STM32F103ZET6）
- LED：待填（如 PB5，低电平亮）
- 按键：待填（如 KEY0=PE4 / WK_UP=PA0）
- 串口：待填（如 USART1 = PA9 TX / PA10 RX，板载 CH340）
- SWD：3V3 / GND / SWDIO(PA13) / SWCLK(PA14)
- 下载方式：待填（ST-Link / DAP-Link / 串口 ISP）

## 进度

- [x] 2026-10-29 基础工具链：VS Code / gcc / Git / hello world 跑通
- [x] 2026-10-29 GitHub 仓库建立，素材管理结构就位
- [ ] 2026-10-29 STM32CubeIDE 安装 + 空工程编译通过
- [ ] 2026-10-29 串口工具 + PulseView 安装
- [ ] 第 1 周 C 语言地基
- [ ] 第 2 周 上板外设
- [ ] 项目一 环境监测终端
- [ ] FreeRTOS + Modbus
- [ ] 项目二 + IAP 升级

## 约定

- 每天至少一次 commit，哪怕只改一行（提交历史本身就是"这是我自己做的"的证据）
- 遇到的问题一律进 `notes/故障记录.md`：现象 → 复现条件 → 定位 → 根因 → 修复 → 如何避免
