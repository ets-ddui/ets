unit UModule;

\{$i UConfigure.inc\}

interface

uses
  UInterface;

function GetModule(AManager: IManager; var AResult: IDispatch): HRESULT; stdcall;

implementation

uses
  Windows, Classes, SyncObjs, SysUtils, UObjComAutoEh, UModuleBase;

type
  \{$METHODINFO ON\}
  TModule = class(TModuleBase)
  published
    //这里添加功能模块
    //function GetSample: IDispatch;
  end;
  \{$METHODINFO OFF\}

function GetModule(AManager: IManager; var AResult: IDispatch): HRESULT;
begin
  SetManager(AManager);
  AResult := TModule.Create;

  if Assigned(AResult) then
    Result := S_OK
  else
    Result := E_OUTOFMEMORY;
end;

\{ TModule \}

//function TModule.GetSample: IDispatch;
//begin
//  Result := WrapperObject(TSample.Create);
//end;

end.
