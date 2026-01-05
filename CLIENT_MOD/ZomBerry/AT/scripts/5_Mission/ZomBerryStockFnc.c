class ZomberryStockFunctions {
	ref ZomberryCmdAPI m_ZomberryCmdAPI;
	autoptr TIntArray m_spectatingList = new TIntArray;
	autoptr TIntStringMap m_deleteList = new TIntStringMap;

	void ZomberryStockFunctions() {
		m_ZomberryCmdAPI = GetZomberryCmdAPI();
	}

	void Init() {
		m_ZomberryCmdAPI.AddCategory("OnTarget", 0xFF42AAFF);
		m_ZomberryCmdAPI.AddCommand("传送 - 传送到光标位置", "TPCur", this, "OnTarget", false);
		m_ZomberryCmdAPI.AddCommand("传送 - 我前面10米", "TPForward", this, "OnTarget", false);
		m_ZomberryCmdAPI.AddCommand("传送 - 我到对方", "TPToTarget", this, "OnTarget");
		m_ZomberryCmdAPI.AddCommand("传送 - 对方到我", "TPToAdmin", this, "OnTarget");
		m_ZomberryCmdAPI.AddCommand("治疗 - 痊愈", "HealTarget", this, "OnTarget", false);
		m_ZomberryCmdAPI.AddCommand("治疗 - 部分治疗", "HealTargetPart", this, "OnTarget", false, {
			new ZBerryFuncParam("治愈疾病", {0, 1, 1,}),
			new ZBerryFuncParam("伤口愈合", {0, 1, 1,}),
			new ZBerryFuncParam("吃饱", {0, 1, 1,}),
		});
		m_ZomberryCmdAPI.AddCommand("上帝模式", "GodTarget", this, "OnTarget", false);
		m_ZomberryCmdAPI.AddCommand("维修手上物品", "RepairTargetHands", this, "OnTarget", false);
		m_ZomberryCmdAPI.AddCommand("加油和维修车辆", "RefuelAndRepair", this, "OnTarget", false);

		m_ZomberryCmdAPI.AddCategory("==", 0xFFFF7C75);
		m_ZomberryCmdAPI.AddCommand("杀死", "KillTarget", this, "==");
		m_ZomberryCmdAPI.AddCommand("咬伤", "BiteTarget", this, "==", true, {
			new ZBerryFuncParam("咬伤", {1, 25, 1,}),
		});
		m_ZomberryCmdAPI.AddCommand("剥光", "StripTarget", this, "==");
		m_ZomberryCmdAPI.AddCommand("复制 - 复制目标的全部物品", "CopyAllFromTarget", this, "==", true);
		m_ZomberryCmdAPI.AddCommand("呕吐", "RejectBellyTarget", this, "==", true, {
			new ZBerryFuncParam("秒数", {5, 30, 5,}),
		});
		m_ZomberryCmdAPI.AddCommand("发笑", "PsycoTarget", this, "==");
		m_ZomberryCmdAPI.AddCommand("喷嚏", "SneezeTarget", this, "==");
		m_ZomberryCmdAPI.AddCommand("设置状态", "SetHealthTarget", this, "==", true, {
			new ZBerryFuncParam("健康", {1, 100, 100,}),
		});
		m_ZomberryCmdAPI.AddCommand("血量", "SetBloodTarget", this, "==", true, {
			new ZBerryFuncParam("血量", {1, 5000, 5000,}),
		});

		m_ZomberryCmdAPI.AddCategory("OnServer", 0xFF909090);
		m_ZomberryCmdAPI.AddCommand("自由视角", "FreeCamAdm", this, "OnServer", false);
		m_ZomberryCmdAPI.AddCommand("白天", "TimeDay", this, "OnServer", false);
		m_ZomberryCmdAPI.AddCommand("夜晚", "TimeNight", this, "OnServer", false);
		m_ZomberryCmdAPI.AddCommand("时间", "SetTime", this, "OnServer", false, {
			new ZBerryFuncParam("时", {0, 23, 12,}),
			new ZBerryFuncParam("分", {0, 59, 0,}),
		});
		m_ZomberryCmdAPI.AddCommand("日期", "SetDate", this, "OnServer", false, {
			new ZBerryFuncParam("天", {1, 30, 1,}),
			new ZBerryFuncParam("月", {1, 12, 1,}),
			new ZBerryFuncParam("年", {1970, 2119, 2019,}),
		});
		m_ZomberryCmdAPI.AddCommand("天气", "SetWeather", this, "OnServer", false, {
			new ZBerryFuncParam("雾", {0, 100, 0,}),
			new ZBerryFuncParam("云", {0, 100, 0,}),
			new ZBerryFuncParam("雨", {0, 100, 0,}),
		});
		m_ZomberryCmdAPI.AddCommand("删除光标附近的对象", "DeleteObj", this, "OnServer", false);
	}

	static void MessagePlayer(PlayerBase player, string msg) {
		Param1<string> param = new Param1<string>( "[ZomBerry]: " + msg );
		if (GetGame().IsMultiplayer()) {
			GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, param, true, player.GetIdentity());
		} else {
			GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCAdmin, "[ZomBerry]", msg, ""));
		}
	}

	static vector GetPosSafe(PlayerBase target) {
		if (!target.GetCommand_Vehicle()) {
			return target.GetPosition();
		} else {
			return target.GetCommand_Vehicle().GetTransport().GetPosition();
		}
	}

	static bool SetPosSafe(PlayerBase target, vector position) {
		if (!target.GetCommand_Vehicle()) {
			target.SetPosition(position);
			return true;
		}
		return false;
	}

	void TPForward( string funcName, int adminId, int targetId, vector cursor ) {
		const int tpDist = 10;
		vector currentPos, curToTgtDir, targetPos;
		PlayerBase admin = ZBGetPlayerById(adminId);

		currentPos = GetPosSafe(admin);
		currentPos[1] = 0;
		cursor[1] = 0;

		curToTgtDir = vector.Direction(currentPos, cursor);
		curToTgtDir.Normalize();

		targetPos[0] = curToTgtDir[0]*tpDist;
		targetPos[2] = curToTgtDir[2]*tpDist;
		targetPos = targetPos + currentPos;

		targetPos[1] = GetGame().SurfaceY(targetPos[0], targetPos[2]);

		if (!SetPosSafe(admin, targetPos)) {
			MessagePlayer(admin, "无法传送：车辆中的管理员");
		}
	}

	void TPCur( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (SetPosSafe(target, cursor)) {
			MessagePlayer(admin, "传送目标");
		} else {
			MessagePlayer(admin, "无法传送：车辆中的目标");
		}
	}

	void TPToTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (SetPosSafe(admin, GetPosSafe(target))) {
			MessagePlayer(admin, "传送到目标");
		} else {
			MessagePlayer(admin, "无法传送：下车！");
		}
	}

	void TPToAdmin( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (SetPosSafe(target, GetPosSafe(admin))) {
			MessagePlayer(admin, "传送目标");
		} else {
			MessagePlayer(admin, "无法传送：车辆中的目标");
		}
	}

	void HealTarget( string funcName, int adminId, int targetId, vector cursor ) {
		HealTargetPart(funcName, adminId, targetId, cursor, {1, 1, 1});
	}

	void HealTargetPart( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		string result_message = target.GetIdentity().GetName() + " 已经 ";

		if (fValues[0]) {
			bool is_area_exposure, is_mask;

			target.GetStomach().ClearContents();
			
			if (target.GetModifiersManager())
			{
				is_mask = target.GetModifiersManager().IsModifierActive(eModifiers.MDF_MASK);
				is_area_exposure = target.GetModifiersManager().IsModifierActive(eModifiers.MDF_AREAEXPOSURE);
				target.GetModifiersManager().DeactivateAllModifiers();
			}

			if (is_area_exposure)
				target.GetModifiersManager().ActivateModifier(eModifiers.MDF_AREAEXPOSURE);
			if (is_mask)
				target.GetModifiersManager().ActivateModifier(eModifiers.MDF_MASK);

			target.RemoveAllAgents();
			
			result_message += "治愈; ";
		}

		if (fValues[1]) {
			if (target.GetBleedingManagerServer())
				target.GetBleedingManagerServer().RemoveAllSources();

			DamageZoneMap zones = new DamageZoneMap;
			DamageSystem.GetDamageZoneMap(target, zones);
			target.SetHealth("", "Health", target.GetMaxHealth("","Health"));
			target.SetHealth("", "Shock", target.GetMaxHealth("","Shock"));
			target.SetHealth("", "Blood", target.GetMaxHealth("","Blood"));
			
			for (int i = 0; i < zones.Count(); i++)
			{
				string zone = zones.GetKey(i);
				target.SetHealth(zone, "Health", target.GetMaxHealth(zone,"Health"));
				target.SetHealth(zone, "Shock", target.GetMaxHealth(zone,"Shock"));
				target.SetHealth(zone, "Blood", target.GetMaxHealth(zone,"Blood"));
			}

			if(target.IsUnconscious())
				DayZPlayerSyncJunctures.SendPlayerUnconsciousness(target, false);
			
			result_message += "康复; ";
		}

		if (fValues[2]) {
			target.GetStatStamina().Set(target.GetStatStamina().GetMax());
			target.GetStatWater().Set(target.GetStatWater().GetMax());
			target.GetStatEnergy().Set(target.GetStatEnergy().GetMax());
			
			result_message += "补充水分和能量";
		}

		PluginLifespan module_lifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan));
		module_lifespan.UpdateBloodyHandsVisibilityEx(target, eBloodyHandsTypes.CLEAN);

		MessagePlayer(admin, result_message);
	}

	void GodTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (target.ZBIsGod()) {
			target.ZBSetGod(false);
			MessagePlayer(target, "上帝模式已停用");
			if (adminId != targetId) MessagePlayer(admin, "上帝模式已停用：" + target.GetIdentity().GetName());

		} else {
			HealTarget("HealTarget", -1, targetId, "0 0 0");

			target.ZBSetGod(true);
			MessagePlayer(target, "上帝模式已激活");
			if (adminId != targetId) MessagePlayer(admin, "上帝模式已激活：" + target.GetIdentity().GetName());
		}
	}

	void RepairTargetHands( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		InventoryItem target_item = target.GetItemInHands();

		if (!target_item) {
			MessagePlayer(admin, "目标手中没有物品");
			return;
		}

		GameInventory target_item_inv = target_item.GetInventory();
		ref array<EntityAI> sub_items = new array<EntityAI>;

		if (target_item_inv)
			target_item_inv.EnumerateInventory(InventoryTraversalType.INORDER, sub_items);

		target_item.SetHealthMax("","");
		for (int i = 0; i < sub_items.Count(); i++)
		{
			EntityAI item = sub_items.Get(i);
			item.SetHealthMax("","");
		}

		MessagePlayer(admin, "将损坏的物品修复到最佳状态");
	}

	void RefuelAndRepair( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		TStringArray slotsCar = new TStringArray;
		TStringArray cfgVehItemCompatSlots = new TStringArray;
		string cfgVehItem, cfgVehItemLower;

		Car carObject;
		ItemBase currentAttachment;
		ref array<Object> nearestObjects = new array<Object>;
		ref array<CargoBase> proxyCargos = new array<CargoBase>;
		int m, cfgVehId, cfgVehiclesCount;

		vector position = GetPosSafe(target);
		GetGame().GetObjectsAtPosition(position, 15, nearestObjects, proxyCargos);

		foreach(Object nearObject: nearestObjects) {
			if (nearObject.IsKindOf("CarScript")) {
				carObject = Car.Cast(nearObject);

				ZomberryBase.DebugLog(2, "ZomBerryFncDbg", "加油和修理：" + carObject.GetType());

				// Get list of available attachment slots from config
				GetGame().ConfigGetTextArray("CfgVehicles " + carObject.GetType() + " attachments", slotsCar);

				// Get list of ALL items in CfgVehicles, we'll filter compatible ones using slot names later on
				cfgVehiclesCount = GetGame().ConfigGetChildrenCount("CfgVehicles");

				for (cfgVehId = 0; cfgVehId < cfgVehiclesCount; cfgVehId++) {

					// Get item name
					GetGame().ConfigGetChildName("CfgVehicles", cfgVehId, cfgVehItem );
					
					// Make a lower case copy
					cfgVehItemLower = cfgVehItem;
					cfgVehItemLower.ToLower();

					// We don't need ruined, and unfortunately the most common root for all car parts is Inventory_Base
					if (!GetGame().IsKindOf(cfgVehItemLower, "Inventory_Base") || cfgVehItemLower.Contains("ruined")) continue;

					// Get list of slots this item can fit into (cfgVehItemCompatSlots)
					if (GetGame().ConfigGetType("CfgVehicles " + cfgVehItem + " inventorySlot") == CT_ARRAY) {
						GetGame().ConfigGetTextArray("CfgVehicles " + cfgVehItem + " inventorySlot", cfgVehItemCompatSlots);
					} else {
						GetGame().ConfigGetText("CfgVehicles " + cfgVehItem + " inventorySlot", cfgVehItemLower);
						cfgVehItemCompatSlots.Insert(cfgVehItemLower);
					}

					// For each of compatible slots
					for (m = 0; m < cfgVehItemCompatSlots.Count(); m++) {

						// If this slot exists in vehicle
						if (slotsCar.Find(cfgVehItemCompatSlots[m]) != -1) {

							// Cast attachment that's currently in this slot to ItemBase
							Class.CastTo(currentAttachment, carObject.FindAttachmentBySlotName(cfgVehItemCompatSlots[m]));

							// If attachment is present
							if (currentAttachment) {
								
								// Skip if health is good
								if (currentAttachment.GetHealth01() > 0.75) continue;

								// Destroy if health is *not so good*
								ZomberryBase.DebugLog(2, "ZomBerryFncDbg", "RefuelAndRepair: " + currentAttachment.GetType() + " (" + currentAttachment.GetHealth().ToString() + " HP) - detached");
								carObject.GetInventory().DropEntity(InventoryMode.PREDICTIVE, carObject, currentAttachment);
								currentAttachment.SetPosition("0 0 0");
								GetGame().ObjectDelete(currentAttachment);
							}

							// Get slot id from name
							int slotId = InventorySlots.GetSlotIdFromString(cfgVehItemCompatSlots[m]);

							// Add new attachment to vehicle
							carObject.GetInventory().CreateAttachmentEx(cfgVehItem, slotId);
							ZomberryBase.DebugLog(2, "ZomBerryFncDbg", "加油和修理: 已找到并附加 " + cfgVehItem);
						}
					}

					cfgVehItemCompatSlots.Clear();
				}

				carObject.SetHealth("Engine", "", carObject.GetMaxHealth("Engine", ""));

				// Refuel
				float fuelReq = carObject.GetFluidCapacity( CarFluid.FUEL ) - (carObject.GetFluidCapacity( CarFluid.FUEL ) * carObject.GetFluidFraction( CarFluid.FUEL ));
				float oilReq = carObject.GetFluidCapacity( CarFluid.OIL ) - (carObject.GetFluidCapacity( CarFluid.OIL ) * carObject.GetFluidFraction( CarFluid.OIL ));
				float coolantReq = carObject.GetFluidCapacity( CarFluid.COOLANT ) - (carObject.GetFluidCapacity( CarFluid.COOLANT ) * carObject.GetFluidFraction( CarFluid.COOLANT ));
				float brakeReq = carObject.GetFluidCapacity( CarFluid.BRAKE ) - (carObject.GetFluidCapacity( CarFluid.BRAKE ) * carObject.GetFluidFraction( CarFluid.BRAKE ));
				carObject.Fill( CarFluid.FUEL, fuelReq );
				carObject.Fill( CarFluid.OIL, oilReq );
				carObject.Fill( CarFluid.COOLANT, coolantReq );
				carObject.Fill( CarFluid.BRAKE, brakeReq );
			}
		}
		MessagePlayer(ZBGetPlayerById(adminId), "在目标附近为车辆加油和修理");
	}

	void KillTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		target.SetHealth(0);
		MessagePlayer(admin, "杀死目标");
	}

	void BiteTarget( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		BleedingSourcesManagerServer BSMgr = target.GetBleedingManagerServer();

		for (int i = 0; i < fValues[0]; ++i) {
			for (int j = 0; j < 15; ++j) {
				if (BSMgr.AttemptAddBleedingSource(Math.RandomInt(0, 100))) break;
			}
		}
		MessagePlayer(admin, "目标被附加咬伤" + fValues[0] + " 次，现在正在流血");
	}

	void StripTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		target.RemoveAllItems();
		MessagePlayer(admin, "目标被剥光");
	}

	// 复制目标玩家身上的物品到管理员（仅在选择目标时可用）
	void CopyAllFromTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		ref array<EntityAI> allItems = new array<EntityAI>;
		target.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, allItems);

		vector adminPos = GetPosSafe(admin);
		adminPos[1] = GetGame().SurfaceY(adminPos[0], adminPos[2]);

		int placedCount = 0;

		for (int i = 0; i < allItems.Count(); i++) {
			EntityAI src = allItems.Get(i);
			// 只复制顶层物品（直接属于玩家的物品），避免重复复制子项
			if (src.GetHierarchyParent() != target) continue;

			placedCount += CreateCopyOnGround(src, adminPos, placedCount);
		}

		MessagePlayer(admin, "复制并丢在脚下完成: " + placedCount.ToString() + " 件（顶级物品及其子项）");
	}

	// 递归复制 src 的所有直接子项到 dst（保持层级结构）
	void CopyChildrenRecursive(EntityAI src, EntityAI dst) {
		// 保留兼容性（不再用于脚下丢弃实现，但保留以备可用）
		ref array<EntityAI> children = new array<EntityAI>;
		src.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, children);

		for (int j = 0; j < children.Count(); j++) {
			EntityAI child = children.Get(j);
			if (child.GetHierarchyParent() != src) continue;

			EntityAI newChild = dst.GetInventory().CreateInInventory(child.GetType());
			if (!newChild) continue;

			ItemBase childIB = ItemBase.Cast(child);
			ItemBase newChildIB = ItemBase.Cast(newChild);
			if (childIB && newChildIB) {
				newChildIB.SetQuantity(childIB.GetQuantity());
				newChildIB.SetHealth(childIB.GetHealth("", ""));
			} else {
				newChild.SetHealth(child.GetHealth());
			}

			CopyChildrenRecursive(child, newChild);
		}
	}

	// 在指定基准位置附近的地面上创建 src 的副本并返回创建的数量（包含子项）
	int CreateCopyOnGround(EntityAI src, vector basePos, int startIndex) {
		// 使用索引分散物品，避免重叠
		vector pos;
		int row = startIndex / 5;
		int col = startIndex - row * 5; // col = startIndex % 5, implemented without '%'
		int rowMod = row - (row / 5) * 5; // rowMod = row % 5, implemented without '%'

		pos[0] = basePos[0] + ((col) - 2) * 0.35;
		pos[2] = basePos[2] + ((rowMod) - 2) * 0.35;
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);

		EntityAI dst = EntityAI.Cast(GetGame().CreateObject(src.GetType(), pos, false, true));
		int created = 0;
		if (dst) {
			ItemBase srcIB = ItemBase.Cast(src);
			ItemBase dstIB = ItemBase.Cast(dst);
			if (srcIB && dstIB) {
				dstIB.SetQuantity(srcIB.GetQuantity());
				dstIB.SetHealth(srcIB.GetHealth("", ""));
			} else {
				dst.SetHealth(src.GetHealth());
			}
			created = 1;
		}

		// 复制子项为独立地面物品，索引依次递增
		ref array<EntityAI> children = new array<EntityAI>;
		src.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, children);

		int localIndex = startIndex;
		for (int k = 0; k < children.Count(); k++) {
			EntityAI child = children.Get(k);
			if (child.GetHierarchyParent() != src) continue;
			localIndex++;
			created += CreateCopyOnGround(child, basePos, localIndex);
		}

		return created;
	}

	void RejectBellyTarget( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (!target.GetCommand_Vehicle()) {
			SymptomBase symptom = ZBGetPlayerById(targetId).GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_VOMIT);
			symptom.SetDuration(fValues[0]);
			MessagePlayer(admin, "目标会生病" + fValues[0] + " 秒");
		} else {
			MessagePlayer(admin, "目标在车内，此操作可能导致游戏崩溃");
		}
	}

	void PsycoTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (!target.GetCommand_Vehicle()) {
			target.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_LAUGHTER);
			MessagePlayer(admin, "目标在笑...");
		} else {
			MessagePlayer(admin, "目标在车内，此操作可能导致游戏崩溃");
		}
	}

	void SneezeTarget( string funcName, int adminId, int targetId, vector cursor ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		if (!target.GetCommand_Vehicle()) {
			target.GetSymptomManager().QueueUpPrimarySymptom(SymptomIDs.SYMPTOM_SNEEZE);
			MessagePlayer(admin, "目标打喷嚏");
		} else {
			MessagePlayer(admin, "目标在车内，此操作可能导致游戏崩溃");
		}
	}

	void SetHealthTarget( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		target.SetHealth(fValues[0]);

		MessagePlayer(admin, "目标的健康状况设置为" + fValues[0]);
	}

	void SetBloodTarget( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		PlayerBase target = ZBGetPlayerById(targetId);
		PlayerBase admin = ZBGetPlayerById(adminId);

		if (!target) {
			MessagePlayer(admin, "找不到目标（可能已断开连接？）");
			return;
		}

		target.SetHealth("", "Blood", fValues[0]);

		MessagePlayer(admin, "目标的血量设置为" + fValues[0]);
	}

	void FreeCamAdm( string funcName, int adminId, int targetId, vector cursor ) {
		int listId = m_spectatingList.Find(adminId);
		PlayerBase adminPly = ZBGetPlayerById(adminId);
		PlayerIdentity adminIdent = adminPly.GetIdentity();
		HumanInputController adminInput = adminPly.GetInputController();

		if (listId != -1) {
			adminInput.OverrideMovementSpeed(false, 0);
			adminInput.OverrideRaise(false, false);
			adminInput.OverrideAimChangeX(false, 0);
			adminInput.OverrideAimChangeY(false, 0);

			m_spectatingList.Remove(listId);

			GetGame().SelectPlayer(adminIdent, adminPly);
			MessagePlayer(ZBGetPlayerById(adminId), "返回玩家身体");
		} else {
			adminInput.OverrideMovementSpeed(true, 0);
			adminInput.OverrideRaise(true, false);
			adminInput.OverrideAimChangeX(true, 0);
			adminInput.OverrideAimChangeY(true, 0);

			m_spectatingList.Insert(adminId);

			GetGame().SelectSpectator(adminIdent, "ZomBerryCamFree", (adminPly.GetPosition() + Vector(0,1.75,0)));
			MessagePlayer(ZBGetPlayerById(adminId), "进入自由视角，使用滚轮更改移动速度");
		}
	}

	void SetTime( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		GetGame().GetWorld().SetDate(year, month, day, fValues[0], fValues[1]);
		MessagePlayer(ZBGetPlayerById(adminId), "时间设置为: " + fValues[0].ToString() + ":" + fValues[1].ToString() + " (可能需要一些时间才能生效)");
	}

	void SetDate( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		GetGame().GetWorld().SetDate(fValues[2], fValues[1], fValues[0], hour, minute);
		MessagePlayer(ZBGetPlayerById(adminId), "日期设置为: " + fValues[0].ToString() + "." + fValues[1].ToString() + "." + fValues[2].ToString() + " (可能需要一些时间才能生效)");
	}

	void TimeDay( string funcName, int adminId, int targetId, vector cursor ) {
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		GetGame().GetWorld().SetDate(year, month, day, 12, 0);
		MessagePlayer(ZBGetPlayerById(adminId), "时间设置为: 12:00 (可能需要一些时间才能生效)");
	}

	void TimeNight( string funcName, int adminId, int targetId, vector cursor ) {
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		GetGame().GetWorld().SetDate(year, month, day, 23, 0);
		MessagePlayer(ZBGetPlayerById(adminId), "时间设置为: 23:00 (可能需要一些时间才能生效)");
	}

	void SetWeather( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		int year, month, day, hour, minute;
		float fcMin, fcMax, fnMin, fnMax, ftMin, ftMax; //Damn, a lot of variables...
		Weather WMgr = GetGame().GetWeather();

		WMgr.GetFog().GetForecastChangeLimits(fcMin, fcMax); //But why would I need to Ctrl+C - Ctrl+V so much code?
		WMgr.GetFog().GetLimits(fnMin, fnMax);
		WMgr.GetFog().GetForecastTimeLimits(ftMin, ftMax); //Because users are important, and they might have their own weather settings!

		WMgr.GetFog().SetForecastChangeLimits(0, 1);
		WMgr.GetFog().SetLimits(0, 1);
		WMgr.GetFog().SetForecastTimeLimits(0, 3600);

		WMgr.GetFog().Set((fValues[0]/100), 1, 360);
		WMgr.GetFog().SetNextChange(0.01);
		WMgr.GetFog().SetForecastChangeLimits(fcMin, fcMax);
		WMgr.GetFog().SetLimits(fnMin, fnMax);
		WMgr.GetFog().SetForecastTimeLimits(ftMin, ftMax);


		WMgr.GetOvercast().GetForecastChangeLimits(fcMin, fcMax);
		WMgr.GetOvercast().GetLimits(fnMin, fnMax);
		WMgr.GetOvercast().GetForecastTimeLimits(ftMin, ftMax);

		WMgr.GetOvercast().SetForecastChangeLimits(0, 1);
		WMgr.GetOvercast().SetLimits(0, 1);
		WMgr.GetOvercast().SetForecastTimeLimits(0, 3600);

		WMgr.GetOvercast().Set((fValues[1]/100), 1, 360);
		WMgr.GetOvercast().SetNextChange(0.01);
		WMgr.GetOvercast().SetForecastChangeLimits(fcMin, fcMax);
		WMgr.GetOvercast().SetLimits(fnMin, fnMax);
		WMgr.GetOvercast().SetForecastTimeLimits(ftMin, ftMax);


		WMgr.GetRain().GetForecastChangeLimits(fcMin, fcMax);
		WMgr.GetRain().GetLimits(fnMin, fnMax);
		WMgr.GetRain().GetForecastTimeLimits(ftMin, ftMax);

		WMgr.GetRain().SetForecastChangeLimits(0, 1);
		WMgr.GetRain().SetLimits(0, 1);
		WMgr.GetRain().SetForecastTimeLimits(0, 3600);

		WMgr.GetRain().Set((fValues[2]/100), 1, 360);
		WMgr.GetRain().SetNextChange(0.01);
		WMgr.SetStorm((fValues[1]*fValues[2])/10000, 0.7, 3600/((fValues[2]/2)+0.1));
		WMgr.GetRain().SetForecastChangeLimits(fcMin, fcMax);
		WMgr.GetRain().SetLimits(fnMin, fnMax);
		WMgr.GetRain().SetForecastTimeLimits(ftMin, ftMax);

		MessagePlayer(ZBGetPlayerById(adminId), "天气变化（由于同步问题，您可能需要重新登录）");
	}

	void DeleteObj( string funcName, int adminId, int targetId, vector cursor ) {
		ref array<Object> nearest_objects = new array<Object>;
		ref array<CargoBase> proxy_cargos = new array<CargoBase>;
		Object toBeDeleted;

		GetGame().GetObjectsAtPosition3D(cursor, 1.5, nearest_objects, proxy_cargos);
		if (nearest_objects.Count() < 1) return;
		toBeDeleted = NULL;

		for (int i = 0; i < nearest_objects.Count(); ++i) {
			string tempObjId = nearest_objects.Get(i).ToString(); tempObjId.ToLower();
			if (nearest_objects.Get(i).IsKindOf("SurvivorBase") || tempObjId.Contains("static")) continue;
			if (nearest_objects.Get(i).IsWell() || nearest_objects.Get(i).IsBush()) continue;
			if (nearest_objects.Get(i).IsRock() || nearest_objects.Get(i).IsTree()) continue;

			toBeDeleted = nearest_objects.Get(i);
			break;
		}
		if (!toBeDeleted) return;

		if (!m_deleteList.Contains(adminId)) {
			m_deleteList.Insert(adminId, toBeDeleted.ToString());
			MessagePlayer(ZBGetPlayerById(adminId), "您将消除 " + toBeDeleted.ToString() + ", 再次执行此动作以继续");
		} else if (m_deleteList.Get(adminId) != toBeDeleted.ToString()) {
			m_deleteList.Set(adminId, toBeDeleted.ToString());
			MessagePlayer(ZBGetPlayerById(adminId), "您将消除 " + toBeDeleted.ToString() + ", 再次执行此动作以继续");
		} else if (m_deleteList.Get(adminId) == toBeDeleted.ToString()) {
			MessagePlayer(ZBGetPlayerById(adminId), toBeDeleted.ToString() + " 消除");
			GetGame().ObjectDelete(toBeDeleted);
			m_deleteList.Remove(adminId);
		}
	}
};
