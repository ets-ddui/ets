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
                                        "SkinName": "SYSTEM[????1,????2]"
                                    }
                                },
                                "Width": 16,
                                "Height": 16
                            }
                        },
                        "OnClick": function () {
                            if (g_LogManager) g_LogManager.Clear;
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
                            "Caption": "????",
                            "Width": 50
                        }
                    },
                    {
                        "__property__": {
                            "Caption": "??־",
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
var g_iLastCount = 0;

function DoCallBack(p_This, p_sMessage) {
    if (!g_LogManager) return;

    var tnRoot = GetFrame("").FindChild("tgLog").RootNode;

    if (p_sMessage == "LOG_CLEAR") {
        tnRoot.Clear;
        g_iLastCount = 0;
    } else {
        var iCount = g_LogManager.Count;
        for (var i = g_iLastCount; i < iCount; ++i) {
            var tnItem = tnRoot.AddChild(i + 1, false);
            tnItem.SetCell(2, g_LogManager.GetLog(i));
        }
        g_iLastCount = iCount;
    }
}

function Init() {
    CreateFrame("", g_Frame);

    var frmMain = GetFrame("");
    frmMain.Bind("OnETSAfterNotify", function (p_This, p_NotifyType) {
        if ("ntToggle" != p_NotifyType) return;

        if (p_This.Visible) {
            p_This.Align = "alBottom";
            p_This.Height = 200;

            g_LogManager = Ets.GetService('LogManager');
            g_iCallBack = g_LogManager.RegistCallBack(DoCallBack);
        } else if (g_LogManager) {
            g_LogManager.UnRegistCallBack(g_iCallBack);
            g_LogManager = undefined;
            g_iLastCount = 0;
        }
    });
}
