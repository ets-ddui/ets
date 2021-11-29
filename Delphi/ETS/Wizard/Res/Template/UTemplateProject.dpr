library {Library};

\{$i UConfigure.inc\}

uses
  \{$IFDEF LAZARUS\}
    \{$IFDEF UNIX\}
      \{$IFDEF UseCThreads\}
      cthreads,
      \{$ENDIF\}
    \{$ENDIF\}
    Interfaces, // this includes the LCL widgetset
  \{$ENDIF\}
  SysUtils,
  Classes,
  UAppInit;

{Export}

begin
  TAppInit.Init(True);
end.
