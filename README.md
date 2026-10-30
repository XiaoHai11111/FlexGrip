# FlexGrip

FlexGrip 是一个基于 ESP32-PICO-D4 的直流有刷电机夹爪项目。一片 DRV8833CPWPR 双 H 桥驱动两个直流有刷电机，分别控制左、右爪；两路 RDC506018A 模拟角度传感器提供关节角度反馈，固件实现位置闭环。夹爪支持串口与 CAN 两种控制方式，CAN 协议兼容 DummyRobot 机械臂主控的 DummyHand 指令格式。

项目目前处于 **夹爪角度闭环已完成并实测验收** 阶段：0~45 全范围位置控制、串口命令与在线调参已通过 COM7 实测；CAN 控制代码已实现，待与机械臂主控实物联调。

## 当前进展

- [x] 双路直流有刷电机 DRV8833 H 桥驱动，左右爪独立 PWM
- [x] 两路 RDC506018A 模拟角度编码器读取与电压→角度标定
- [x] 电机电流采样（基础实现，暂未接入控制闭环）
- [x] 夹爪运动学：0~45 输入 ↔ 左右电机目标角度映射
- [x] 位置闭环：匀速运动轨迹 + 速度前馈 + 速度阻尼 + 到位积分
- [x] 串口命令控制与在线调参（angle / open / close / pid / ki / speed / ff 等）
- [x] CAN 控制（DummyHand 协议兼容：set_angle / set_max_current）
- [x] SolidWorks 结构模型与 ROS/URDF + Gazebo 仿真包
- [x] 原理图、PCB Gerber 与网表
- [ ] CAN 与机械臂主控实物联调
- [ ] 电流/夹持力闭环
- [ ] 编码器零位自动标定（ANGLE_OFFSET 尚未启用）
- [ ] 控制参数非易失保存与上电恢复
- [ ] 与 DummyRobot 主控完整抓取联调
- [ ] 结构、装配与量产最终方案

## 目录结构

```text
FlexGrip/
├─ 1.Model/        SolidWorks 结构模型（自制件、电机、URDF 导出）
├─ 2.Hardware/     PCB Gerber 与生产文件
├─ 3.Firmware/     ESP32 固件工程与 CAN 例程
├─ 4.Images/       电源与 PCB 图片
├─ 5.Documents/    原理图 PDF 与网表
├─ 6.Software/     ROS/URDF + Gazebo 仿真包
└─ README.md       项目入口文档
```

主要入口：

- 固件：`3.Firmware/FlexGrip_ESP32_fw/`
- CAN 例程：`3.Firmware/ESP32-CAN/`
- 仿真包：`6.Software/FlexGripper/`
- 原理图：`5.Documents/SCH_Schematic1_2026-10-31.pdf`

## 开发环境版本

下表为已在本项目完成构建/烧录验证的版本，更新版本需重新验证。

| 组件 | 已验证版本或配置 |
| --- | --- |
| 操作系统 | Windows x64 |
| PlatformIO Core | `C:\Users\24302\.platformio\penv\Scripts\platformio.exe` |
| PlatformIO 环境 | `pico32` |
| PlatformIO 平台 | Espressif 32 3.5.0（`espressif32 @ ~3.5.0`） |
| 开发板 | ESP32 Pico Kit（ESP32-PICO-D4，4 MB Flash） |
| Arduino-ESP32 | 1.0.6 / `framework-arduinoespressif32` 3.10006.210326 |
| Xtensa 工具链 | 5.2.0（`toolchain-xtensa32` 2.50200.97） |
| esptool.py | 3.1.0（`tool-esptoolpy` 1.30100.210531） |
| 分区表 | `min_spiffs.csv` |
| 串口监视 | 115200 baud |
| 上传 | COM7，921600 baud |

主要本地库：

| 库 | 版本 | 说明 |
| --- | --- | --- |
| ESP32 AnalogWrite | 0.2.0 | `lib_deps` 依赖，用于 DRV8833 的 PWM 驱动 |
| ESP32-CAN | 0.0.1 | `lib/ESP32-CAN` 本地库，用于 CAN 收发 |

`lib/` 下另有 NimBLE，当前夹爪固件未引用，不影响夹爪功能。

## 快速开始

### 编译固件

```powershell
cd .\3.Firmware\FlexGrip_ESP32_fw
& 'C:\Users\24302\.platformio\penv\Scripts\platformio.exe' run
```

成功构建应出现 `[SUCCESS]`，主要产物：

```text
3.Firmware/FlexGrip_ESP32_fw/.pio/build/pico32/firmware.bin
3.Firmware/FlexGrip_ESP32_fw/.pio/build/pico32/firmware.elf
```

### 烧录

```powershell
& 'C:\Users\24302\.platformio\penv\Scripts\platformio.exe' run -t upload --upload-port COM7
```

`platformio.ini` 已默认 `upload_port = COM7`。

### 串口测试

用 115200 baud 打开 COM7，上电会打印版本信息。可用命令：

```text
angle <0~45>          设置开口量（0 = 闭合/夹紧，45 = 打开）
open / close          全开 / 闭合
enable / disable      使能 / 失能
status                打印当前状态
pid <kp> <ki> <kd>    设置位置 PID 增益
ki <left> <right>     设置左右到位积分增益
speed <deg/s>         设置运动速度
ff <left> <right>     设置左右速度前馈
```

## 软件结构

固件按 `HAL / Ctrl / Configs` 分层：

- `HAL`：硬件抽象。`HAL_ADC` 角度/电流采集，`HAL_Motor` H 桥 PWM 驱动，`HAL_Serial` 串口命令解析，`HAL_CAN` CAN 收发，`HAL_Def` 公共类型。
- `Ctrl`：控制算法。`Pid` 为可复用的位置式 PID（开放接口）；`GripControl` 为夹爪位置控制器，实现“匀速轨迹 + 速度前馈 + 速度阻尼 + 到位积分”。
- `Configs`：`Config.h` 集中引脚、运动学、PID 与 CAN 参数，`Version.h` 版本信息。

控制链路：

```text
命令(串口/CAN) → GripControl
   ├─ 目标角度映射（0~45 → 左右电机角度）
   ├─ 匀速轨迹（左右同速度、同时长 → 同步）
   ├─ 速度前馈 + 速度阻尼 + 到位积分 → PWM 占空比
   └─ HAL_Motor → DRV8833 → 直流电机
HAL_ADC ← RDC506018A 角度传感器（位置反馈）
```

## 当前 ESP32 引脚

| 模块 | ESP32 引脚 | 说明 |
| --- | --- | --- |
| 左角度传感器 U5 | GPIO13 | ADC，RDC506018A 模拟量 |
| 右角度传感器 U6 | GPIO15 | ADC，RDC506018A 模拟量 |
| 电流检测输入 | GPIO12 | ADC 电流采样 |
| 电流检测 | GPIO14 | ADC 电流采样 |
| 右电机 AIN1 | GPIO27 | DRV8833 A 通道，HIGH = 右爪夹紧 |
| 右电机 AIN2 | GPIO26 | DRV8833 A 通道 |
| 左电机 BIN1 | GPIO33 | DRV8833 B 通道，HIGH = 左爪打开 |
| 左电机 BIN2 | GPIO25 | DRV8833 B 通道 |
| CAN_R | GPIO4 | CAN 接收 |
| CAN_D | GPIO5 | CAN 发送 |

角度传感器标定：ADC 原始值 0~4095 → 0~3300 mV → 0~333.3°（见 `Config.h` 中 `ANGLE_ADC_*` 宏）。

## 夹爪运动学

| 输入 | 左电机 | 右电机 |
| --- | --- | --- |
| 0（闭合 = 夹紧） | 143° | 149° |
| 45（打开） | 98° | 194° |

输入 0~45 线性插值：左 = 143 − 输入，右 = 149 + 输入。方向约定：`Motor_SetDuty` 正值表示“夹紧”方向，左右电机的方向差异在 `HAL_Motor` 内部处理。

## CAN 协议（兼容 DummyHand）

| 功能 | CAN ID | 数据 |
| --- | --- | --- |
| set_angle | `0x382`（`7<<7` 加 `0x02`） | 前 4 字节小端 float，0~45 |
| set_enable（经 set_max_current） | `0x381`（`7<<7` 加 `0x01`） | 前 4 字节小端 float，>0 使能、=0 失能 |

节点 ID = 7，与 DummyRobot 主控 `DummyHand` 保持一致。

## AI 协作提示词

### 通用任务提示词

```text
请基于当前 FlexGrip 夹爪项目完成：[目标]，并先读取本 README 与
E:\Projects\FlexGrip\3.Firmware\FlexGrip_ESP32_fw\src\Configs\Config.h

本次目标：[分析/设计/修改/验证的内容]
涉及范围：[结构/硬件/固件/控制算法/串口/CAN/仿真]
已知条件：[硬件版本、固件版本、角度标定、控制参数；未知项写“未知”]
预期结果：[可验证的现象或指标]
允许的外部动作：[无/编译/烧录/串口测试/CAN 联调]
版本操作：[不涉及/仅生成提交信息/提交/提交并推送]

请先检查 git status 和未提交改动，保留无关修改；区分已确认事实、合理推断、候选方案和待验证事项。
修改后执行与风险相称的编译或测试，并报告实际命令、结果、改动文件、未验证内容和已知风险。
只有明确授权后，才能烧录硬件、对接机械臂主控、提交或推送 Git。
```

### 固件编译报错提示词

```text
请根据完整的 PlatformIO 编译日志定位第一个真正导致失败的报错，并结合源码修复。

固件目录：
E:\Projects\FlexGrip\3.Firmware\FlexGrip_ESP32_fw

要求：
1. 不修改无关代码，不覆盖现有未提交改动。
2. 不屏蔽源文件或删除功能来掩盖错误。
3. 修复后执行：
   & 'C:\Users\24302\.platformio\penv\Scripts\platformio.exe' run
4. 出现 [SUCCESS]、完成链接与镜像生成才算通过。
5. 报告根因、修改文件、RAM/Flash 占用与仍存在的 warning。
6. 未授权不执行烧录。
```

## 项目文档

| 文档 | 内容 |
| --- | --- |
| [原理图](5.Documents/SCH_Schematic1_2026-10-31.pdf) | 硬件原理图 PDF |
| [原理图网表](5.Documents/Netlist_Schematic1_2026-10-31.tel) | 原理图网表 |
| [PCB 网表](5.Documents/Netlist_PCB1_2026-10-31.tel) | PCB 网表 |
| [PCB Gerber](2.Hardware/Gerber_PCB1_2024-12-30.zip) | PCB 生产文件 |

## 数据与安全边界

- 角度、电流等开发数据仅用于原型验证。
- 夹爪运动存在夹伤风险，测试时保持安全距离，先小角度验证方向。
- 未经明确授权，不烧录硬件、不对接真实机械臂主控、不进行真实抓取。
- CAN 协议按 DummyRobot `DummyHand` 参考实现，不同主控版本需在实物上复核。
- 未接入电流/夹持力闭环前，不要依赖“角度到位”来判断是否夹住物体。

## 已知问题

- 左电机静摩擦明显大于右电机，极限位（尤其 45°）会稳定在目标附近约 1~1.5°，属当前机械特性的极限。
- 夹爪断电时会向打开方向滑移，上电后第一条指令从其自然停住的位置起步。
- CAN 控制已实现并编译通过，尚未与机械臂主控实物联调。
- 编码器零位偏移 `ANGLE_OFFSET_LEFT/RIGHT` 尚未启用，当前直接使用实测角度标定。
- 电流采样为基础实现，尚未接入力/电流闭环。
- DummyRobot 主控 `DummyHand::SetAngle` 目前硬编码限幅到 30，需把其中 `30` 改为 `45` 才能使用完整行程。

## 协作约定

1. 修改前执行 `git status`，保留他人的未提交改动。
2. 电机方向约定：`Motor_SetDuty` 正值 = 夹紧方向，左右差异在 `HAL_Motor` 内部处理。
3. 左右角度映射以 `Config.h` 中的 `GRIP_*_ANGLE` 为准。
4. 只把实际编译/烧录/测试通过的项标记为完成。
5. 未经明确授权不执行烧录、真实 CAN 通信、提交或推送。

## Git 推送（GitHub）

远程：`origin` = `https://github.com/XiaoHai11111/FlexGrip.git`，分支 `master`。

```powershell
git status                  # 先检查改动，保留他人未提交内容
git add -A
git commit -m "feat(grip): <本次改动说明>"
git push origin master
```

commit message模板

```
Feature/Fix:新增功能/修复内容(尽量总结到20字以内)
	修改描述：
		why:(总结至少30字以内)
		how:(总结至少30字以内)
	影响范围：(总结至少50字以内)
```

