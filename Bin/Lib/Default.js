Require('IO');

function main(p_iCount, p_iIndex) {
    var cli = IO.GetProcess();
    cli.Start("cmd", "", "", function (p_sData) {
        ShowLog(p_sData);
    });
    cli.Close();
    IO.Run(false);
}
