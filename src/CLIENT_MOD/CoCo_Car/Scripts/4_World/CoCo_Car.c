// CoCo_Car 4_World 脚本模块
// 这里处理对红隼船动作的重写和事件记录

// 船只点火记录数据结构（按照VehicleSystem_V2格式）
class BoatEngineRecord
{
    string m_Ower;           // 玩家GUID（所有者）
    int m_InGarage;          // 是否在车库中（默认为1）
    int m_Hash;             // 船只哈希ID
    string m_Type;          // 船只类型
    autoptr array<string> m_Slots; // 船只配件列表

    void BoatEngineRecord(string playerGUID, string boatID, string boatType, array<string> attachments)
    {
        m_Ower = playerGUID;
        m_InGarage = 0; // 默认在车库中
        // m_Hash 基于所有者GUID + 船只类型生成（每个玩家每种船只类型一个记录）
        string hashString = playerGUID + "_" + boatType;
        m_Hash = Math.AbsInt(hashString.Hash()); // 使用字符串哈希值作为m_Hash
        m_Type = boatType;
        m_Slots = new array<string>();

        // 复制配件列表
        for (int i = 0; i < attachments.Count(); i++)
        {
            m_Slots.Insert(attachments[i]);
        }
    }
};

// CoCo_Car RPC管理器
class CoCoCarRPCManager
{
    void CoCoCarRPCManager()
    {
        // 注册服务器端RPC
        if (GetGame().IsServer())
        {
            GetRPCManager().AddRPC("CoCo_Car", "GetPlayerVehiclesRPC", this, SingleplayerExecutionType.Server);
        }
    }

    // 服务器端RPC处理函数 - 获取玩家载具列表
    void GetPlayerVehiclesRPC(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type == CallType.Server)
        {
            Param1<string> data;
            if (!ctx.Read(data))
            {
                Print("[CoCo_Car] - 无法读取RPC参数");
                return;
            }

            string playerGUID = data.param1;
            Print("[CoCo_Car] - 服务器收到RPC请求，玩家: " + playerGUID);

            // 读取PlayerData
            string playerFilePath = "$profile:VehicleSystem_V2/PlayerData/" + playerGUID + ".json";
            array<int> ownedHashes = new array<int>();

            if (FileExist(playerFilePath))
            {
                FileHandle playerFile = OpenFile(playerFilePath, FileMode.READ);
                if (playerFile)
                {
                    string fileContent = "";
                    string line;
                    while (FGets(playerFile, line) > 0)
                    {
                        fileContent += line;
                    }
                    CloseFile(playerFile);

                    // 解析哈希列表
                    int pos = 0;
                    while (pos < fileContent.Length())
                    {
                        // 跳过非数字字符
                        while (pos < fileContent.Length() && !(fileContent[pos] >= "0" && fileContent[pos] <= "9"))
                        {
                            pos++;
                        }

                        if (pos >= fileContent.Length())
                            break;

                        // 提取数字
                        int start = pos;
                        while (pos < fileContent.Length() && (fileContent[pos] >= "0" && fileContent[pos] <= "9"))
                        {
                            pos++;
                        }

                        if (start < pos)
                        {
                            string numStr = fileContent.Substring(start, pos - start);
                            int hashValue = numStr.ToInt();
                            if (hashValue > 0)
                            {
                                ownedHashes.Insert(hashValue);
                            }
                        }
                    }

                    Print("[CoCo_Car] - 服务器解析到 " + ownedHashes.Count() + " 个载具哈希");
                }
            }

            // 发送响应回客户端
            Param1<array<int>> response = new Param1<array<int>>(ownedHashes);
            GetRPCManager().SendRPC("CoCo_Car", "GetPlayerVehiclesResponse", response, true, sender);
        }
    }
}

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

    // 保存到JSON记录文件（严格按照VehicleSystem_V2格式）
    if (player.GetIdentity())
    {
        string playerGUID = player.GetIdentity().GetPlainId();

        // 将配件字符串转换为数组
        array<string> attachmentArray = new array<string>();
        if (attachmentsInfo != "无")
        {
            attachmentsInfo.Split(",", attachmentArray);
            // 清理数组中的空格
            for (int i = 0; i < attachmentArray.Count(); i++)
            {
                attachmentArray[i] = attachmentArray[i].Trim();
            }
        }

        SaveBoatEngineRecord(playerGUID, boatID, boatType, attachmentArray);
    }
}

// 保存船只点火记录到JSON文件（严格按照VehicleSystem_V2格式）
static void SaveBoatEngineRecord(string playerGUID, string boatID, string boatType, array<string> attachments)
{
    // 创建记录数据
    BoatEngineRecord record = new BoatEngineRecord(playerGUID, boatID, boatType, attachments);

    // 记录文件名基于玩家GUID+类型哈希（每个玩家每种类型一个文件）
    string playerTypeHash = Math.AbsInt((playerGUID + "_" + boatType).Hash()).ToString();
    string recordFileName = playerTypeHash + ".json";
    string recordFilePath = "$profile:VehicleSystem_V2/VehicleData/" + recordFileName;

    // 同时更新玩家的所有权记录
    UpdatePlayerOwnershipRecord(playerGUID, record.m_Hash, boatType);

    // 保存到文件（严格按照VehicleSystem_V2的VehicleData格式）
    FileHandle writeFile = OpenFile(recordFilePath, FileMode.WRITE);
    if (writeFile)
    {
        // 写入JSON格式的记录（完全按照VehicleSystem_V2的VehicleData格式）
        FPrintln(writeFile, "{");
        FPrintln(writeFile, "    \"m_Ower\": \"" + record.m_Ower + "\",");
        FPrintln(writeFile, "    \"m_InGarage\": " + record.m_InGarage + ",");
        FPrintln(writeFile, "    \"m_Hash\": " + record.m_Hash + ",");
        FPrintln(writeFile, "    \"m_Type\": \"" + record.m_Type + "\",");
        FPrintln(writeFile, "    \"m_Slots\": [");

        // 写入配件列表
        for (int i = 0; i < record.m_Slots.Count(); i++)
        {
            string comma = "";
            if (i < record.m_Slots.Count() - 1)
            {
                comma = ",";
            }
            FPrintln(writeFile, "        \"" + record.m_Slots[i] + "\"" + comma);
        }

        FPrintln(writeFile, "    ]");
        FPrintln(writeFile, "}");

        CloseFile(writeFile);
        Print("[CoCo_Car] - 船只点火记录已保存到: " + recordFilePath);
    }
    else
    {
        Print("[CoCo_Car] - 错误: 无法保存船只点火记录");
    }
}

// 更新玩家的所有权记录（按照VehicleSystem_V2的PlayerData格式）
static void UpdatePlayerOwnershipRecord(string playerGUID, int boatHash, string boatType)
{
    // 基于玩家GUID和船只类型检查是否已经记录过
    string playerTypeHash = Math.AbsInt((playerGUID + "_" + boatType).Hash()).ToString();
    string vehicleFileName = playerTypeHash + ".json";
    string vehicleFilePath = "$profile:VehicleSystem_V2/VehicleData/" + vehicleFileName;

    // 如果该玩家已经拥有过这种类型的船只，直接跳过（车库刷出的船只自动绑定）
    if (FileExist(vehicleFilePath))
    {
        Print("[CoCo_Car] - 玩家 " + playerGUID + " 已经拥有 " + boatType + "，跳过重复记录");
        return;
    }

    // 只有玩家第一次启动某种类型的船只时才创建记录
    // 注意：这意味着其他玩家如果启动你的车库船，可能会获得绑定
    // 这是一个已知限制，建议保护好车库船只

    // 玩家记录文件名
    string playerFileName = playerGUID + ".json";
    string playerFilePath = "$profile:VehicleSystem_V2/PlayerData/" + playerFileName;

    // 收集该玩家拥有的所有船只哈希
    array<int> ownedHashes = new array<int>();

    // 首先尝试读取现有的PlayerData文件
    if (FileExist(playerFilePath))
    {
        // 读取现有文件内容并解析哈希列表
        FileHandle readFile = OpenFile(playerFilePath, FileMode.READ);
        if (readFile)
        {
            string line;
            string fileContent = "";
            while (FGets(readFile, line) > 0)
            {
                fileContent += line;
            }
            CloseFile(readFile);

            // 简单的字符串解析来提取哈希列表
            int listStart = fileContent.IndexOf("\"m_OwerHashList\": [");
            if (listStart != -1)
            {
                // 从listStart位置开始查找结束括号
                string remainingContent = fileContent.Substring(listStart, fileContent.Length() - listStart);
                int bracketEnd = remainingContent.IndexOf("]");
                int listEnd = -1;
                if (bracketEnd != -1)
                {
                    listEnd = listStart + bracketEnd;
                }

                if (listEnd != -1)
                {
                    string hashListString = fileContent.Substring(listStart + 19, listEnd - (listStart + 19));
                    array<string> hashStrings = new array<string>();
                    hashListString.Split(",", hashStrings);
                    foreach (string hashStr : hashStrings)
                    {
                        hashStr.Trim();
                        if (hashStr != "" && hashStr != "null")
                        {
                            int hashValue = hashStr.ToInt();
                            if (hashValue > 0)
                            {
                                ownedHashes.Insert(hashValue);
                            }
                        }
                    }
                }
            }
        }
    }

    // 如果没有现有记录，尝试从VehicleData中扫描该玩家的船只
    if (ownedHashes.Count() == 0)
    {
        // 这里应该扫描VehicleData文件夹，但由于脚本限制，我们暂时只添加当前船只
        Print("[CoCo_Car] - 玩家 " + playerGUID + " 的PlayerData为空，将从VehicleData重建记录");
    }

    // 确保新船只的哈希被包含
    bool hashExists = false;
    for (int j = 0; j < ownedHashes.Count(); j++)
    {
        if (ownedHashes[j] == boatHash)
        {
            hashExists = true;
            break;
        }
    }

    if (!hashExists)
    {
        ownedHashes.Insert(boatHash);
        Print("[CoCo_Car] - 为玩家 " + playerGUID + " 添加新船只哈希: " + boatHash);
    }

    // 保存玩家记录（按照VehicleSystem_V2的PlayerData格式）
    FileHandle playerFile = OpenFile(playerFilePath, FileMode.WRITE);
    if (playerFile)
    {
        FPrintln(playerFile, "{");
        FPrintln(playerFile, "    \"m_Guid\": \"" + playerGUID + "\",");
        FPrintln(playerFile, "    \"m_Find\": " + ownedHashes.Count() + ",");  // 使用船只数量作为发现数量
        FPrintln(playerFile, "    \"m_Spawn\": 1,"); // 这里简化
        FPrintln(playerFile, "    \"m_OwerHashList\": [");

        // 写入拥有的船只哈希列表
        for (int k = 0; k < ownedHashes.Count(); k++)
        {
            string comma = "";
            if (k < ownedHashes.Count() - 1)
            {
                comma = ",";
            }
            FPrintln(playerFile, "        " + ownedHashes[k] + comma);
        }

        FPrintln(playerFile, "    ]");
        FPrintln(playerFile, "}");

        CloseFile(playerFile);
        Print("[CoCo_Car] - 玩家所有权记录已更新: " + playerFilePath + " (共" + ownedHashes.Count() + "艘船只)");
    }
    else
    {
        Print("[CoCo_Car] - 错误: 无法更新玩家所有权记录");
    }
}

// 获取船只配件的函数
static string GetBoatAttachments(RFWC_base boat)
{
    if (!boat)
        return "无";

    array<string> attachments = new array<string>();

    // 自动遍历所有插槽，获取已安装的配件
    GameInventory inventory = boat.GetInventory();
    if (inventory)
    {
        // 获取所有附件
        array<EntityAI> allAttachments = new array<EntityAI>();
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
    array<string> uniqueAttachments = new array<string>();
    foreach (string attachmentType : attachments)
    {
        if (uniqueAttachments.Find(attachmentType) == -1)
        {
            uniqueAttachments.Insert(attachmentType);
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


// 重写红隼船的启动动作，在服务器端捕获点火事件
modded class ActionStartBoat
{
    override void OnStartServer(ActionData action_data)
    {
        Print("[CoCo_Car] - ActionStartBoat triggered");
        super.OnStartServer(action_data);

        // 在服务器端捕获点火事件
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
                        // 直接记录事件
                        LogBoatEngineStart(player, RFWC);
                    }
                }
            }
        }
    }
};

