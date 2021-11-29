LoadPlugin('IO');

function main(p_iCount, p_iIndex) {
    //var cli = IO.GetTCPClient("www.baidu.com", "80");
    var cli = IO.GetProcess();
    cli.Start("cmd", "", function (p_sData) {
        ShowLog(p_sData);
    });
    cli.Write("exit\n");
    IO.Run();
}
