# 2026-11-01-blink · 第一次上板点灯

> 第 0 周 · 第 4 天。目标不是"点亮一个灯"，是走通 **改代码 → 编译 → 烧录 → 看现象** 这个闭环。
> 闭环通了，后面所有实验都只是换代码。

## 当天验收（三条，缺一条不算完成）

- [ ] STM32 上 LED 按你写的周期闪烁，**且改一次周期后重新烧录，现象跟着变**（证明不是烧的例程）
- [ ] 亲手看过 51 的 ISP 烧录和 STM32 的 SWD 下载的差别，填完 [`ISP与SWD对比.md`](ISP与SWD对比.md)
- [ ] 两块板都亮的视频各一段；缺的配件已下单

## 前置检查（开工前 5 分钟，别跳过）

**结论：一个都不用下载，全齐了。**（2026-11-01 实际扫盘确认）

| 工具 | 状态 | 位置 / 说明 |
|---|---|---|
| Keil MDK-ARM 5.38a | ✅ 已装 | `D:\Keil_v5`，`TOOL_VARIANT=mdk_std` |
| Keil C51 | ✅ 已装 | `D:\Keil_v5\C51`，和 MDK 是两个编译器，同一个 uVision 界面 |
| Keil.STM32F1xx_DFP 2.4.1 | ✅ **已装（108 MB）** | `D:\MDK5\Keil\STM32F1xx_DFP\2.4.1`，含 F103ZE 器件定义 + `STM32F10x_512.FLM` 烧录算法 |
| STM32CubeMX | ✅ 已装 | 用来生成 Keil 工程、配时钟和引脚 |
| STM32Cube_FW_F1_V1.8.6 | ✅ **已下载** | `C:\Users\zhouh\STM32Cube\Repository\`，CubeMX 生成代码要用它，不用联网 |
| ST-Link 驱动 | ✅ 已装 | 虚拟串口 COM4，设备管理器能见到 |
| 两个 CubeIDE | 未装，今天也不装 | 见下 |

**开工第一件事：验证 Keil 认得你的芯片。**
打开 `Keil uVision5` → `Project → New uVision Project` → 在 `Device` 列表里选 `STMicroelectronics → STM32F103ZE`。**能选中就说明环境没问题，直接进步骤 B。**选不到就打开 `Pack Installer` 检查 `STM32F1xx_DFP` 是否 Enabled。

> **ST 的两个 IDE 和 Keil 的关系**（别被名字绕晕）：
> - `STM32CubeIDE`：Eclipse 内核，自带 GCC
> - `STM32CubeIDE for Visual Studio Code`：VS Code 扩展包（扩展 ID `stm32-vscode-extension`，3.x 起），走 **CMake + Ninja + Arm Clang** 工具链 + clangd 索引
> - `Keil MDK`：ARMCC/AC6 编译器，工程文件 `.uvprojx`，器件支持靠 DFP
>
> 三者**做的事一样，但工具链和工程文件互不通用**——Keil 的 `.uvprojx` 拿到 CubeIDE 里打不开，反过来也一样。你 Keil 全套已就位，今天再拉一套 CMake + Ninja + Clang bundle（几个 GB）纯属自找麻烦。
>
> 至于官网下载卡住：ST 官网要注册账号，下载环节经常失败，**这是常态不是你网络的问题**。既然一个字节都不用下，就别在这上面耗时间。
>
> 什么时候值得回头装 VS Code 版：想要 clangd 的跳转补全、想用 CMake 统一管工程、或公司要求。不是入门必需。
> 注意：**51 的 C51 编译器只有 Keil 有**，ST 的两个 IDE 完全不支持 51——这一条决定了两块板都在你这台机器上时，Keil 是唯一能通吃的工具。
> 另外：计划里 11-18 要"看反汇编"对比 `|=` 与 BSRR，Keil 的 `View → Disassembly Window` 一样能看。



## 接线（先接再上电）

STM32 板（尚硅谷 F103ZET6）：

| ST-Link | 板子 | 备注 |
|---|---|---|
| 3V3 | 3V3 | **不要接 5V**，也别同时插两根 USB 供电 |
| GND | GND | 必须共地，否则识别不到 |
| SWDIO | PA13 | |
| SWCLK | PA14 | |

板载 LED：**PA0 / PA1 / PA8，低电平点亮**（引脚为 0 时亮）。所以"点亮"是 `HAL_GPIO_WritePin(..., GPIO_PIN_RESET)`，别写成 SET。

**51 板：STC89C52RC**（8051 内核 / 8 KB Flash / 512 B RAM / 11.0592 MHz 晶振居多）
板载 LED 一般接在 P1 或 P2 口，**低电平点亮**（写 `0` 亮，写 `1` 灭）——和 STM32 一样是低电平有效，这点今天会重复遇到两次。

## 步骤 A · 51 点灯（限时 60 分钟，卡住就跳过）

### A1. 先确认下载通道

STC89C52RC 只能通过**串口**烧录，走 `P3.0 (RXD)` / `P3.1 (TXD)`：

- 51 板上自带 USB 转串口（有 CH340/PL2303 芯片和 USB 口）→ 一根 USB 线，设备管理器里会多一个 COM 口
- 没有 → 需要 USB-TTL 模块，**交叉接**：模块 `TX` → 板子 `P3.0(RXD)`，模块 `RX` → 板子 `P3.1(TXD)`，`GND` 接 `GND`

⚠️ `P3.0/P3.1` 烧录时被占用，所以这两脚上不能同时挂别的设备。另外 **TX/RX 接反是 51 最常见的不出效果原因**，一直卡在"正在检测目标单片机"就先去对调这两根线。

### A2. Keil C51 建工程

1. `Project → New uVision Project` → Device 选 **`Atmel → AT89C52`**（STC89C52RC 与它寄存器兼容，能直接用）
   想更规范：打开 STC-ISP → 右侧「Keil 仿真设置」→ 点「添加型号和头文件到 Keil 中」→ 之后 Device 里能直接选 `STC89C52RC`
2. 新建 `main.c`：

```c
#include <reg52.h>          /* STC 库装好后也可以 #include <STC89C52RC.H> */

void main(void)
{
    while (1)
    {
        P1 = 0x00;          /* 低电平点亮：8 个灯全亮 */
        /* 想闪就加延时，但没有库，得自己写 for 空循环 */
    }
}
```

3. **`Options for Target → Output` 勾上 `Create HEX File`** ← 不勾的话 STC-ISP 里没文件可下，这是第二大坑
4. `F7` 编译，确认 `0 Error(s)`，工程目录下出现 `.hex`

### A3. 用 STC-ISP 烧进去

`D:\工具\51\51烧录工具\STC-ISP-v6.96.exe`

1. 芯片型号选 **`STC89C52RC`**（选错会提示型号不匹配）
2. 串口选对、波特率先设 `2400`（不稳定再往上试，别一上来就 115200）
3. 点「打开程序文件」选刚编译出的 `.hex`
4. 点 **「下载/编程」** → 然后**给板子断电，再重新上电**
   STC 的引导程序只在**冷启动那一刻**检查有没有下载请求，所以必须先点下载、后上电。顺序反了就是一直停在"正在检测目标单片机..."——**这是 51 最反直觉的一步**，也是它和 STM32 最本质的区别
5. 灯亮 → 录视频

> **超过 1 小时没点亮就停手**，记下卡在哪一步，明天再查。今天的重点是 STM32，51 只是用来对比"两种下载方式"。


## 步骤 B · STM32 点灯（主线，至少留 2 小时）

1. **CubeMX 新建工程** → 搜 `STM32F103ZE` → 选 `STM32F103ZETx`
2. **先配 SYS**：`System Core → SYS → Debug` 选 **`Serial Wire`**
   ⚠️ 这一步漏了，烧进去的程序会把 SWD 引脚复用掉，**下次就连不上了**，只能靠按住复位键或接 BOOT0 救回来。这是新手第一天最容易中的招
3. **配时钟**：`System Core → RCC` → `High Speed Clock (HSE)` 选 `Crystal/Ceramic Resonator`
   然后切到顶部 **`Clock Configuration`** 标签，从左往右四个环节依次过一遍：

   | 环节 | 选什么 | 对应寄存器位 | 说明 |
   |---|---|---|---|
   | `Input frequency` | **8** MHz | — | 板上晶振频率 |
   | `PLL Source Mux` | **HSE** | `RCC_CFGR.PLLSRC` | 给 PLL 供 8MHz |
   | `PLLMul` | **`X9`** | `RCC_CFGR.PLLMUL` | 8 × 9 = 72MHz |
   | `System Clock Mux` | **`PLLCLK`** | `RCC_CFGR.SW` | 把 PLL 输出接到 SYSCLK，**这一步最容易漏** |
   | `HCLK (MHz)` | **72** | `RCC_CFGR.HPRE` | AHB 不分频 |
   | `APB1 Prescaler` | **`/2`**（36MHz） | `RCC_CFGR.PPRE1` | F103 的 APB1 上限就是 36MHz，填 72 会报橙色超频警告 |
   | `APB2 Prescaler` | **`/1`**（72MHz） | `RCC_CFGR.PPRE2` | APB2 上限 72MHz |

   偷懒做法：直接在 `HCLK` 框里输入 `72` 回车，CubeMX 会自动把上面几个 Mux 一并解算好。但**手动走一遍才看得懂 `SystemClock_Config()` 里那串 `RCC->CFGR` 赋值在干什么**——第 2 周（11-17）要用寄存器重配一遍时钟树，现在先建立印象。

   > 为什么必须用外部晶振 HSE？内部 HSI 也是 8MHz，但它走 PLL 只能先 `/2` 得 4MHz，最高 ×16 = 64MHz，**够不到 72**。所以 F103 想跑满 72MHz 就必须接晶振。

4. **配 GPIO**：点 PA0 → 选 `GPIO_Output`；`GPIO Settings` 里把 `GPIO output level` 设成 `High`（高电平=灭，上电不会突然亮）
5. **生成工程**：`Project Manager → Toolchain/IDE` 选 **`MDK-ARM`**，版本选 `V5`；工程名 `Blink_F103`
6. Keil 打开生成的 `.uvprojx` → `Options for Target → Debug` 选 `ST-Link Debugger` → 右边 `Settings` 里 `Port` 选 `SW`，能读到 `IDCODE` 才算连上
7. `Flash → Configure Flash Tools` 勾 **`Reset and Run`**（否则烧完要手动按复位）
8. 在 `main.c` 的 `while (1)` 里写翻转：

```c
/* USER CODE BEGIN 3 */
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
HAL_Delay(500);          /* 500ms 亮、500ms 灭 */
/* USER CODE END 3 */
```

9. `F7` 编译 → `F8` 下载 → 看灯
10. **改周期**：`HAL_Delay(500)` 改成 `100`，重新编译下载，确认闪得更快 ← 这一步是验收的关键，证明闭环真的通了

## 卡壳预案

| 现象 | 先查什么 |
|---|---|
| Keil 的 Device 里搜不到 STM32F103ZE | 打开 `Pack Installer` 看 `STM32F1xx_DFP` 是否 Enabled；确认 `Options for Target → Device` 里没勾错系列 |
| `Error: Flash Download failed - Target DLL has been cancelled` | Debug 页没选对 ST-Link，或 SWDIO/SWCLK 接反 |
| `No target connected` / `IDCODE` 读不到 | ① 3V3 和 GND 没接 ② 板子没上电 ③ 之前烧的过程序关了 SWD → 按住复位键再点下载 |
| 烧录成功但灯不亮 | LED 是低电平点亮，检查写的是 `RESET` 还是 `SET`；或选的引脚不是板上那颗灯 |
| 灯常亮不闪 | `HAL_Delay` 没生效 → 时钟没配成 72MHz（`SystemClock_Config` 里看），或卡在 `HAL_Init` |
| 编译报 `cannot open source input file "stm32f1xx_hal.h"` | CubeMX 生成时没勾 `Copy only the necessary library files`，或路径含中文/空格 |
| **51**：STC-ISP 一直停在"正在检测目标单片机..." | ① 没冷启动（要先点下载再上电）② TX/RX 接反 ③ 波特率太高，降到 2400 ④ P3.0/P3.1 被其他模块占用 ⑤ 板子独立供电没共地 |
| **51**：STC-ISP 提示"型号不匹配" | 芯片型号选错了，STC89C52RC 和 STC89C52（无 RC）固件不同 |
| **51**：Keil 编译成功但找不到 `.hex` | `Options for Target → Output` 没勾 `Create HEX File` |
| **51**：烧录成功灯不亮 | 低电平点亮，`P1 = 0x00` 才是全亮；另外不同板子 LED 可能接在 P1 或 P2 |

## 当天产出

- [ ] `_shots/2026-11/2026-11-01_01-51-blink.mp4`（51 灯亮，10 秒够）
- [ ] `_shots/2026-11/2026-11-01_02-stm32-blink.mp4`（STM32 闪烁，能看出周期变化最好）
- [ ] `_shots/2026-11/2026-11-01_03-keil-build.png`（`0 Error(s), 0 Warning(s)` 那行）
- [ ] `_shots/2026-11/2026-11-01_04-stlink-idcode.png`（Debug 设置里读到 IDCODE）
- [ ] 缺的配件已下单（`__________`）
- [ ] 收尾：`notes/每日日志.md` 写今天的卡点；commit 一次

> 截图和视频放 `_shots/`（已在 `.gitignore` 里，不进仓库）。**要进仓库的是**：CubeMX 工程（`Blink_F103/`）、对比文档、日志。

## 生成物位置

CubeMX 生成的工程放本目录下：

```
02-stm32/2026-11-01-blink/
├── README.md                    ← 本文件
├── ISP与SWD对比.md               ← 当天学习产出
├── 2026-11-01_blink-on.jpg      ← 灯亮实物照片（入库证据）
├── 2026-11-01_keil-0-error.jpg  ← 编译 0 Error 截图（入库证据）
└── Blink_F103/                  ← CubeMX 生成（含 .ioc、MDK-ARM/ 子目录）
    ├── Blink_F103.ioc
    ├── Core/Src/main.c          ← 改 while(1) 的地方
    └── MDK-ARM/Blink_F103.uvprojx
```

`.ioc` 文件一定要进 Git——它是整个工程的"配方"，重装系统后靠它能一键重建。`.uvprojx` 也进，它是 Keil 的工程文件。
