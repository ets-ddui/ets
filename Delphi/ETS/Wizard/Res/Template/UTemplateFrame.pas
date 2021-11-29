unit {UnitName};

interface

uses
  UFrameBase;

type
  T{ClassName} = class(TFrameBase)
  end;

implementation

\{$R *.dfm\}

initialization
  TFrameBase.RegFrame(T{ClassName});

end.
