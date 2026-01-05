class CfgPatches
{
    class CoCo_Car
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts"
        };
    };
};

class CfgMods
{
    class CoCo_Car
    {
        dir = "CoCo_Car";
        picture = "";
        inputs = "CoCo_Car\\Scripts\\data\\inputs.xml";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "CoCo_Car";
        credits = "CoCo";
        author = "CoCo";
        authorID = "0";
        version = "1.0";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"CoCo_Car/Scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"CoCo_Car/Scripts/5_Mission"};
            };
        };
    };
};
