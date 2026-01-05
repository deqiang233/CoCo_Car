class VPPButton: ScriptedWidgetEventHandler
{
	ButtonWidget Button;
	string 		 ButtonType;
	string 	     ButtonName;
	string 	     ButtonIcon;
	string       ButtonDescription;
	private      Widget m_Root; //root
	private bool m_PermissionActive;
	
	void VPPButton(Widget btn, string btnType, string name, string icon, string description)
	{
		VPPAdminHud.m_OnPermissionsChanged.Insert(this.OnPermissionChange);

		m_Root    		  = btn;
		Button            = ButtonWidget.Cast(btn.FindAnyWidget("Button"));
		ButtonType		  = btnType;
		ButtonName        = name;
		ButtonIcon        = icon;
		ButtonDescription = description;
		ImageWidget.Cast(btn.FindAnyWidget("Image")).LoadImageFile(0, icon);
		TextWidget.Cast(btn.FindAnyWidget("ButtonName")).SetText(name);

		// Set unique color for each button based on type
		SetButtonColor(btnType);

		ToolTipHandler toolTip;
		Button.GetScript(toolTip);
		toolTip.SetTitle(name+":");
		toolTip.SetContentText(description);
		toolTip.SetLeftSide(true);

		btn.SetHandler(this);
		btn.Show(false);
	}

	void SetButtonColor(string btnType)
	{
		int r, g, b, a;
		a = 255; // Alpha always 255 (fully opaque)
		
		switch (btnType)
		{
			case "MenuPlayerManager":
				r = 30; g = 144; b = 255; // 蓝色 - DodgerBlue
				break;
			case "MenuItemManager":
				r = 50; g = 205; b = 50; // 绿色 - LimeGreen
				break;
			case "MenuTeleportManager":
				r = 147; g = 112; b = 219; // 紫色 - MediumPurple
				break;
			case "EspToolsMenu":
				r = 0; g = 206; b = 209; // 青色 - DarkTurquoise
				break;
			case "MenuCommandsConsole":
				r = 255; g = 140; b = 0; // 橙色 - DarkOrange
				break;
			case "MenuServerManager":
				r = 25; g = 25; b = 112; // 深蓝色 - MidnightBlue
				break;
			case "MenuWeatherManager":
				r = 135; g = 206; b = 250; // 天蓝色 - LightSkyBlue
				break;
			case "MenuObjectManager":
				r = 255; g = 215; b = 0; // 金黄色 - Gold
				break;
			case "MenuBansManager":
				r = 220; g = 20; b = 60; // 深红色 - Crimson
				break;
			case "MenuPermissionsEditor":
				r = 255; g = 105; b = 180; // 粉色 - HotPink
				break;
			case "MenuWebHooks":
				r = 34; g = 139; b = 34; // 深绿色 - ForestGreen
				break;
			case "MenuXMLEditor":
				r = 199; g = 21; b = 133; // 紫红色 - MediumVioletRed
				break;
			default:
				r = 255; g = 0; b = 0; // 默认红色
				break;
		}
		
		Button.SetColor(ARGB(a, r, g, b));
	}

	void OnWidgetScriptInit(Widget w)
	{
		w.SetHandler(this);
	}

	void OnPermissionChange(map<string,bool> permissions)
	{
		if (!permissions || permissions.Count() <= 0)
			return;

		permissions.Find(ButtonType, m_PermissionActive);
		m_Root.Show(m_PermissionActive);
		DestroySubMenu(); //close up menu related to this button
	}

	void DestroySubMenu()
	{
		VPPAdminHud root = VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud));
			if (!root)
				return;

		AdminHudSubMenu menu = root.GetSubMenuByType(ButtonType.ToType());
		if (menu && !m_PermissionActive)
		{
			menu.HideSubMenu();
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == Button)
		{
			VPPAdminHud root = VPPAdminHud.Cast(GetVPPUIManager().GetMenuByType(VPPAdminHud));
			if (!root)
				return false;

			typename btnType = ButtonType.ToType();
			AdminHudSubMenu menu = root.GetSubMenuByType(btnType);
			if (menu)
			{
				if (menu.IsSubMenuVisible())
				{
					menu.HideSubMenu();
				}else if (m_PermissionActive){
					menu.ShowSubMenu();
				}
				return true;
			}

			if (m_PermissionActive){
				root.CreateSubMenu(btnType);
			}
			return true;
		}
		return super.OnClick(w, x, y, button);
	}
	
	void ~VPPButton()
	{
		if (m_Root != null)
			m_Root.Unlink();

		VPPAdminHud.m_OnPermissionsChanged.Remove(this.OnPermissionChange);
	}
};