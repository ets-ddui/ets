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
                                        "SkinName": "SYSTEM[���1,���2]"
                                    }
                                },
                                "Width": 16,
                                "Height": 16
                            }
                        },
                        "OnClick": function () {
                            ShowLog("�ű������¼�����");
                        }
                    }
                }
            ]
        },
        {
            "__class__": "TETSTreeGrid",
            "__property__": {
                "Name": "tgLog",
                "Align": "alClient",
                "Columns": [
                    {
                        "__property__": {
                            "Caption": "���",
                            "Width": 50
                        }
                    },
                    {
                        "__property__": {
                            "Caption": "��־",
                            "Percent": true
                        }
                    }
                ]
            }
        }
    ]
};

var g_LogManager = undefined;
var g_iCallBack = -1;

function DoCallBack(p_This, p_sMessage) {
    if (!g_LogManager) return;

    var tnRoot = GetFrame("").FindChild("tgLog").RootNode;
    var tnItem = tnRoot.AddChild(tnRoot.ChildCount, false);
}

function Init() {
    CreateFrame("", g_Frame);

    var frmMain = GetFrame("");
    frmMain.Bind("OnETSAfterNotify", function (p_This, p_NotifyType) {
        if ("ntToggle" != p_NotifyType) return;

        p_This.Align = "alBottom";
        p_This.Height = 200;

        if (p_This.Visible) {
            g_LogManager = Ets.GetService('LogManager');
            g_iCallBack = g_LogManager.RegistCallBack(DoCallBack);
        } else if (g_LogManager) {
            g_LogManager.UnRegistCallBack(g_iCallBack);
            g_LogManager = undefined;
        }
    });
}
