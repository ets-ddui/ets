var g_Frame = {
    "__class__": "TDUIPanel",
    "__property__": {
        "Align": "alClient"
    },
    "__child__": [
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
                    ShowLog("�ű������¼�����");
                }
            }
        }
    ]
};

var g_sWorkSpace = "";

function SelectToolChain(p_sToolChain) {
    switch (p_sToolChain) {
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

function InitFileList(p_This, p_sPath, p_iIndex, p_iCount) {
    //1.0 �����������
    var tgFiles = GetFrame("").GetFrame("[0].tgFiles");
    var tnRootNode = tgFiles.RootNode;
    tnRootNode.Clear();
    g_sWorkSpace = "";

    //2.0 Ŀ¼��Ĭ�ϵ��ɹ���������
    var arrProject = p_sPath.match(/^.*[\\\/]([^\\\/]*)$/);
    if (!arrProject) {
        return;
    }
    var sProjName = arrProject[1];

    //3.0 ����������
    var arrToolChain = [];

    if (Ets.FileExists([p_sPath, "makefile"].join("/"))) arrToolChain.push("Make");
    if (Ets.FileExists([p_sPath, "BUILD.gn"].join("/"))) arrToolChain.push("GN");
    if (Ets.FileExists([p_sPath, "/", sProjName, ".sln"].join(""))) arrToolChain.push("VC");
    if (!arrToolChain) return;

    //4.0 �����ʼ��
    g_sWorkSpace = p_sPath;
    var tnProject = tnRootNode.AddChild(sProjName, false);
}

function Init() {
    CreateFrame("", g_Frame);

    var frmMain = GetFrame("");
    frmMain.AcceptDropFiles = true;
    frmMain.Bind("OnDropFiles", InitFileList);

    var tgFiles = frmMain.GetFrame("[0].tgFiles");
    var tcFirstColumn = tgFiles.Columns.GetItems(0);
    tcFirstColumn.Caption = '�������';
    tcFirstColumn.Percent = true;
}
