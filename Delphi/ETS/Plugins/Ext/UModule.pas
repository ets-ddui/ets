{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)，可扩展工具集。

  本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
  发布此库的目的是希望其有用，但不做任何保证。
  如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

  开源地址: https://github.com/ets-ddui/ets
  开源协议: The MIT License (MIT)
  作者邮箱: xinghun87@163.com
  官方博客：https://blog.csdn.net/xinghun61
}
unit UModule;

{$i UConfigure.inc}

interface

uses
  UInterface;

function GetModule(AManager: IManager; var AResult: IDispatch): HRESULT; stdcall;

implementation

uses
  Classes, Windows, SyncObjs, SysUtils, UModuleBase,
  UQueueManager, UScriptThread, UFileReader, UCmd, UTls, UTool;

type
  {$METHODINFO ON}
  TModule = class(TModuleBase)
  published
    function GetQueue(AQueueName: String): IDispatch;
    function GetThreadContainer: IDispatch;
    function GetFileReader: IDispatch;
    function RunCmd(AFileName: String): IDispatch;
  end;
  {$METHODINFO OFF}

function GetModule(AManager: IManager; var AResult: IDispatch): HRESULT; stdcall;
begin
  SetManager(AManager);
  AResult := TModule.Create;

  if Assigned(AResult) then
    Result := S_OK
  else
    Result := E_OUTOFMEMORY;
end;

{ TModule }

function TModule.GetFileReader: IDispatch;
begin
  Result := WrapperObject(TFileReader.Create);
end;

function TModule.GetQueue(AQueueName: String): IDispatch;
begin
  Result := TManager.GetQueue(AQueueName);
end;

function TModule.GetThreadContainer: IDispatch;
begin
  Result := GetScriptThreadContainer;
end;

function TModule.RunCmd(AFileName: String): IDispatch;
begin
  Result := WrapperObject(TCmd.Create(AFileName));
end;

procedure DllMain(AReason: Integer);
begin
  if AReason = DLL_THREAD_DETACH then
    UTls.DeleteCurrentTls;
end;

initialization
  DllProc := DllMain;

end.
