class PermissionBox : VPPPlayerTemplate
{
	private CheckBoxWidget m_PermCheckBox;
	private string 		   m_PermName;
	void PermissionBox(GridSpacerWidget grid, string name)
	{
		m_LayoutPath   = VPPATUIConstants.VPPPermissionBox;
		m_EntryBox     = GetGame().GetWorkspace().CreateWidgets( m_LayoutPath, grid);
		m_PermCheckBox = CheckBoxWidget.Cast(m_EntryBox.FindAnyWidget("CheckBox"));
		m_PermName     = name;
		
		// Localize the permission name - replace colons with underscores for string table lookup
		string sanitizedName = name;
		sanitizedName.Replace(":", "_");
		string localizedKey = "#VSTR_PERM_" + sanitizedName;
		m_PermCheckBox.SetText(localizedKey);
	}
	
	void ~PermissionBox()
	{
		if (m_EntryBox != null)
		m_EntryBox.Unlink();
	}
	
	void SetChecked(bool state)
	{
		m_PermCheckBox.SetChecked(state);
	}
	
	bool IsChecked()
	{
		return m_PermCheckBox.IsChecked();
	}
	
	string GetName()
	{
		return m_PermName;
	}
}