# Learn —— STM32 基础实验工程

按外设逐个推进的 STM32F103C8T6 实验工程，工具链是 STM32CubeMX + HAL 库 + Keil MDK。
一个目录一个实验；外层目录用两位序号排序，工程内部名称保持原样。配套学习笔记放在 `E:\Code\笔记`。

## 工程列表

| 工程 | 主题 |
| --- | --- |
| `01_LED_Test` | GPIO 输出 / 点灯 |
| `02_EXTI_Test` | 外部中断（按键） |
| `03_TIM_Test` | 定时器基础 |
| `04_PWM_Test` | PWM 输出 |
| `05_Input_Captrue_Fre_Test` | 输入捕获测频率 / 周期 |
| `06_Input_Capture_Duty Cycle` | 输入捕获测脉宽 / 占空比 |
| `07_Input_Capture` | 输入捕获综合验收（阶段一，DAY18） |
| `08_UART_Test` | UART 基础：USART1 115200 8N1 单字节发送（DAY19） |
| `09_UART_Tx` | UART 发送：HAL_UART_Transmit 发字符串 + printf 重定向（DAY20） |
| `10_UART_RX` | UART 接收：阻塞接收攒指令，电脑发 ON/OFF 控制 LED（DAY21） |
| `11_I2C_Test` | I2C1 基础：检查设备地址是否应答 |
| `12_I2C_Debug` | I2C1 调试：检查设备应答并读取 MPU6050 的 WHO_AM_I 寄存器 |
| `13_I2C_MPU6050` | 读取 MPU6050 加速度与角速度数据，并在 OLED 上显示角速度 |

## 单个工程的标准结构

```
<序号>_<工程名>/
├── Core/Inc  Core/Src        自己写的代码              → 进仓库
├── <工程名>.ioc              CubeMX 配置               → 进仓库
├── .mxproject                CubeMX 元数据             → 进仓库
├── MDK-ARM/<工程名>.uvprojx   Keil 工程文件             → 进仓库
├── MDK-ARM/<工程名>.uvoptx   调试器 / 烧录设置          → 进仓库
├── Drivers/                  CMSIS + HAL 副本           → 不进仓库
└── MDK-ARM/<工程名>/         编译产物（.o/.axf/.hex）    → 不进仓库
```

## 为什么仓库里没有 Drivers/

每个 CubeMX 工程都会各自复制一份 CMSIS + HAL；本目录现有 13 个工程，
驱动副本体积大且内容高度重复。仓库只保存"自己写的代码 + 工程配置"，避免库文件淹没提交记录。

**影响**：从仓库重新 clone / checkout 出来的工程缺少 `Drivers/`，Keil 编译会报找不到头文件。

**恢复办法（二选一）**：

1. 用 CubeMX 打开该工程的 `.ioc`，重新生成代码，Drivers 会按本机固件包重新复制；
2. 从使用相同 STM32 固件包版本的已有工程复制 `Drivers/`。

部分源码中的中文注释使用 GBK 编码；在 GitHub 网页预览中可能显示乱码，本地查看时请选用 GBK/CP936 编码。

## 提交习惯

- 每个 DAY 收工提交一次，写清：做了什么、验证出什么数据、哪里还不熟。
- 动 CubeMX 之前先提交一次；重新生成后用 `git diff` 看它到底改了哪些代码。
- 提交前用 `git status` 确认没有 `Drivers/` 或编译产物被加进来。
