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
                                        "SkinName": "SYSTEM[����1,����2]"
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

function Init() {
    CreateFrame("", g_Frame);

    var frmMain = GetFrame("");
    frmMain.Bind("OnETSAfterNotify", function (p_This, p_NotifyType) {
        if ("ntToggle" != p_NotifyType) return;

        var frmMain = GetFrame("");
        frmMain.Align = "alBottom";
        frmMain.Height = 200;
    });
}
