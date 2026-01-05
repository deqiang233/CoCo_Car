class ZomberryCmdAPI {
	protected ref ZBerryCategoryArray m_oCategoryList = new ZBerryCategoryArray;
	protected int m_lastFuncId = 0;

	void ZomberryCmdAPI() {

	}

	void AddCategory(string catName, int funcColor) {
		if (m_oCategoryList.Contains(catName)) {
			ZomberryBase.Log( "ZomBerryCmdAPI", "警告：无法添加类别 " + catName + ", 同名类别已存在。");
			return;
		}

		ref ZBerryCategory catParam = new ZBerryCategory(catName, funcColor);

		m_oCategoryList.Insert(catParam);
		ZomberryBase.Log( "ZomBerryCmdAPI", "信息： 新增类别 " + catName);
	}

	void AddCommand(string dispName, string funcName, Class instance, string catName, bool onTarget = true, autoptr ZBerryFuncParamArray funcParams = NULL) {
		if (m_oCategoryList.Contains(catName)) {
			if (m_oCategoryList.GetByName(catName).GetByName(funcName) != NULL) {
				ZomberryBase.Log( "ZomBerryCmdAPI", "警告：添加 " + funcName + ", 但同名函数已经存在。");
			}

			int funcColor = m_oCategoryList.GetByName(catName).GetColor();

			if (!funcParams) funcParams = new ZBerryFuncParamArray;

			ref ZBerryFunction funcData = new ZBerryFunction(m_lastFuncId, dispName, funcName, catName, instance, funcColor, onTarget, funcParams);
			++m_lastFuncId;

			m_oCategoryList.GetByName(catName).Insert(funcData);

			ZomberryBase.Log( "ZomBerryCmdAPI", "信息： 附加功能 " + funcName + " 分类 " + catName);
		} else {
			ZomberryBase.Log( "ZomBerryCmdAPI", "警告：未知类别 " + catName + ", 无法添加函数 " + funcName);
		}
	}

	ZBerryFunction GetFunc(int funcId) {
		for (int i = 0; i < m_oCategoryList.Count(); ++i) {
			if (m_oCategoryList.Get(i).IsAvailable(funcId)) {
				return m_oCategoryList.Get(i).Get(funcId);
			}
		}

		return NULL;
	}

	ref ZBerryCategoryArray GetList() {
		return m_oCategoryList;
	}

	void Debug() {
		ZomberryBase.Log( "ZomBerryCmdAPIDbg", "将整个命令列表记录到 script.log");
		for (int i = 0; i < m_oCategoryList.Count(); ++i) { //Full list breakdown
			ref ZBerryCategory catEntry = m_oCategoryList.Get(i);
			catEntry.Debug();
		}
	}
};
