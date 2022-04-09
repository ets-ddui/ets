{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
            https://gitee.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
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
