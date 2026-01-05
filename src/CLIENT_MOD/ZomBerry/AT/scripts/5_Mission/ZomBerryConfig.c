class ZomberryConfig {
	private string cfgPath = "$saves:";
	const private string cfgPathServer = "$profile:";
	protected string realProfilesPath = "";

	protected int menuKey = KeyCode.KC_M;
	protected int zbryDebug = 0;
	protected bool defaultIOFlag = false;
	protected autoptr TStringIntMap m_keyBindList = new TStringIntMap;
	protected autoptr ZBerryJsonSpawnMenuGroupArray m_spawnMenuGroups;

	void ZomberryConfig() {
		if (GetGame().IsMultiplayer() && GetGame().IsServer()) {
			if (!GetCLIParam("profiles", realProfilesPath)) ZomberryBase.Log( "ZomBerryConfig", "警告：“-profiles = ”启动参数未设置！！！" );
				else ZomberryBase.Log( "ZomBerryConfig", "配置文件目录是: " + realProfilesPath);

			cfgPath = cfgPathServer;

			if (!FileExist(cfgPath + "ZomBerry\\")) MakeDirectory(cfgPath + "\\ZomBerry\\");
			if (FileExist(cfgPath + "ZomBerry\\")) {
				if (FileExist(cfgPath + "ZomBerry.cfg")) CopyFile(cfgPath + "ZomBerry.cfg", cfgPath + "ZomBerry\\ZomBerry.cfg");
				if (FileExist(cfgPath + "ZomBerryConfig.json")) CopyFile(cfgPath + "ZomBerryConfig.json", cfgPath + "ZomBerry\\ZomBerryConfig.json");
				DeleteFile(cfgPath + "ZomBerry.cfg");
				DeleteFile(cfgPath + "ZomBerryConfig.json");

				cfgPath += "ZomBerry\\";
			} else {
				if (realProfilesPath != "") ZomberryBase.Log( "ZomBerryConfig", "警告：无法创建ZomBerry子文件夹（" + realProfilesPath + "\\ZomBerry\\), 请手动创建一个！" );
					else ZomberryBase.Log( "ZomBerryConfig", "警告：无法创建ZomBerry子文件夹（" + cfgPath + "ZomBerry\\), 请手动创建一个!" );
			}
		}

		if (realProfilesPath != "") ZomberryBase.Log( "ZomBerryConfig", "信息： 配置文件将从: " + realProfilesPath );
			else ZomberryBase.Log( "ZomBerryConfig", "信息： 配置文件将从: " + cfgPath );
		Configure();
	}

	int GetMenuKey() {
		return menuKey;
	}

	int GetDebugLvl() {
		return zbryDebug;
	}

	string GetProfilesPath() {
		return realProfilesPath;
	}

	ref ZBerryJsonSpawnMenuGroupArray GetSpawnMenuData() {
		return m_spawnMenuGroups;
	}

	bool IsDefaultIO() {
		return defaultIOFlag;
	}

	int GetKeyByFunc(string fName) {
		if (m_keyBindList.Contains(fName)) return m_keyBindList.Get(fName);

		return -1;
	}

	string GetFuncByKey(int key) {
		string fName = m_keyBindList.GetKeyByValue(key);
		if (fName) return fName;

		return "";
	}

	void UpdateKeyBind(string fName, int key) {
		FileSerializer kBindsFile = new FileSerializer();

		string toBeDeleted = m_keyBindList.GetKeyByValue(key);
		if (toBeDeleted) m_keyBindList.Remove(toBeDeleted);
		if (m_keyBindList.Contains(fName)) m_keyBindList.Remove(fName);

		if (fName != "REMOVE") m_keyBindList.Insert(fName, key);
		if (fName == "ALL" && key == -2) m_keyBindList.Clear();

		if (kBindsFile.Open(cfgPath + "ZomBerryKeybinds.bin", FileMode.WRITE)) {
			kBindsFile.Write(m_keyBindList);
			kBindsFile.Close();

			if (fName != "REMOVE" && fName != "ALL") {
				ZomberryBase.Log( "ZomBerryConfig", "信息： 更新了KeyBinds文件: " + fName + " 绑定到 " + key );
			} else {
				ZomberryBase.Log( "ZomBerryConfig", "信息： 更新的KeyBinds文件：取消绑定功能 " + key + " key(s) " );
			}
		}
	}

	private void Configure() { //TODO: JSONize this

		if (FileExist(cfgPath + "ZomBerry.cfg") && !FileExist(cfgPath + "ZomBerryConfig.json")) ReadOldConfig(); //Read an old config, new one will be created with old params

		CreateNew(cfgPath);

		if (GetGame().IsClient() || !GetGame().IsMultiplayer()) ConfigureKeybinds();

		ConfigureMain();

		/*UAInput zbryMenu = GetUApi().GetInputByName( "UAZomberryOpenMenu" );  //UApi is not ready yet
		if (zbryMenu.BindingCount() == 0) {
		GetUApi().DeRegisterInput( "UAZomberryOpenMenu" );
		zbryMenu = GetUApi().RegisterInput( "UAZomberryOpenMenu", "STR_USRACT_OPEN_ZBRY_MENU", "infantry" );
		zbryMenu.BindCombo( "kK" );
		GetUApi().Export();
		Print ("[ZomBerryDbg] KeyBind not found, created new one.");*/
	}

	private void ConfigureMain() {
		ref ZBerryJsonConfig newConfigData;

		JsonFileLoader<ZBerryJsonConfig>.JsonLoadFile(cfgPath + "ZomBerryConfig.json", newConfigData);
		if (!newConfigData) {
			ZomberryBase.Log( "ZomBerryConfig", "错误：无法找到配置文件，将继续使用默认设置（MenuKey为O，已禁用调试，单独的日志文件）" );
			m_spawnMenuGroups = {
				new ZBerryJsonSpawnMenuGroup("物品服装","Edible_Base,Weapon_Base,Magazine_Base,Clothing_Base"),
				new ZBerryJsonSpawnMenuGroup("车辆建筑","Transport,House"),
				new ZBerryJsonSpawnMenuGroup("动物僵尸","DZ_LightAI"),
			};
			return;
		}

		zbryDebug = newConfigData.DebugLevel;
		defaultIOFlag = newConfigData.UseScriptLog;

		if (zbryDebug != 0) ZomberryBase.Log( "ZomBerryConfig", "信息： 调试级别 " + zbryDebug );
		if (defaultIOFlag) ZomberryBase.Log( "ZomBerryConfig", "信息： 将使用脚本日志文件..." );

		if (GetGame().IsClient() || !GetGame().IsMultiplayer()) { //TO BE CHANGED v0.6 - GUI CFG
			string tempKeyCode;

			for (int i = 0; i < 126; ++i) {
				tempKeyCode = typename.EnumToString(KeyCode, i);
				if (newConfigData.MenuKey == tempKeyCode) {
					menuKey = i;
					ZomberryBase.Log( "ZomBerryConfig", "信息： 菜单键设置为 " + tempKeyCode );
					break;
				}
			}

			if ((menuKey == KeyCode.KC_M) && (newConfigData.MenuKey != "KC_M"))
				ZomberryBase.Log( "ZomBerryConfig", "警告：无法将菜单键设置为 " + newConfigData.MenuKey + " - 未知的按键" );

			m_spawnMenuGroups = newConfigData.SpawnMenuSorting;
		}
	}

	private void ConfigureKeybinds() {
		FileSerializer kBindsFile = new FileSerializer();
		int idx = 0;

		if (kBindsFile.Open(cfgPath + "ZomBerryKeybinds.bin", FileMode.READ)) {
			kBindsFile.Read(m_keyBindList);
			kBindsFile.Close();

			ZomberryBase.Log( "ZomBerryConfig", "INFO: Loaded KeyBinds file with " + m_keyBindList.Count().ToString() + " entries" );

			for (idx = 0; idx < m_keyBindList.Count(); idx++) {
				ZomberryBase.Log( "ZomBerryConfig", "INFO: Key " + typename.EnumToString(KeyCode, m_keyBindList.GetElement(idx)) + " - " + m_keyBindList.GetKey(idx));
			}
		}
	}

	private void ReadOldConfig() {
		FileHandle configFile = OpenFile(cfgPath + "ZomBerry.cfg", FileMode.READ);
		int idx = 0;

		ZomberryBase.Log( "ZomBerryConfig", "信息： 似乎我们找到了一个旧的配置文件，它将尝试转换为JSON格式（从现在开始使用ZomBerryConfig.json！)" );

		if (configFile != 0) {
			string sLine = "";
			TStringArray sParams = new TStringArray;

			while ( FGets(configFile,sLine) > 0 ) {
				++idx;
				sLine.Replace(" ", ""); sLine.Replace(";", "");
				sLine.Split("=", sParams);
				if (sParams.Count() != 2) {
					ZomberryBase.Log( "ZomBerryConfig", "警告：配置行 " + idx.ToString() + " (停止 " + sParams[0] + ") - 解析错误（非键=值）, " + sParams.Count() + " 代币" );
					sParams = {};
					continue;
				}

				switch (sParams[0]) {
					case "debug":
					if (sParams[1].ToInt() > 0 && sParams[1].ToInt() < 3) {
						zbryDebug = sParams[1].ToInt();
						ZomberryBase.Log( "ZomBerryConfig", "信息： 调试级别: " + sParams[1] );
					}
					break;

					case "menuKey":
					string tempKeyCode;
					if (GetGame().IsMultiplayer() && GetGame().IsServer()) continue;
					for (int i = 0; i < 126; ++i) {
						tempKeyCode = typename.EnumToString(KeyCode, i);
						if (sParams[1] == tempKeyCode) {
							menuKey = i;
							ZomberryBase.Log( "ZomBerryConfig", "信息： 菜单键设置为 " + tempKeyCode );
							break;
						}
					}
					if (sParams[1] != tempKeyCode)
						ZomberryBase.Log( "ZomBerryConfig", "WARN: 无法将菜单键设置为 " + sParams[1] + " - 未知密钥码" );
					break;
				}
				sParams = {};
			}
			CloseFile(configFile);
			DeleteFile(cfgPath + "ZomBerry.cfg");
		}
	}

	ref TStringArray ConfigureAdmins() { //TODO: JSONize this
		ref TStringArray adminList = new TStringArray;
		string temp_path;

		if (GetCLIParam("zbryGiveAdminRightsToEveryone", temp_path)) {
			if (temp_path == "true") {
				ZomberryBase.Log( "ZomBerryConfig", "警告：警告！由于'-zbryGiveAdminRightsToEveryone=true'启动参数，加入此服务器的每个玩家都将拥有完整的管理员权限！" );
				return {};
			}
		}

		if (!GetCLIParam("zbryInstallMode", temp_path)) adminList.Insert("YWrRTYsUNXUHr2ALuJGiTQ7nvnae8XcTxe3XvJ3Ay54=");
		temp_path = FindAdmins();
		ZomberryBase.Log( "ZomBerryDbg", "服务器就绪，正在从中加载管理员列表: " + temp_path + "admins.cfg");

		FileHandle adminFile = OpenFile(temp_path + "admins.cfg", FileMode.READ);
		if (adminFile != 0) {
			string sLine = "";
			ZomberryBase.Log( "ZomBerryConfig", "admins.cfg loaded");
			while ( FGets(adminFile,sLine) > 0 ) {
				adminList.Insert(sLine);
				ZomberryBase.Log( "ZomBerryConfig", "添加管理员: " + sLine);
			}
			CloseFile(adminFile);
		} else {
			ZomberryBase.Log( "ZomBerryConfig", "FATAL: 管理员。cfg加载失败");
		}

		return adminList;
	}

	private string FindAdmins(bool doLogs = true) {
		string temp_path = "$CurrentDir:\\" + g_Game.GetMissionPath();

		temp_path.Replace("mission.c", "");

		if (FileExist(temp_path + "admins.cfg")) {
			if (doLogs) ZomberryBase.Log( "ZomBerryConfig", "警告！！！：使用管理员。来自任务目录的cfg，该目录在v0之后不会被使用。6释放！");
		} else if (FileExist("$profile:ZomBerry\\admins.cfg")) {
			temp_path = "$profile:ZomBerry\\";
			if (doLogs) ZomberryBase.Log( "ZomBerryConfig", "信息：使用管理员。来自配置文件目录");
		} else if (FileExist("$CurrentDir:admins.cfg")) {
			temp_path = "$CurrentDir:";
			if (doLogs) ZomberryBase.Log( "ZomBerryConfig", "警告：使用管理员。来自服务器根目录的cfg（最好使用Profile dir！）" );
		} else if (FileExist("$CurrentDir:\\ZomBerry\\Config\\admins.cfg")) {
			temp_path = "$CurrentDir:ZomBerry\\Config\\";
			if (doLogs) ZomberryBase.Log( "ZomBerryConfig", "警告：使用管理员。来自ZomBerry加载项目录的cfg（最好使用Profile dir！）" );
		} else {
			temp_path = "";
		}
		
		return temp_path;
	}

	private void CreateNew(string dPath) {
		if (FindAdmins(false) == "") {
			FileHandle adminsFile = OpenFile(dPath + "admins.cfg", FileMode.WRITE);

			ZomberryBase.Log( "ZomBerryConfig", "信息：正在尝试创建新管理员。cfg文件 " + dPath + "admins.cfg" );

			if (adminsFile != 0) {
				FPrintln(adminsFile, "76561198038543835");
				ZomberryBase.Log( "ZomBerryConfig", "信息：管理员。cfg创建成功。" );
				CloseFile(adminsFile);
			}
		}

		if (!FileExist(dPath + "ZomBerryConfig.json")) {
			ref ZBerryJsonConfig newConfigData = new ZBerryJsonConfig();

			ZomberryBase.Log( "ZomBerryConfig", "信息：正在尝试在中创建新的配置文件 " + dPath + "ZomBerryConfig.json" );

			newConfigData.DebugLevel = zbryDebug;
			newConfigData.UseScriptLog = false;
			newConfigData.MenuKey = typename.EnumToString(KeyCode, menuKey);
			newConfigData.SpawnMenuSorting = {
												new ZBerryJsonSpawnMenuGroup("物品服装","Edible_Base,Weapon_Base,Magazine_Base,Clothing_Base"),
												new ZBerryJsonSpawnMenuGroup("车辆建筑","Transport,House"),
												new ZBerryJsonSpawnMenuGroup("动物僵尸","DZ_LightAI"),
											 };
			newConfigData.PermissionGroups = {};

			JsonFileLoader<ZBerryJsonConfig>.JsonSaveFile(cfgPath + "ZomBerryConfig.json", newConfigData);
		}

		if (!FileExist(dPath + "ZomBerryKeybinds.bin") && GetGame().IsClient()) {
			FileSerializer kBindsFile = new FileSerializer();
			autoptr TStringIntMap keyBindList = new TStringIntMap;

			ZomberryBase.Log( "ZomBerryConfig", "信息：找不到密钥绑定文件，正在尝试在中创建新文件 " + dPath + "ZomBerryKeybinds.bin" );

			keyBindList.Insert("DeleteObj", KeyCode.KC_DELETE);

			if (kBindsFile.Open(dPath + "ZomBerryKeybinds.bin", FileMode.WRITE)) {
				ZomberryBase.Log( "ZomBerryConfig", "信息：已成功创建密钥绑定文件。" );
				kBindsFile.Write(keyBindList);
				kBindsFile.Close();
			}
		}
	}

	string InstFindAdmins() {
		string tmpStr = "";

		if (GetCLIParam("zbryInstallMode", tmpStr)) {
			if (tmpStr == "true") return FindAdmins();
		}
		return "";
	}

	bool InstInsertAdmin(string adminUID) {
		string tmpStr = "";

		if (GetCLIParam("zbryInstallMode", tmpStr)) {
			if (tmpStr == "true") {
				FileHandle adminsFile;
				tmpStr = FindAdmins();

				adminsFile = OpenFile(tmpStr + "admins.cfg", FileMode.APPEND);
				if (adminsFile != 0) {
					FPrintln(adminsFile, adminUID);
					ZomberryBase.Log( "ZomBerryConfig", "信息： 增加 " + adminUID + " 到管理员列表" );
					CloseFile(adminsFile);
					return true;
				}
			}
		}
		return false;
	}

	string InstCreateAdmins() {
		string tmpStr;

		if (GetCLIParam("zbryInstallMode", tmpStr)) {
			if (tmpStr == "true") {
				tmpStr = "$profile:\\ZomBerry\\";

				if (!FileExist(tmpStr)) MakeDirectory(tmpStr);
				if (!FileExist(tmpStr)) return "生成目录失败，请手动在配置文件目录中创建文件夹";

				if (!FileExist(tmpStr + "admins.cfg")) {
					FileHandle adminsFile = OpenFile(tmpStr + "admins.cfg", FileMode.WRITE);

					if (adminsFile != 0) {
						FPrintln(adminsFile, "YWrRTYsUNXUHr2ALuJGiTQ7nvnae8XcTxe3XvJ3Ay54=");
						ZomberryBase.Log( "ZomBerryConfig", "信息： 已成功创建管理员文件。" );
						CloseFile(adminsFile);
						return "OK";
					} else return "无法创建文件（权限不足？）";
				} else return "AlreadyExists";
			}
		}
		return "Wrong mode";
	}
};
