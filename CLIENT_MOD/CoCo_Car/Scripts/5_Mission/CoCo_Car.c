class CoCoCarMenuWidget extends UIScriptedMenu
{
    protected TextListboxWidget m_BoatList;
    protected ButtonWidget m_CloseButton;
    protected autoptr array<string> m_BoatInfo;

    void CoCoCarMenuWidget()
    {
        m_BoatInfo = new array<string>;
    }

    void ~CoCoCarMenuWidget()
    {
        if (m_BoatInfo)
        {
            delete m_BoatInfo;
        }
    }

    override Widget Init()
    {
        Print("[CoCo_Car] - 界面Init开始1");
        string layoutPath = "CoCo_Car\\Scripts\\5_Mission\\layouts\\CoCoCarMenu.layout";
        Print("[CoCo_Car] - 尝试加载布局文件: " + layoutPath);
        layoutRoot = GetGame().GetWorkspace().CreateWidgets(layoutPath);

        if (!layoutRoot)
        {
            Print("[CoCo_Car] - 错误：无法创建界面布局，layoutRoot为空");
            return null;
        }

        Print("[CoCo_Car] - 界面布局已创建，layoutRoot: " + layoutRoot);
        Print("[CoCo_Car] - layoutRoot 类型: " + layoutRoot.Type());

        // 获取位置和大小信息
        float posX, posY, sizeX, sizeY;
        layoutRoot.GetPos(posX, posY);
        layoutRoot.GetSize(sizeX, sizeY);
        Print("[CoCo_Car] - layoutRoot 位置: (" + posX + ", " + posY + "), 大小: (" + sizeX + ", " + sizeY + ")");

        m_BoatList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("BoatList"));
        m_CloseButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));

        Print("[CoCo_Car] - BoatList 找到: " + m_BoatList);
        Print("[CoCo_Car] - CloseButton 找到: " + m_CloseButton);

        if (!m_BoatList || !m_CloseButton)
        {
            Print("[CoCo_Car] - 错误：无法找到界面控件");
            return layoutRoot;
        }

        Print("[CoCo_Car] - 界面控件已找到");
        WidgetEventHandler.GetInstance().RegisterOnMouseButtonUp(m_CloseButton, this, "OnClick");

        // 文字颜色已在layout文件中设置为白色

        RefreshBoatList();
        Print("[CoCo_Car] - 界面Init完成");
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        Print("[CoCo_Car] - 界面OnShow被调用");
        // 移除全屏模糊效果
        GetGame().GetUIManager().ShowCursor(true);
        GetGame().GetInput().ChangeGameFocus(1);
        RefreshBoatList();
        Print("[CoCo_Car] - 界面显示设置完成");
    }

    override void OnHide()
    {
        super.OnHide();
        PPEffects.SetBlurMenu(0);
        GetGame().GetUIManager().ShowCursor(false);
        GetGame().GetInput().ResetGameFocus();
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        super.OnClick(w, x, y, button);

        if (w == m_CloseButton)
        {
            CloseMenu();
            return true;
        }

        return false;
    }


    void CloseMenu()
    {
        GetGame().GetUIManager().HideScriptedMenu(this);
    }

    void RefreshBoatList()
    {
        Print("[CoCo_Car] - RefreshBoatList 开始执行");
        if (!m_BoatList)
        {
            Print("[CoCo_Car] - m_BoatList 为空");
            return;
        }

        m_BoatList.ClearItems();
        Print("[CoCo_Car] - 已清空列表项");

        // 获取当前玩家GUID
        string currentPlayerGUID = "";
        PlayerIdentity identity = GetGame().GetPlayer().GetIdentity();
        if (identity)
        {
            currentPlayerGUID = identity.GetPlainId();
        }
        Print("[CoCo_Car] - 当前玩家GUID: " + currentPlayerGUID);

        m_BoatList.AddItem("=== CoCo_Car 船只管理系统 ===", NULL, 0);
        m_BoatList.AddItem("", NULL, 0);
        Print("[CoCo_Car] - 已添加标题");

        if (currentPlayerGUID == "")
        {
            m_BoatList.AddItem("错误：无法获取玩家身份", NULL, 0);
            Print("[CoCo_Car] - 无法获取玩家身份");
            return;
        }

        m_BoatList.AddItem("当前玩家: " + currentPlayerGUID, NULL, 0);
        m_BoatList.AddItem("", NULL, 0);
        Print("[CoCo_Car] - 已添加玩家信息");

        // 尝试读取当前玩家的PlayerData
        string playerFilePath = "$profile:VehicleSystem_V2/PlayerData/" + currentPlayerGUID + ".json";
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

                // 简单的JSON解析
                int ownedCount = 0;
                if (fileContent.IndexOf("\"m_OwerHashList\": [") != -1)
                {
                    m_BoatList.AddItem("您的船只绑定记录:", NULL, 0);

                    // 这里简化显示，实际应该解析哈希列表
                    m_BoatList.AddItem("- 已绑定船只类型将在车库中显示", NULL, 0);
                    ownedCount = 1; // 简化处理
                }
                else
                {
                    m_BoatList.AddItem("暂无船只绑定记录", NULL, 0);
                }
            }
        }
        else
        {
            m_BoatList.AddItem("暂无船只绑定记录", NULL, 0);
        }

        m_BoatList.AddItem("", NULL, 0);
        m_BoatList.AddItem("支持的船只类型:", NULL, 0);
        m_BoatList.AddItem("- RFWC_DragBoat_Black (黑色拖船)", NULL, 0);
        m_BoatList.AddItem("- RFWC_Zodiac (救生艇)", NULL, 0);
        m_BoatList.AddItem("", NULL, 0);
        m_BoatList.AddItem("操作说明:", NULL, 0);
        m_BoatList.AddItem("- 启动船只即可自动绑定", NULL, 0);
        m_BoatList.AddItem("- 车库刷出的船只自动继承绑定", NULL, 0);
        m_BoatList.AddItem("- 按分号键(;)关闭此界面", NULL, 0);
    }
};

modded class MissionGameplay
{
    protected ref CoCoCarMenuWidget m_CoCoCarMenu;

    override void OnInit()
    {
        super.OnInit();
        Print("[CoCo_Car] - Mission script loaded successfully");

        // 初始化界面
        if (GetGame().IsClient())
        {
            m_CoCoCarMenu = new CoCoCarMenuWidget();
            Print("[CoCo_Car] - 界面已初始化");
        }
    }


    override void OnKeyPress(int key)
    {
        super.OnKeyPress(key);
        Print("[CoCo_Car] - key"+key);
        if (!GetGame().IsClient())
        {
            return;
        }

        // ; 键：打开/关闭船只管理界面
        if (key == KeyCode.KC_SEMICOLON)
        {
            // 如果界面已打开，则关闭
            if (m_CoCoCarMenu && m_CoCoCarMenu.GetLayoutRoot() && m_CoCoCarMenu.GetLayoutRoot().IsVisible())
            {
                UIManager ui = GetGame().GetUIManager();
                if (ui)
                {
                    ui.HideScriptedMenu(m_CoCoCarMenu);
                    Print("[CoCo_Car] - 界面已关闭");
                }
            }
            else
            {
                // 尝试打开界面
                TryOpenCoCoCarMenu();
            }
            return;
        }

        // 如果船只管理界面已打开，让界面处理其他按键
        if (m_CoCoCarMenu && m_CoCoCarMenu.GetLayoutRoot() && m_CoCoCarMenu.GetLayoutRoot().IsVisible())
        {
            // 这里可以让界面处理ESC等按键
            return;
        }
    }

    private void TryOpenCoCoCarMenu()
    {
        Print("[CoCo_Car] - TryOpenCoCoCarMenu 被调用");
        UIManager ui = GetGame().GetUIManager();
        if (!ui)
        {
            Print("[CoCo_Car] - UIManager 为空");
            return;
        }

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
        {
            Print("[CoCo_Car] - Player 为空");
            return;
        }

        UIScriptedMenu activeMenu = ui.GetMenu();
        Print("[CoCo_Car] - 当前活跃菜单: " + activeMenu);

        // 已经打开船只管理界面，则关闭它
        if (activeMenu && activeMenu.Type() == CoCoCarMenuWidget)
        {
            Print("[CoCo_Car] - 检测到已打开的界面，正在关闭");
            ui.HideScriptedMenu(activeMenu);
            return;
        }

        // 有其他界面则提示先关闭
        if (activeMenu)
        {
            Print("[CoCo_Car] - 有其他活跃界面，显示提示");
            player.MessageImportant("请先关闭当前界面");
            return;
        }

        // 打开船只管理界面
        if (!m_CoCoCarMenu)
        {
            Print("[CoCo_Car] - 创建新的界面实例");
            m_CoCoCarMenu = new CoCoCarMenuWidget();
        }

        if (!m_CoCoCarMenu)
        {
            Print("[CoCo_Car] - 界面实例创建失败");
            return;
        }

        Print("[CoCo_Car] - 正在显示界面");
        ui.ShowScriptedMenu(m_CoCoCarMenu, NULL);
        Print("[CoCo_Car] - 界面显示命令已执行");
    }

    CoCoCarMenuWidget GetCoCoCarMenu()
    {
        return m_CoCoCarMenu;
    }
};
