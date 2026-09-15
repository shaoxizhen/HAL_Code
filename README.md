# Test —— STM32 基础实验工程

按外设逐个推进的 STM32F103C8T6 实验工程，工具链是 STM32CubeMX + HAL 库 + Keil MDK。
一个目录一个实验，配套学习笔记放在 `E:\Code\笔记`。

## 工程列表

| 工程 | 主题 |
| --- | --- |
| `LED_Test` | GPIO 输出 / 点灯 |
| `EXTI_Test` | 外部中断（按键） |
| `TIM_Test` | 定时器基础 |
| `PWM_Test` | PWM 输出 |
| `Input_Captrue_Fre_Test` | 输入捕获测频率 / 周期 |
| `Input_Capture_Duty Cycle` | 输入捕获测脉宽 / 占空比 |
| `Input_Capture` | 输入捕获综合验收（阶段一，DAY18） |
| `UART_Test` | UART 基础：USART1 115200 8N1 单字节发送（DAY19） |

## 单个工程的标准结构

```
<工程名>/
├── Core/Inc  Core/Src        自己写的代码              → 进仓库
├── <工程名>.ioc              CubeMX 配置               → 进仓库
├── .mxproject                CubeMX 元数据             → 进仓库
├── MDK-ARM/<工程名>.uvprojx   Keil 工程文件             → 进仓库
├── MDK-ARM/<工程名>.uvoptx   调试器 / 烧录设置          → 进仓库
├── Drivers/                  CMSIS + HAL 副本           → 不进仓库
└── MDK-ARM/<工程名>/         编译产物（.o/.axf/.hex）    → 不进仓库
```

## 为什么仓库里没有 Drivers/

每个 CubeMX 工程都会各自复制一份 CMSIS + HAL，本目录下 8 个工程合计约 500 MB，
内容完全相同。仓库只保存"自己写的代码 + 工程配置"，避免库文件淹没提交记录。

**影响**：从仓库重新 clone / checkout 出来的工程缺少 `Drivers/`，Keil 编译会报找不到头文件。

**恢复办法（二选一）**：

1. 用 CubeMX 打开该工程的 `.ioc`，重新生成代码，Drivers 会按本机固件包重新复制；
2. 从本目录任意一个已有 `Drivers/` 的工程复制过去（内容相同）。

## 提交习惯

- 每个 DAY 收工提交一次，写清：做了什么、验证出什么数据、哪里还不熟。
- 动 CubeMX 之前先提交一次；重新生成后用 `git diff` 看它到底改了哪些代码。
- 提交前用 `git status` 确认没有 `Drivers/` 或编译产物被加进来。
