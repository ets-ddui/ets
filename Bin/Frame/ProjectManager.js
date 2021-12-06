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
                        }
                    }
                },
                {
                    "__class__": "TETSComboBox",
                    "__property__": {
                        "Align": "alLeft",
                        "Width": "100"
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
        },
        {
            "__class__": "TDUIButton",
            "__property__": {
                "Caption": "Test",
                "Left": 500,
                "Top": 10,
                "Width": 100,
                "Height": 25,
                "OnClick": function () {
                    ShowLog("脚本窗口事件测试");
                }
            }
        }
    ]
};

var g_sWorkSpace = "";
var g_sToolChain = "";

function SelectToolChain() {
    switch (g_sToolChain) {
        case "Make":
            return Require("Frame/ToolChain/Make.js");
        case "GN":
            return Require("Frame/ToolChain/GN.js");
        case "VC":
            return Require("Frame/ToolChain/VC.js");
        default:
            return undefined;
    }
}

function DoToolChainChanged() {
    var tgFiles = GetFrame("").GetFrame("[0].tgFiles");
    var tnRootNode = tgFiles.RootNode;

    var objToolChain = SelectToolChain();
    var arrTargets = objToolChain.ListTargets("");
}

function InitFileList(p_This, p_sPath, p_iIndex, p_iCount) {
    //1.0 清理界面数据
    var tgFiles = GetFrame("").GetFrame("[0].tgFiles");
    var tnRootNode = tgFiles.RootNode;
    tnRootNode.Clear();
    g_sWorkSpace = "";
    g_sToolChain = "";

    //2.0 目录名默认当成工程名处理
    var arrProject = p_sPath.match(/^.*[\\\/]([^\\\/]*)$/);
    if (!arrProject) {
        return;
    }
    var sProjName = arrProject[1];

    //3.0 工具链测试
    var arrToolChain = [];

    if (Ets.FileExists([p_sPath, "makefile"].join("/"))) arrToolChain.push("Make");
    if (Ets.FileExists([p_sPath, "BUILD.gn"].join("/"))) arrToolChain.push("GN");
    if (Ets.FileExists([p_sPath, "/", sProjName, ".sln"].join(""))) arrToolChain.push("VC");
    if (!arrToolChain) return;

    //4.0 界面初始化
    g_sWorkSpace = p_sPath;
    g_sToolChain = arrToolChain[0];
    var tnProject = tnRootNode.AddChild(sProjName, false);
    DoToolChainChanged();
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
