# CoCo_Car - DayZ载具管理系统

## 简介

CoCo_Car是一个DayZ模组，为玩家提供先进的载具绑定和管理功能。通过VehicleSystem_V2系统，实现玩家与载具的持久绑定关系。

## 主要功能

### 🚗 载具绑定系统
- 玩家启动载具时自动绑定
- 支持多种载具类型（船只、汽车等）
- 绑定信息持久保存到服务器

### 🎮 管理界面
- 按分号键(;)打开管理界面
- 显示已绑定的载具列表
- 提供操作指南和说明

### 🔧 系统集成
- 与VehicleSystem_V2完美集成
- 支持服务器端数据存储
- 客户端/服务器端通信

## 支持的载具类型

### 水上载具
- RFWC_DragBoat_Black (黑色拖船)
- RFWC_Zodiac (救生艇)

### 地面载具
- CivilSedan (民用轿车)
- Hatchback_02 (掀背车)
- Sedan_02 (轿车)
- Offroad_02 (越野车)
- Truck_01_Covered (卡车)

## 安装方法

1. 将`CoCo_Car`文件夹复制到你的DayZ服务器的`@CLIENT_MOD`目录
2. 确保服务器已安装VehicleSystem_V2
3. 重启服务器

## 使用方法

1. 进入游戏，找到并启动载具
2. 系统会自动绑定载具到你的账户
3. 按分号键(;)打开管理界面查看已绑定的载具

## 技术实现

### 客户端脚本 (5_Mission)
- `CoCo_Car.c` - 主要界面和逻辑
- `layouts/CoCoCarMenu.layout` - 界面布局文件

### 服务器端脚本 (4_World)
- `CoCo_Car.c` - 载具绑定和数据存储

### 配置文件
- `config.cpp` - 模组配置
- `Scripts/data/inputs.xml` - 输入绑定

## 开发说明

- 使用DayZ脚本语言开发
- 支持VehicleSystem_V2数据格式
- 客户端/服务器端分离架构

## 版本历史

### v1.0.0
- 初始版本发布
- 基础载具绑定功能
- 管理界面

## 作者

deqiang233

## 许可证

本项目仅供学习和研究使用，请遵守DayZ模组开发规范。
