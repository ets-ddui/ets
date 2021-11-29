__this__ = this;

(function () {
    //1.0 基础功能定义
    function GetPlatform() {
        return {
            targetOS: "Win32",
            compiler: "Delphi2007",
            pluginPath: [".", "./Dll/Plugin.D"],
            jsPath: [".", "./Script/JScript", "./Lib/JScript/Lib"]
        };
    }

    function GetNakeName(p_sFileName) {
        var sResult = p_sFileName.replace(/^.*\//, "");
        return sResult.replace(/\.[^.]*$/, "");
    }

    function FindFile(p_lstPath, p_sFileName, p_sFileExt) {
        //1.0 绝对路径处理
        if (/^\w:/.test(p_sFileName)) { //本地绝对路径
            if (Ets.FileExists(p_sFileName)) {
                return p_sFileName;
            } else {
                return "";
            }
        } else if (/^\/\//.test(p_sFileName)) { //网络映射路径
            if (Ets.FileExists(p_sFileName)) {
                return p_sFileName;
            } else {
                return "";
            }
        }

        //2.0 相对路径处理
        var sFileName = p_sFileName;
        if (/^\//.test(sFileName)) { //去掉路径开头的“/”字符
            sFileName = sFileName.replace(/^\//, "");
        }

        for (var sPath in p_lstPath) {
            if (Ets.FileExists("%1/%2".Format(p_lstPath[sPath], sFileName))) {
                return "%1/%2".Format(p_lstPath[sPath], sFileName);
            } else if (Ets.FileExists("%1/%2%3".Format(p_lstPath[sPath], sFileName, p_sFileExt))) {
                return "%1/%2%3".Format(p_lstPath[sPath], sFileName, p_sFileExt);
            }
        }

        return "";
    }

    var g_iErrorLevel = 3;
    __this__.Debug = function (p_iErrorLevel, p_sMessage) {
        if (p_iErrorLevel >= g_iErrorLevel) {
            ShowLog(p_sMessage);
        }
    };

    __this__SetErrorLevel = function (p_iErrorLevel) {
        if (p_iErrorLevel < 0 || p_iErrorLevel > 20) {
            Debug(16, "错误级别不合法");
            return;
        }

        g_iErrorLevel = p_iErrorLevel;
    };

    __this__.ThrowError = function (p_sErrorMessage) {
        Debug(20, "(致命错误)%1".Format(p_sErrorMessage));
        throw p_sErrorMessage;
    }

    //2.0 Ets模块功能封装(将Ets中的功能复制到JScript的全局对象中)
    __modules__ = {__plugins__: {}, __codes__: {}};

    __this__.LoadPlugin = function (p_sFileName) {
        //1.0 查找文件的存在性
        var sFileName = FindFile(GetPlatform().pluginPath, p_sFileName.replace(/\\/g, "/"), ".dll");
        if ("" === sFileName) {
            ThrowError("插件不存在：%1".Format(p_sFileName));
        }

        //2.0 加载模块，并按全路径名缓存到全局对象中
        sFileName = sFileName.toLowerCase();
        if (!(sFileName in __modules__.__plugins__)) {
            __modules__.__plugins__[sFileName] = Ets.LoadPlugin(sFileName);
        }

        //3.0 按文件名注册加载的模块，当外部代码没有保存Require的返回值时，可以直接通过文件名访问模块的功能
        var sNakeName = GetNakeName(p_sFileName);
        if (!(sNakeName in __this__)) {
            __this__[sNakeName] = __modules__.__plugins__[sFileName];
        }

        return __modules__.__plugins__[sFileName];
    };

    __this__.Require = function (p_sFileName) {
        //1.0 查找文件的存在性
        var sFileName = FindFile(GetPlatform().jsPath, p_sFileName.replace(/\\/g, "/"), ".js");
        if ("" === sFileName) {
            ThrowError("模块不存在：%1".Format(p_sFileName));
        }

        //2.0 加载模块，并按全路径名缓存到全局对象中
        sFileName = sFileName.toLowerCase();
        if (!(sFileName in __modules__.__codes__)) {
            __modules__.__codes__[sFileName] = Ets.Require(sFileName);
        }

        //3.0 按文件名注册加载的模块，当外部代码没有保存Require的返回值时，可以直接通过文件名访问模块的功能
        var sNakeName = GetNakeName(p_sFileName);
        if (!(sNakeName in __this__)) {
            __this__[sNakeName] = __modules__.__codes__[sFileName];
        }

        return __modules__.__codes__[sFileName];
    };

    __this__.CreateWorker = function (p_iThreadCount, p_sScriptFile, p_funcGetEntry) {
        //1.0 查找脚本的存在性
        var sFileName = FindFile(GetPlatform().jsPath, p_sScriptFile.replace(/\\/g, "/"), ".js");
        if ("" === sFileName) {
            ThrowError("模块不存在：%1".Format(p_sScriptFile));
        }

        //2.0 创建线程
        LoadPlugin("Ext");
        var thd = Ext.GetThreadContainer();
        if (p_funcGetEntry instanceof Function) {
            for (var i = 0; i < p_iThreadCount; ++i) {
                thd.AddThread(sFileName, p_funcGetEntry(i));
            }
        } else {
            for (var i = 0; i < p_iThreadCount; ++i) {
                thd.AddThread(sFileName, p_funcGetEntry);
            }
        }

        return thd;
    };

    __this__.Sleep = function (p_iMiliSeconds) {
        Ets.Sleep(p_iMiliSeconds);
    };

    __this__.ShowLog = function (p_sFormat) {
        Ets.ShowLog(p_sFormat);
    };

    __this__.Log = function (p_sFormat, p_sFileName) {
        if (typeof p_sFileName === "undefined")
            p_sFileName = './Log/1.log';
        Ets.Log(p_sFormat, p_sFileName);
    };

    __this__.GetSetting = function (p_sName) {
        return Ets.GetSetting(p_sName);
    };

    __this__.Stop = function () {
        return Ets.GetStop;
    };

    __this__.TicketCount = function () {
        return Ets.GetTicketCount;
    };

    __this__.GetFrame = function (p_sPath) {
        return Ets.GetFrame(p_sPath);
    };

    //p_Frame：父控件，可输入路径名或控件对象指针
    //p_Template：子控件创建模板，可输入字符串或json对象
    __this__.CreateFrame = function (p_Frame, p_Template) {
        if (typeof p_Frame === "string") {
            p_Frame = GetFrame(p_Frame)
        }

        function O2S(p_Value) {
            var arrResult = [];
            arrResult.push("{");

            var bAppendComma = false;
            for (var strName in p_Value) {
                if (p_Value[strName] instanceof Function) {
                    continue;
                }

                arrResult.push(bAppendComma ? "," : "");
                arrResult.push('"');
                arrResult.push(strName);
                arrResult.push('":');
                arrResult.push(V2S(p_Value[strName]));

                bAppendComma = true;
            }

            arrResult.push("}");

            return arrResult.join("");
        }
        function A2S(p_Value) {
            var arrResult = [];
            arrResult.push("[");

            var bAppendComma = false;
            for (var iIndex in p_Value) {
                arrResult.push(bAppendComma ? "," : "");
                arrResult.push(V2S(p_Value[iIndex]));

                bAppendComma = true;
            }

            arrResult.push("]");

            return arrResult.join("");
        }
        function V2S(p_Value) {
            if (p_Value instanceof Array) {
                return A2S(p_Value);
            } else if (p_Value instanceof Object) {
                return O2S(p_Value);
            } else if (typeof p_Value === "string") {
                return ['"', p_Value, '"'].join("");
            } else {
                return p_Value;
            }
        }
        function Bind(p_Value, p_Template) {
            if ("__property__" in p_Template) {
                for (var sName in p_Template["__property__"]) {
                    if (p_Template["__property__"][sName] instanceof Function) {
                        p_Value.Bind(sName, p_Template["__property__"][sName]);
                    }
                }
            }

            if ("__child__" in p_Template) {
                for (var i in p_Template["__child__"]) {
                    Bind(p_Value.GetFrame("[%1]".Format(i)), p_Template["__child__"][i])
                }
            }
        }

        var frmResult = p_Frame.CreateFrame(V2S(p_Template));
        Bind(frmResult, p_Template);

        return frmResult;
    };

    //3.0 性能调试相关
    var g_objTimer = {};
    __this__.Profile = function (p_sID, p_funcExecute) {
        if (!(p_sID in g_objTimer)) {
            g_objTimer[p_sID] = {msec: 0, succ: 0, fail: 0};
        }

        var iTime = TicketCount();
        var bResult = p_funcExecute();
        iTime = TicketCount() - iTime;
        g_objTimer[p_sID].msec += iTime;
        if (bResult) {
            g_objTimer[p_sID].succ += 1;
        } else {
            g_objTimer[p_sID].fail += 1;
        }
    };

    __this__.PrintTimer = function (p_sDescription) {
        var sResult = p_sDescription;
        if (typeof sResult === "undefined") sResult = "性能统计结果";

        for (var sID in g_objTimer) {
            var obj = g_objTimer[sID];
            if (0 != obj.succ + obj.fail) {
                sResult += ("%n  %1"
                    + "%n    平均耗时: %2(ms) 总耗时: %3(ms)"
                    + "%n    成功: %4 失败: %5").Format(
                    sID,
                    obj.msec / (obj.succ + obj.fail), obj.msec,
                    obj.succ, obj.fail);
            }
        }

        Debug(5, sResult);
    };

    //4.0 JScript基础对象功能扩展
    //调用样例
    //'测试样例 %1 %2'.Format('测试1', '测试2');
    //备注：此函数频繁调用会严重影响性能
    String.prototype.Format = function () {
        var m_Arguments = arguments;

        return this.replace(/%(\d+|%|n)/g, function (p_sMatchString, p_sIndex)
            {
                if ("%" == p_sIndex) {
                    return "%"; //“%”本身
                } else if ("n" == p_sIndex) {
                    return String.fromCharCode(13, 10); //换行
                } else {
                    return m_Arguments[Number(p_sIndex) - 1]; //参数替换
                }
            });
    };

    String.prototype.Trim = function () {
        return this.replace(/^\s+/, "").replace(/\s+$/, "");
    };
})();
