@set path=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin;D:\DevTool\VS2010\VC\bin;D:\DevTool\VS2010\Common7\IDE

@mkdir temp

@midl /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include" /I "%~dp0." ^
	/dlldata "./temp/dlldata.c" ^
	/iid "./temp/%~n1_i.c" ^
	/proxy "./temp/%~n1_p.c" ^
	%~1

@pause