static string g_zbryVer = "0.6.4";

class ZomberryBase {
	protected string remoteZbryVer = g_zbryVer;
	protected bool isAdmin = false;
	protected int instStage = 1;
	protected autoptr TStringArray adminList = new TStringArray;
	ref ZomberryStockFunctions m_ZomberryStockFunctions;
	static ref ZomberryConfig m_ZomberryConfig;
	static ref ZomberryLogger m_ZomberryLogger;
	static ref ZomberryKeybinds m_ZomberryKeybinds;

	void ZomberryBase() {
		m_ZomberryStockFunctions = new ref ZomberryStockFunctions;
		string tmpStr = "";

		if (!GetConfig().IsDefaultIO()) {
			GetLogger().SwitchToCustomIO();
		}

		if (GetCLIParam("zbryInstallMode", tmpStr) && GetGame().IsMultiplayer()) {
			if (tmpStr == "true") {
				GetZomberryCmdAPI().AddCategory("ZomBerry 安装模式", COLOR_RED);
				GetZomberryCmdAPI().AddCommand("文件中的管理员列表", "InstMode", this, "ZomBerry 安装模式", false, {
					new ZBerryFuncParam("Step", {0, 0, 0,}),
				});
				GetZomberryCmdAPI().AddCommand("第一步：检查是否有管理员。发现了cfg", "InstMode", this, "ZomBerry 安装模式", false, {
					new ZBerryFuncParam("Step", {1, 1, 1,}),
				});
			}
		} else {
			if (GetGame().IsServer() || !GetGame().IsMultiplayer()) m_ZomberryStockFunctions.Init();
		}

		if (GetConfig().GetDebugLvl() >= 2) {
			GetZomberryCmdAPI().Debug();
		}

		GetRPCManager().AddRPC( "ZomBerryAT", "AdminAuth", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "ZomBerryAT", "SyncPlayersRequest", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "ZomBerryAT", "SyncFunctionsRequest", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "ZomBerryAT", "ExecuteCommand", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "ZomBerryAT", "SpawnObject", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "ZomBerryAT", "MapTeleport", this, SingeplayerExecutionType.Client );
		//GetRPCManager().AddRPC( "ZomBerryAT", "MiscFunctionData", this, SingeplayerExecutionType.Server );
	}

	static ref ZomberryConfig GetConfig() {
		if ( !m_ZomberryConfig ) {
			m_ZomberryConfig = new ref ZomberryConfig;
		}

		return m_ZomberryConfig;
	}

	static ref ZomberryLogger GetLogger() {
		if ( !m_ZomberryLogger ) {
			m_ZomberryLogger = new ref ZomberryLogger;
		}

		return m_ZomberryLogger;
	}

	static ref ZomberryKeybinds GetKeyBindsMgr() {
		if ( !m_ZomberryKeybinds ) {
			m_ZomberryKeybinds = new ref ZomberryKeybinds;
		}

		return m_ZomberryKeybinds;
	}

	static void Log( string module, string toLog ) {
		GetLogger().Log( module, toLog );
	}

	static void DebugLog( int dLvl, string module, string toLog ) {
		GetLogger().DebugLog( dLvl, module, toLog );
	}

	void OnClientReady() {
		Log( "ZomBerryDbg", "已发送身份验证请求" );

		GetRPCManager().SendRPC( "ZomBerryAT", "AdminAuth", new Param2< bool, string >( true, g_zbryVer ), true, NULL );
	}

	void OnServerReady() {
		adminList = GetConfig().ConfigureAdmins();
	}

	string GetRemoteVersion() {
		return remoteZbryVer;
	}

	bool IsAdmin(PlayerIdentity plyIdent = NULL) {
		string tmpStr = "";
		if (!GetGame().IsMultiplayer() || GetGame().IsClient()) return isAdmin;
		if ((adminList.Find(plyIdent.GetId()) != -1) || (adminList.Find(plyIdent.GetPlainId()) != -1)) return true;

		if (GetCLIParam("zbryGiveAdminRightsToEveryone", tmpStr) || GetCLIParam("zbryInstallMode", tmpStr)) {
			if (tmpStr == "true") return true;
		}
		return false;
	}

	void InstMode( string funcName, int adminId, int targetId, vector cursor, autoptr TIntArray fValues ) {
		int step = fValues[0];
		string admPath = "";
		TStringArray outMessages = {};
		PlayerBase adminPly = ZBGetPlayerById(adminId);

		if (!GetCLIParam("zbryInstallMode", admPath)) {
			if (admPath != "true") return;
		}

		switch (step) {
			case 0:
				outMessages.Insert("admins 中的 admins.cfg 列表（如果存在）：");
				outMessages.InsertAll(GetConfig().ConfigureAdmins());
				break;

			case 1:
				admPath = GetConfig().InstFindAdmins();
				if (admPath == "") {
					outMessages.Insert("admins.cfg 没有找到");
				} else {
					if (admPath == "$CurrentDir:\\ZomBerry\\Config\\") outMessages.Insert("admins.cfg 在默认的mod位置找到，请尝试在正确的位置创建它！");
					outMessages.Insert("admins.cfg 被发现于: " + admPath);
				}

				if (instStage == 1) {
					GetZomberryCmdAPI().AddCommand("第2步：尝试创建 admins.cfg 在适当的位置", "InstMode", this, "ZomBerry 安装模式", false, {
						new ZBerryFuncParam("Step", {2, 2, 2,}),
					});
					if (admPath != "" && admPath != "$CurrentDir:\\ZomBerry\\Config\\") {
						GetZomberryCmdAPI().AddCommand("第3步：尝试将您的BIGUID添加到文件中", "InstMode", this, "ZomBerry 安装模式", false, {
							new ZBerryFuncParam("Step", {3, 3, 3,}),
						});
						++instStage;
					}
					++instStage;
					SyncFunctionsRequest( CallType.Server, NULL, adminPly.GetIdentity(), NULL );
				}
				break;

			case 2:
				string stageTwoResult = GetConfig().InstCreateAdmins();
				outMessages.Insert("admins.cfg 创造结果: " + stageTwoResult);

				if (instStage == 2 && (stageTwoResult == "OK" || stageTwoResult == "AlreadyExists")) {
					GetZomberryCmdAPI().AddCommand("第3步：尝试将您的BIGUID添加到文件中", "InstMode", this, "ZomBerry安装模式", false, {
						new ZBerryFuncParam("Step", {3, 3, 3,}),
					});
					++instStage;
					SyncFunctionsRequest( CallType.Server, NULL, adminPly.GetIdentity(), NULL );
				}
				break;

			case 3:
				if (GetConfig().InstInsertAdmin(adminPly.GetIdentity().GetId())) {
					outMessages.Insert("您的BIGUID已成功添加到 admins.cfg!");
				} else {
					outMessages.Insert("无法将你的BIGUID添加到 admins.cfg");
				}
				break;
		}

		for (int i = 0; i < outMessages.Count(); i++) {
			Param1<string> param = new Param1<string>( "[ZomBerry]: " + outMessages[i] );
			GetGame().RPCSingleParam(adminPly, ERPCs.RPC_USER_ACTION_MESSAGE, param, true, adminPly.GetIdentity());
		}
	}

	void AdminAuth( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		Param2< bool, string > authInfo; //direction (true = to server), version (only in client dir)
		if ( !ctx.Read( authInfo ) ) {
			Log( "ZomBerryAT", "警告: " + sender.GetName() + " (" + sender.GetId() + ") - 验证错误，请将客户端更新为最新版本 (v" + g_zbryVer + "+)!");
			return;
		}

		if ( type == CallType.Server ) {
			if (IsAdmin(sender)) {
				GetRPCManager().SendRPC( "ZomBerryAT", "AdminAuth", new Param2< bool, string >( false, g_zbryVer ), true, sender );
				Log( "ZomBerryDbg", "授权回复管理员 " + sender.GetName() + " (" + sender.GetId() + ")");
				if (authInfo.param2 != g_zbryVer) {
					Log( "ZomBerryAT", "警告: Admin " + sender.GetName() + " (" + sender.GetId() + ") ZomBerry 版本不匹配！ S: v" + g_zbryVer + ", C: v" + authInfo.param2 + ", 客户端可能无法启动!");
				}
			} else {
				if (adminList.Count() <= 1) GetRPCManager().SendRPC( "ZomBerryAT", "AdminAuth", new Param2< bool, string >( false, g_zbryVer + "CFGFailed" ), true, sender );
				Log( "ZomBerryDbg", "已忽略身份验证请求（不是管理员） " + sender.GetName() + " (" + sender.GetId() + ")" );
			}
		} else {
			if (!authInfo.param1 || GetGame().IsMultiplayer()) {
				Log( "ZomBerryDbg", "收到授权回复" );
				remoteZbryVer = authInfo.param2;
				if (authInfo.param2.Substring(0, 3) != g_zbryVer.Substring(0, 3)) {
					Log( "ZomBerryAT", "警告：ZomBerry版本不匹配！C: v" + g_zbryVer + ", S: v" + authInfo.param2 + ", 客户端不会启动!");
				} else {
					if (authInfo.param2 != g_zbryVer) Log( "ZomBerryAT", "警告: ZomBerry版本不匹配！ C: v" + g_zbryVer + ", S: v" + authInfo.param2);
					if (!remoteZbryVer.Contains("CFGFailed")) isAdmin = true;
				}
			} else {
				Log( "ZomBerryDbg", "授权被忽略，单人游戏" );
				isAdmin = true;
			}
			GetConfig();
		}
	}

	void SyncPlayersRequest( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		ref ZBerryPlayer plyData;
		ref ZBerryPlayerArray playerListS = new ZBerryPlayerArray;
		array<Man> players = new array<Man>;
		PlayerBase player;
		int plyId;
		string plyName;
		bool plyAdmin;

		if ( type == CallType.Server && GetGame().IsServer() ) {
			if (IsAdmin(sender)) {
				if (GetCLIParam("zbryInstallMode", plyName)) {
					if (plyName == "true") {
						playerListS.Insert(new ZBerryPlayer(3, "ZomBerry", false, Vector(0,0,0), 0, 0));
						playerListS.Insert(new ZBerryPlayer(3, "Installation Mode", false, Vector(0,0,0), 0, 0));
						playerListS.Insert(new ZBerryPlayer(3, "Active", true, Vector(0,0,0), 0, 0));
						playerListS.Insert(new ZBerryPlayer(3, "No real players displayed!", false, Vector(0,0,0), 0, 0));
						GetRPCManager().SendRPC( "ZomBerryAT", "SyncPlayers", new Param2<ref ZBerryPlayerArray, int> (playerListS, GetGame().GetTime()), true, sender );

						return;
					}
				}

				GetGame().GetPlayers(players);

				for (int i = 0; i < players.Count(); ++i) {
					player = PlayerBase.Cast(players.Get(i));
					plyId = player.GetIdentity().GetPlayerId();
					plyName = player.GetIdentity().GetName();
					plyAdmin = IsAdmin(player.GetIdentity());

					if (player.GetItemInHands() && !player.GetCommand_Vehicle()) {plyName += (" [" + player.GetItemInHands().GetInventoryItemType().GetName() + "]")}
					if (player.GetCommand_Vehicle()) {plyName += (" [" + player.GetCommand_Vehicle().GetTransport().GetDisplayName() + "]")}

					plyData = new ZBerryPlayer(plyId, plyName, plyAdmin, player.GetPosition(), player.GetHealth(), player.GetHealth("", "Blood"));
					playerListS.Insert(plyData);
				}

				GetRPCManager().SendRPC( "ZomBerryAT", "SyncPlayers", new Param2<ref ZBerryPlayerArray, int> (playerListS, GetGame().GetTime()), true, sender );
				Log( "ZomBerryDbg", "" + sender.GetName() + " (" + sender.GetId() + ") - 玩家列表同步");

			} else {
				Log( "ZomBerryDbg", "" + sender.GetName() + " (" + sender.GetId() + ") - 玩家列表同步被拒绝（不是管理员）" );
			}
		} else {
			player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player.GetItemInHands()) {plyName = (" [" + player.GetItemInHands().GetInventoryItemType().GetName() + "]")}

			plyData = new ZBerryPlayer(0, "Player" + plyName, true, player.GetPosition(), player.GetHealth(), player.GetHealth("", "Blood"));

			playerListS.Insert(plyData);
			GetRPCManager().SendRPC( "ZomBerryAT", "SyncPlayers", new Param2<ref ZBerryPlayerArray, int> (playerListS, GetGame().GetTime()), true, NULL );
			Log( "ZomBerryDbg", "服务器列表同步单服务器");
		}
	}

	void SyncFunctionsRequest( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		ref ZBerryCategoryArray catList;

		if ( type == CallType.Server && GetGame().IsServer() ) {
			if (IsAdmin(sender)) {
				catList = GetZomberryCmdAPI().GetList();

				GetRPCManager().SendRPC( "ZomBerryAT", "SyncFunctions", new Param1<ref ZBerryCategoryArray> (catList), true, sender );
				Log( "ZomBerryDbg", "" + sender.GetName() + " (" + sender.GetId() + ") - 功能列表同步");

			} else {
				Log( "ZomBerryDbg", "" + sender.GetName() + " (" + sender.GetId() + ") - 玩家列表同步被拒绝（不是管理员）" );
			}
		} else {
			catList = GetZomberryCmdAPI().GetList();

			GetRPCManager().SendRPC( "ZomBerryAT", "SyncFunctions", new Param1<ref ZBerryCategoryArray> (catList), true, NULL );
			Log( "ZomBerryDbg", "服务器列表同步单服务器");
		}
	}

	void ExecuteCommand( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		Param5< int, int, int, vector, autoptr TIntArray > funcParam; //Function id, Admin Id, Target Id, Admin cursor, Function params
		Param5< string, int, int, vector, autoptr TIntArray > oldFuncParam; //Backwards compatibility
		if ( !ctx.Read( funcParam ) ) return;

		oldFuncParam = new Param5< string, int, int, vector, autoptr TIntArray >(GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName(), funcParam.param2, funcParam.param3, funcParam.param4, funcParam.param5 );

		int targetId = funcParam.param3;
		if ( type == CallType.Server && GetGame().IsServer() ) {
			PlayerIdentity targetIdent = ZBGetPlayerById(targetId).GetIdentity();
			if (IsAdmin(sender)) {
				GetGame().GameScript.CallFunctionParams( GetZomberryCmdAPI().GetFunc(funcParam.param1).GetInstance(), GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName(), NULL, oldFuncParam );
				if (targetId != funcParam.param2) {
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") executed " + GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName() + " on target " + targetIdent.GetName()  + " (" + targetIdent.GetId() + ")");
				} else {
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") executed " + GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName());
				}
			} else {
				Log( "ZomBerryAdmin", "警告: " + sender.GetName() + " (" + sender.GetId() + ") 试图解决 " + GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName() + " 但不是管理员");
			}
		} else {
			if (!GetGame().IsMultiplayer()) {
				Log( "ZomBerryAdmin", "Executed " + GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName() + " (singleplayer)");
				GetGame().GameScript.CallFunctionParams( GetZomberryCmdAPI().GetFunc(funcParam.param1).GetInstance(), GetZomberryCmdAPI().GetFunc(funcParam.param1).GetName(), NULL, oldFuncParam );
			}
		}
	}

	void SpawnObject( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		//string objName, int adminId, vector targetPlace, bool insideInventory
		Param4< string, int, vector, bool > tgtParam;
		ItemBase item;
		string tmpStr = "";

		if (GetCLIParam("zbryInstallMode", tmpStr)) {
			if (tmpStr == "true") return;
		}

		if ( !ctx.Read( tgtParam ) ) return;

		if ( type == CallType.Server && GetGame().IsServer() ) {
			if (IsAdmin(sender)) {
				if (tgtParam.param4) {
					item = ItemBase.Cast(ZBGetPlayerById(tgtParam.param2).GetInventory().CreateInInventory(tgtParam.param1));
					if (item) item.SetQuantity(item.GetQuantityMax());
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 添加的 " + tgtParam.param1 + " 他们的库存");
				} else {
					GetGame().CreateObject(tgtParam.param1, tgtParam.param3, false, true );
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 生成 " + tgtParam.param1 + " 在 " + tgtParam.param3);
				}
			} else {
				Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 试图繁殖 " + tgtParam.param1 + " (不是管理员)");
			}
		} else {
			if (!GetGame().IsMultiplayer()) {
				if (tgtParam.param4) {
					item = ItemBase.Cast(ZBGetPlayerById(tgtParam.param2).GetInventory().CreateInInventory(tgtParam.param1));
					if (item) item.SetQuantity(item.GetQuantityMax());
					Log( "ZomBerryAdmin", "添加 " + tgtParam.param1 + " 盘点 (单人游戏)");
				} else {
					GetGame().CreateObject(tgtParam.param1, tgtParam.param3, false, true );
					Log( "ZomBerryAdmin", "生成 " + tgtParam.param1 + " 在 " + tgtParam.param3 + " (单人游戏)");
				}
			}
		}
	}

	void MapTeleport( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target ) {
		Param1< vector > teleParam;

		if ( !ctx.Read( teleParam ) ) return;

		float atlZ = GetGame().SurfaceY(teleParam.param1[0], teleParam.param1[2]);
		vector reqpos = Vector(teleParam.param1[0], atlZ, teleParam.param1[2]);
		int adminId = 0;
		if (GetGame().IsMultiplayer()) adminId = sender.GetPlayerId();
		PlayerBase adminPly = ZBGetPlayerById(adminId);

		if ( type == CallType.Server && GetGame().IsServer() ) {
			if (IsAdmin(sender)) {
				if (!adminPly.GetCommand_Vehicle()) {
					adminPly.SetPosition(reqpos);
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 远程传送到位置 " + reqpos.ToString());
				} else {
					Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 试图传送，但在车里");
				}
			} else {
				Log( "ZomBerryAdmin", "" + sender.GetName() + " (" + sender.GetId() + ") 试图传送（不是管理员）");
			}
		} else {
			if (!GetGame().IsMultiplayer()) {
				if (!adminPly.GetCommand_Vehicle()) {
					adminPly.SetPosition(reqpos);
					Log( "ZomBerryAdmin", "远程传送到位置 " + reqpos.ToString());
				} else {
					Log( "ZomBerryAdmin", "传送失败，玩家在车内");
				}
			}
		}
	}
};

modded class MissionServer {
	ref ZomberryBase m_ZomberryBase;

	void MissionServer() {
		ZomberryBase.Log( "ZomBerry", "启动服务器端 v" + g_zbryVer );
	}

	private ref ZomberryBase GetZomberryBase() {
		if ( !m_ZomberryBase ) {
			m_ZomberryBase = new ref ZomberryBase;
		}

		return m_ZomberryBase;
	}

	override void OnInit() {
		super.OnInit();

		GetZomberryBase().OnServerReady();
	}
};

modded class MissionGameplay {
	bool m_plyWarned = false;
	ref ZomberryBase m_ZomberryBase;
	ref ZomberryMenu m_ZomberryMenu;

	void MissionGameplay() {
		m_ZomberryBase = new ref ZomberryBase;

		ZomberryBase.Log( "ZomBerry", "启动服务器端 v" + g_zbryVer );
	}

	void ~MissionGameplay() {

		if (m_ZomberryMenu) delete m_ZomberryMenu;
		if (m_ZomberryBase) delete m_ZomberryBase;
	}

	private ref ZomberryMenu GetZomberryMenu() {
		if ( !m_ZomberryMenu ) {
			m_ZomberryMenu = new ref ZomberryMenu;
			m_ZomberryMenu.Init();
			GetRPCManager().AddRPC( "ZomBerryAT", "SyncPlayers", m_ZomberryMenu, SingeplayerExecutionType.Client );
			GetRPCManager().AddRPC( "ZomBerryAT", "SyncFunctions", m_ZomberryMenu, SingeplayerExecutionType.Client );
		}

		return m_ZomberryMenu;
	}

	override void OnMissionStart() {
		super.OnMissionStart();

		m_ZomberryBase.OnClientReady();
	}

	override void OnKeyPress(int key) {
		super.OnKeyPress(key);

		if (GetZomberryMenu().GetLayoutRoot().IsVisible()) {
			GetZomberryMenu().OnKeyPress(key);
		} else if (m_ZomberryBase.IsAdmin()) {
			ZomberryBase.GetKeyBindsMgr().OnKeyPress(key);
		}
	}

	void DisplayMessage(string msg) {
		GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCAdmin, "", msg, ""));
	}

	override void OnUpdate(float timeslice) {
		super.OnUpdate(timeslice);

		if (GetUApi().GetInputByName("UAZBerryOpenAdminMenu").LocalPress()) {
			string r_zbryVer = m_ZomberryBase.GetRemoteVersion();
			UIScriptedMenu menu = GetUIManager().GetMenu();

			if (!menu && !GetZomberryMenu().GetLayoutRoot().IsVisible() && m_ZomberryBase.IsAdmin()) {
				GetUIManager().ShowScriptedMenu( GetZomberryMenu(), NULL );
				PlayerControlDisable(INPUT_EXCLUDE_ALL);
			} else if (GetZomberryMenu().GetLayoutRoot().IsVisible() && GetZomberryMenu().GetCloseClearance()) {
				GetUIManager().HideScriptedMenu( GetZomberryMenu() );
				PlayerControlEnable(false);
			} else if (r_zbryVer.Substring(0, 3) != g_zbryVer.Substring(0, 3)) {
				DisplayMessage("[ZomBerry]: Admin auth成功，但由于版本不匹配，客户端被禁用。 C: " + g_zbryVer + ", S: " + r_zbryVer);
			} else if (r_zbryVer.Contains("CFGFailed")) {
				DisplayMessage("[ZomBerry]: 服务器端已加载，但配置错误。列表中有0名管理员，请检查管理员。脚本日期时间。日志和常见问题解答");
			}
			if (g_zbryVer.Substring(2, 3).ToFloat() > r_zbryVer.Substring(2, 3).ToFloat() && !m_plyWarned) {
				DisplayMessage("[ZomBerry] 信息： 别忘了将服务器更新为 v" + g_zbryVer + "+ 获取最新功能!");
				DisplayMessage("[ZomBerry] 信息： 服务器当前正在上运行: v" + r_zbryVer);
				m_plyWarned = true;
			}
		}

		if (GetUApi().GetInputByName("UAUIBack").LocalPress() && GetZomberryMenu().GetLayoutRoot().IsVisible() && GetZomberryMenu().GetCloseClearance()) {
			GetUIManager().HideScriptedMenu( GetZomberryMenu() );
			PlayerControlEnable(false);
		}
	}
};
