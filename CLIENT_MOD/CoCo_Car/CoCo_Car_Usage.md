# CoCo_Car 红隼船点火记录系统使用说明

## 概述

CoCo_Car mod 是一个用于记录红隼船（Red Falcon Water Craft）点火事件的系统。当玩家启动船只引擎时，该系统会自动在服务器日志中记录船只信息。

## 工作原理

### 红隼船点火控制流程

1. **动作触发**: 当玩家坐在船只驾驶员位置时，可以执行 `ActionStartBoat` 动作
2. **条件检查**: 系统检查以下启动条件：
   - 船只未被搁浅 (`!RFWC.c_grounded`)
   - 引擎未损坏 (`RFWC.GetHealthLevel("Engine") < GameConstants.STATE_RUINED`)
   - 玩家在驾驶员座位 (`RFWC.CrewMemberIndex(player) == DayZPlayerConstants.VEHICLESEAT_DRIVER`)
   - 电瓶正常 (电量 > 5.0, 未损坏)
   - 火花塞正常 (未损坏)
   - 有燃油 (`RFWC.GetFluidFraction(CarFluid.FUEL) > 0`)

3. **引擎启动**: 满足条件后执行以下步骤：
   ```cpp
   RFWC.m_engine_on = true;           // 设置引擎状态
   RFWC.SetEngineStarted(true);       // 调用启动方法
   ```

4. **事件记录**: 我们的重写 `SetEngineStarted` 方法捕获启动事件并记录日志

### 记录内容

当船只引擎启动时，服务器日志会显示：
```
[CoCo_Car] - 玩家点火事件: 玩家名称 启动了 船只类型 (ID: 船只ID) | 配件: 配件列表
```

**示例输出:**
```
[CoCo_Car] - 玩家点火事件: 张三 启动了 RFWC_Zodiac (ID: 1_2_3_4) | 配件: TruckBattery, SparkPlug, HatchbackWheel
[CoCo_Car] - 船只点火记录已保存到: $profile:VehicleSystem_V2/VehicleData/433206181.json
[CoCo_Car] - 玩家所有权记录已更新: $profile:VehicleSystem_V2/PlayerData/76561198000000000.json
```

## JSON记录文件

系统会自动在 `VehicleSystem_V2` 文件夹中创建记录，完全按照**VehicleSystem_V2的格式**：

- **船只记录文件**: `VehicleSystem_V2/VehicleData/{哈希ID}.json` (如: `433206181.json`) - 按照VehicleData格式
- **玩家记录文件**: `VehicleSystem_V2/PlayerData/{玩家GUID}.json` (如: `76561198000000000.json`) - 按照PlayerData格式

```json
{
    "m_Ower": "76561198000000000",
    "m_InGarage": 1,
    "m_Hash": 1234,
    "m_Type": "RFWC_Zodiac",
    "m_Slots": [
        "TruckBattery",
        "SparkPlug",
        "HatchbackWheel"
    ]
}
```

同时为每位玩家创建 **PlayerData格式** 的所有权记录，包含所有该玩家拥有的船只哈希：

```json
{
    "m_Guid": "76561199064072006",
    "m_Find": 5,
    "m_Spawn": 1,
    "m_OwerHashList": [
        433206181,
        1169795898,
        967586429,
        1959030797,
        1479437815,
        1680081518
    ]
}
```

## 数据结构

### BoatEngineRecord 类

按照VehicleSystem_V2格式设计的数据结构：

```cpp
// 船只点火记录数据结构（按照VehicleSystem_V2格式）
class BoatEngineRecord
{
    string m_Ower;           // 玩家GUID（所有者）
    int m_InGarage;          // 是否在车库中（这里表示是否记录中）
    int m_Hash;             // 船只哈希ID
    string m_Type;          // 船只类型
    string m_StartTime;     // 启动时间
    vector m_StartPosition; // 启动位置
    autoptr array<string> m_Slots; // 船只配件列表

    void BoatEngineRecord(string playerGUID, string boatID, string boatType, array<string> attachments)
    {
        m_Ower = playerGUID;
        m_InGarage = 1; // 默认在车库中
        m_Hash = Math.AbsInt(boatID.ToInt()); // 将字符串ID转换为整数哈希，使用绝对值
        m_Type = boatType;
        m_Slots = new array<string>;

        // 复制配件列表
        for (int i = 0; i < attachments.Count(); i++)
        {
            m_Slots.Insert(attachments[i]);
        }
    }
};
```

## 技术说明

### DayZ脚本限制

**注意**: DayZ Enforce脚本引擎不支持三元运算符 (`?:`)，因此代码中使用传统的if-else语句进行条件判断，避免语法错误。

## 技术实现

### 监听机制

CoCo_Car mod 通过重写红隼船的 `ActionStartBoat` 动作来监听点火事件：

1. **动作拦截**: 重写 `ActionStartBoat.OnStartServer()` 方法
2. **事件捕获**: 在动作执行时获取玩家和船只信息
3. **信息记录**: 调用 `CoCo_Car.OnBoatEngineStarted()` 记录详细信息

### 修改的文件

#### 1. CoCo_Car mod: `Scripts/4_World/CoCo_Car.c`

在4_World脚本模块中包含所有逻辑：

```cpp
// 船只点火事件记录函数
static void LogBoatEngineStart(PlayerBase player, RFWC_base boat)
{
    if (!player || !boat)
        return;

    // 获取玩家信息
    string playerName = "Unknown";
    if (player.GetIdentity())
    {
        playerName = player.GetIdentity().GetName();
    }

    string boatType = boat.GetType();

    // 获取船只ID
    string boatID = GetBoatID(boat);

    // 获取船只配件信息
    string attachmentsInfo = GetBoatAttachments(boat);

    Print("[CoCo_Car] - 玩家点火事件: " + playerName + " 启动了 " + boatType + " (ID: " + boatID + ") | 配件: " + attachmentsInfo);
}



// 获取船只配件的函数
static string GetBoatAttachments(RFWC_base boat)
{
    if (!boat)
        return "无";

    array<string> attachments = new array<string>;

    // 自动遍历所有插槽，获取已安装的配件
    GameInventory inventory = boat.GetInventory();
    if (inventory)
    {
        // 获取所有附件
        array<EntityAI> allAttachments = new array<EntityAI>;
        inventory.EnumerateInventory(InventoryTraversalType.LEVELORDER, allAttachments);

        foreach (EntityAI attachment : allAttachments)
        {
            if (attachment && attachment != boat) // 排除船只本身
            {
                string itemType = attachment.GetType();
                if (itemType != "" && itemType != "Simulation_Dummy_Item")
                {
                    attachments.Insert(itemType);
                }
            }
        }
    }

    // 去重处理（同一个类型的配件可能有多个）
    array<string> uniqueAttachments = new array<string>;
    foreach (string attachment : attachments)
    {
        if (uniqueAttachments.Find(attachment) == -1)
        {
            uniqueAttachments.Insert(attachment);
        }
    }

    // 格式化配件信息
    if (uniqueAttachments.Count() > 0)
    {
        string result = "";
        for (int j = 0; j < uniqueAttachments.Count(); j++)
        {
            if (j > 0) result += ", ";
            result += uniqueAttachments[j];
        }
        return result;
    }
    else
    {
        return "无";
    }
}

// 获取船只ID的函数
static string GetBoatID(RFWC_base boat)
{
    if (!boat)
        return "N/A";

    // 使用PersistentID获取船只的唯一标识符
    int b1, b2, b3, b4;
    boat.GetPersistentID(b1, b2, b3, b4);

    // 将ID转换为字符串格式
    string persistentID = b1.ToString() + "_" + b2.ToString() + "_" + b3.ToString() + "_" + b4.ToString();

    // 如果PersistentID都为0，则使用备用方法
    if (persistentID == "0_0_0_0")
    {
        persistentID = boat.GetID().ToString();
    }

    return persistentID;
}

// 重写红隼船的启动动作
modded class ActionStartBoat
{
    override void OnStartServer(ActionData action_data)
    {
        super.OnStartServer(action_data);

        // 在服务器端捕获点火事件并直接记录
        if (GetGame().IsServer())
        {
            PlayerBase player = action_data.m_Player;
            HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

            if (vehCommand)
            {
                Transport trans = vehCommand.GetTransport();
                if (trans)
                {
                    RFWC_base RFWC;
                    if (Class.CastTo(RFWC, trans))
                    {
                        LogBoatEngineStart(player, RFWC);
                    }
                }
            }
        }
    }
}
```

#### 2. CoCo_Car mod: `Scripts/5_Mission/CoCo_Car.c`

Mission类只负责生命周期管理：

```cpp
class CoCo_Car : MissionBase
{
    void CoCo_Car()
    {
        Print("[CoCo_Car] - Initializing CoCo_Car mod (Event-driven)");
    }

    // 只负责基本的Mission生命周期管理
    override void OnMissionStart()
    {
        super.OnMissionStart();
        Print("[CoCo_Car] - Mission started, CoCo_Car mod active");
    }
}
```

### 优势

1. **非侵入式**: 不需要修改红隼船的源代码
2. **实时检测**: 通过动作系统实时捕获点火事件
3. **完整信息**: 记录玩家和船只的详细信息
4. **高兼容性**: 与任何版本的红隼船兼容

## 安装和使用

1. **安装mod**: 将 `CoCo_Car` 文件夹放入服务器的 `@mods` 目录
2. **确保红隼船**: 服务器需要安装红隼船mod (Red Falcon Water Craft)
3. **启动服务器**: mod会自动激活并开始监听船只点火事件
4. **查看日志**: 船只点火事件会自动记录到服务器控制台和日志文件中

## 故障排除

### 常见问题

1. **看不到日志输出**
   - 确认mod已正确加载到服务器
   - 检查服务器控制台和日志文件
   - 确认玩家确实执行了船只启动动作

2. **ID显示为0_0_0_0**
   - 这是正常的，有些船只可能没有有效的PersistentID
   - 系统会自动使用备用ID方法

3. **条件检查失败**
   - 确认船只满足所有启动条件（电瓶、火花塞、燃油）
   - 确认玩家在驾驶员位置
   - 确认船只没有搁浅或损坏

4. **mod冲突**
   - 确保没有其他mod也在重写 `ActionStartBoat` 类
   - 如果有冲突，可能需要调整mod加载顺序

## 自动配件检测

系统会自动遍历船只的所有插槽和附件，检测并记录所有已安装的配件：
- **自动识别**: 无需预定义配件类型列表，系统会自动发现所有附件
- **实时获取**: 动态获取船只当前的所有附件
- **去重处理**: 相同类型的多个配件会被合并显示
- **全面覆盖**: 包括所有可能的船只配件和附件

## 扩展功能

如需添加更多功能，可以在 `SetEngineStarted` 方法中扩展：

- 记录驾驶员信息
- 保存到数据库
- 发送Discord通知
- 触发其他服务器事件

---

**最后更新**: 2026年1月5日 (添加船只管理界面)
