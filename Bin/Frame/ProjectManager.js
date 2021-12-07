var g_Frame = {
    "__class__": "TDUIPanel",
    "__property__": {
        "Align": "alClient"
    },
    "__child__": [
        {
            "__class__": "TDUIPanel",
            "__property__": {
                "Align": "alTop",
                "Height": 25
            },
            "__child__": [
                {
                    "__class__": "TDUIButton",
                    "__property__": {
                        "Align": "alLeft",
                        "Enabled": false,
                        "Width": 25,
                        "Shape": {
                            "__property__": {
                                "Picture": {
                                    "__property__": {
                                        "SkinName": "SYSTEM[开启1,开启2]"
                                    }
                                },
                                "Width": 16,
                                "Height": 16
                            }
                        },
                        "OnClick": function () {
                            ShowLog("脚本窗口事件测试");
                        }
                    }
                },
                {
                    "__class__": "TETSComboBox",
                    "__property__": {
                        "Name": "cbToolChain",
                        "Align": "alLeft",
                        "Enabled": false,
                        "Width": 100,
                        "OnChange": DoToolChainChange
                    }
                }
            ]
        },
        {
            "__class__": "TETSTreeGrid",
            "__property__": {
                "Name": "tgFiles",
                "Align": "alLeft",
                "Width": 150
            }
        }
    ]
};

var g_sWorkSpace = "";
var g_objToolChain = undefined;

function DoToolChainChange() {
    var frmMain = GetFrame("");
    var tnProjNode = frmMain.FindChild("tgFiles").RootNode.GetItems(0);
    tnProjNode.Clear();

    var cbToolChain = frmMain.FindChild("cbToolChain");
    switch (cbToolChain.Text) {
        case "Make":
            g_objToolChain = Require("Frame/ToolChain/Make.js");
            break;
        case "GN":
            g_objToolChain = Require("Frame/ToolChain/GN.js");
            break;
        case "VC":
            g_objToolChain = Require("Frame/ToolChain/VC.js");
            break;
    }

    var arrTargets = g_objToolChain.ListTargets("");
    for (var i in arrTargets) {
        tnProjNode.AddChild(arrTargets[i], false);
    }
}

function InitFileList(p_This, p_sPath, p_iIndex, p_iCount) {
    g_sWorkSpace = "";
    g_objToolChain = undefined;

    //1.0 清理界面数据
    var frmMain = GetFrame("");
    var cbToolChain = frmMain.FindChild("cbToolChain");
    cbToolChain.Clear();
    var tnRootNode = frmMain.FindChild("tgFiles").RootNode;
    tnRootNode.Clear();

    //2.0 目录名默认当成工程名处理
    var arrProject = p_sPath.match(/^.*[\\\/]([^\\\/]*)$/);
    if (!arrProject) {
        return;
    }
    var sProjName = arrProject[1];

    //3.0 工具链测试
    if (Ets.FileExists([p_sPath, "makefile"].join("/"))) cbToolChain.AddData("Make");
    if (Ets.FileExists([p_sPath, "BUILD.gn"].join("/"))) cbToolChain.AddData("GN");
    if (Ets.FileExists([p_sPath, "/", sProjName, ".sln"].join(""))) cbToolChain.AddData("VC");
    if (0 == cbToolChain.ItemCount) return;

    //4.0 界面初始化
    g_sWorkSpace = p_sPath;
    tnRootNode.AddChild(sProjName, false);
    cbToolChain.Enabled = true;
    cbToolChain.ItemIndex = 1;
}

function Init() {
    CreateFrame("", g_Frame);

    var frmMain = GetFrame("");
    frmMain.AcceptDropFiles = true;
    frmMain.Bind("OnDropFiles", InitFileList);

    var tgFiles = frmMain.GetFrame("[0].tgFiles");
    var tcFirstColumn = tgFiles.Columns.GetItems(0);
    tcFirstColumn.Caption = '解决方案';
    tcFirstColumn.Percent = true;
}
