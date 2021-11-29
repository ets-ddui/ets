function ToObject(p_sJson) {
    if (p_sJson instanceof String || typeof p_sJson === "string") {
        return eval("(function(){return %1})();".Format(p_sJson));
    } else {
        ReportError("入参必须为字符串类型");
    }
}

function ToJson(p_Object) {
    if (p_Object instanceof Function) {
        throw "不支持函数类型";
    }

    if (!(p_Object instanceof Object)) {
        throw "入参必须为对象/数组";
    }

    var arrResult = [];

    if (p_Object instanceof Array) {
        arrResult.push("[");
    } else {
        arrResult.push("{");
    }

    for (var sName in p_Object) {
        var obj = p_Object[sName];

        if (obj instanceof Function) {
            throw "不支持函数类型(%1)".Format(sName);
        }

        if (1 < arrResult.length) {
            arrResult.push(",'%1':".Format(sName));
        } else {
            arrResult.push("'%1':".Format(sName));
        }

        if (obj instanceof Array || obj instanceof Object) {
            arrResult.push(ToJson(obj));
        } else if (typeof obj == "string") {
            arrResult.push("'%1'".Format(obj.replace(/\\/g, "\\\\").replace(/'/g, "\\'")));
        } else if (typeof obj == "number") {
            arrResult.push(obj);
        } else if (typeof obj == "boolean") {
            arrResult.push(obj);
        } else {
            throw "无法识别的数据类型(%1, %2)".Format(sName, typeof obj);
        }
    }

    if (p_Object instanceof Array) {
        arrResult.push("]");
    } else {
        arrResult.push("}");
    }

    return arrResult.join("");
}
