unit UFrame;

\{$i UConfigure.inc\}

interface

uses
  Windows, Classes, UInterface, UFrameBase;

function GetFrame(AManager: IManager; AID: String; AOwner: TComponent; var AResult: IChild): HRESULT; stdcall;

implementation

function GetFrame(AManager: IManager; AID: String; AOwner: TComponent; var AResult: IChild): HRESULT;
begin
  SetManager(AManager);
  AResult := TFrameBase.GetFrame(AID, AOwner);

  if Assigned(AResult) then
    Result := S_OK
  else
    Result := E_OUTOFMEMORY;
end;

end.
