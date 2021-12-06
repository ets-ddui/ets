(function SelectToolChain(p_This) {
    Require("IO");
    var c_sExec = "Bin/GN/gn.exe";

    p_This.Configure = function (p_sProjectRootPath) {
    }

    p_This.ListTargets = function (p_sParentTarget) {
        var arrResult = [];

        var gn = IO.GetProcess();
        gn.Start(c_sExec, "", "", function (p_sData) {
            ShowLog(p_sData);
            arrResult.push(p_sData);
        });
        gn.Close();
        IO.Run(false);

        return arrResult;
    }
}(this));
