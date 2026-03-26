# E603 SoC 修改与编译说明

## 1. SoC功能修改说明

### 1.1 修改概述

本版本E603 SoC相较于官方预编译的MCS和BIT文件，主要进行了以下功能修改：

1. **LED输出功能重构**：重新配置LED0~LED7的硬件功能
2. **开关输入功能增加**：新增SW0~SW7作为GPIO输入
3. **GPIO模块扩展**：增加GPIO模块以支持总线通信和通用输入输出功能

### 1.2 详细修改内容

#### 1.2.1 LED功能修改

**原始功能配置：**
- LED0：显示`wfi_mode`状态（亮起表示处理器进入WFI模式）
- LED1：显示`cpu_sleep`状态（亮起表示CPU进入睡眠模式）
- LED7：显示MCU就绪状态（MCU准备完毕后自动亮起）

**修改后配置：**
上述LED功能已全部禁用，LED0~LED7现均配置为GPIO输出端口。

#### 1.2.2 开关输入功能

新增SW0~SW7作为GPIO输入端口，用于外部信号采集。

#### 1.2.3 GPIO寄存器映射

GPIO模块基地址：`0x10010000`

| 寄存器名称 | 偏移地址 | 功能描述 |
|------------|----------|----------|
| LED输出寄存器 | `0x0000` | 控制LED0~LED7输出状态 |
| SW输入寄存器 | `0x0004` | 读取SW0~SW7输入状态 |

### 1.3 使用示例

参考示例程序位于：`application/baremetal/demo_gpio`

测试步骤请参照[实验手册.md](../实验手册.md)中实验一的相关说明。

---

## 2. 编译环境与步骤

### 2.1 环境要求

#### 2.1.1 Vivado版本兼容性

**官方版本限制：**
- 原始E603 SoC设计对`mig_7series` IP核的版本限制为4.2
- 建议使用Vivado 2019及以上版本进行编译

**本版本修改：**
为适配实验室环境（Vivado 2018.1），已移除IP核版本限制。

**注意：** 编译报告显示DDR3存在部分时序冲突警告，但实际测试暂未发现功能异常。如遇稳定性问题，建议升级至Vivado 2019或更高版本。

#### 2.1.2 版本恢复方法

如需恢复原始版本限制，请修改文件：
`e603_hbird-main/fpga/boards/ddr200t/script/ip.tcl`

将以下代码段：
```tcl
if ($env(DDR3_EN)) {
    create_ip -vendor xilinx.com -library ip -name mig_7series -module_name ddr3 -dir $ipdir -force
    set_property CONFIG.XML_INPUT_FILE [file normalize $scriptdir/mig_a.prj] [get_ips ddr3]
}
```

修改为：
```tcl
if ($env(DDR3_EN)) {
    create_ip -vendor xilinx.com -library ip -name mig_7series -version 4.2 -module_name ddr3 -dir $ipdir -force
    set_property CONFIG.XML_INPUT_FILE [file normalize $scriptdir/mig_a.prj] [get_ips ddr3]
}
```

### 2.2 编译步骤

#### 2.2.1 Windows系统

1. **环境配置**
   - 确认系统已安装Vivado工具
   - 打开命令提示符，执行环境配置脚本：
     ```bash
     path/to/Xilinx/Vivado/2017.4/settings64.bat
     ```

2. **编译过程**
   ```bash
   # 进入项目目录
   cd e603_hbird-main/fpga
   
   # 清理并重新编译
   make clean && make mcs
   ```

3. **输出文件**
   - 编译完成后，MCS文件位于：
     `e603_hbird-main/fpga/gen/ddr200t/obj/`
   - 预计编译耗时：约20分钟

#### 2.2.2 Linux系统

在终端中直接执行上述编译命令即可。

---

## 3. 注意事项

### 3.1 硬件注意事项

1. **DDR3时序**：如使用Vivado 2018.1编译，需关注DDR3时序冲突警告
2. **GPIO使用**：修改后的GPIO功能需通过示例程序验证
3. **电源要求**：确保开发板供电稳定

### 3.2 软件注意事项

1. **路径规范**：项目路径不应包含中文字符
2. **版本兼容**：建议使用Vivado 2019及以上版本以获得最佳兼容性
3. **环境变量**：确保Vivado环境变量正确配置

### 3.3 测试建议

1. **功能验证**：首次使用前，建议运行GPIO示例程序验证修改功能
2. **稳定性测试**：进行长时间运行测试以确保系统稳定性
3. **回滚方案**：保留原始BIT文件以备需要时恢复

---

## 4. 相关文档

1. [实验手册.md](../实验手册.md) - 详细实验步骤和操作指南
2. [README.md](../README.md) - 项目概述和基本说明
3. [Nuclei_E603_Databook.pdf](./doc/Nuclei_E603_Databook.pdf) - E603处理器数据手册
4. [Nuclei_E603_Integration_Guide.pdf](./doc/Nuclei_E603_Integration_Guide.pdf) - E603集成指南

---