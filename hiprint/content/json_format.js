function formatJson(text_id, format_id) {
    let o_json = $("#" + text_id).val(),
        f_json = $("#" + format_id);

    o_json = o_json.trim();
    if (!isJSON(o_json)) {
        return false
    }
    // 这两步主要是为了压缩json字符串
    let json = JSON.parse(o_json);
    let zip_json_str = JSON.stringify(json);
    f_json.val(formatCode(zip_json_str))

}
/* 
    格式化json字符串函数
    zip_json_str 是压缩后的json字符串，里面没有换行符，空格等无用的符号
    
*/
function formatCode(zip_json_str) {
    let m = zip_json_str.length
    if (m < 1) {
        return ""
    }
    let left_symbols = ["[", "{"]
    let right_symbols = ["]", "}"]

    let insert_obj = {}
    let in_string = false
    let layer = 0

    for (let i = 0; i < m; i++) {
        let next = i + 1
        let last = i
        if (left_symbols.indexOf(zip_json_str[i]) > -1) {
            layer += 1
            insert_obj["" + next] = "\n" + new Array(layer + 1).join("\t")
        }
        if (in_string) {
            if ("\"" === zip_json_str[i]) {
                in_string = false
            }
            continue
        }
        if ("\"" === zip_json_str[i]) {
            in_string = true
            continue
        }
        if ("," === zip_json_str[i]) {
            insert_obj["" + next] = "\n" + new Array(layer + 1).join("\t")
            continue
        }
        if (":" === zip_json_str[i]) {
            insert_obj["" + last] = " "
            insert_obj["" + next] = " "
            continue
        }
        if (right_symbols.indexOf(zip_json_str[i]) > -1) {
            layer -= 1
            insert_obj["" + last] = "\n" + new Array(layer + 1).join("\t")
        }
    }

    // 开始插入到原始字符串中
    let copy_json_arr = zip_json_str.split('')
    let addnum = 0
    for (let k in insert_obj) {
        let index = parseInt(k) + addnum

        copy_json_arr.splice(index, 0, insert_obj[k + ""])

        addnum += 1
    }
    return copy_json_arr.join('')
}
// 使用原生js检查一下字符串是否合法
function isJSON(str) {
    if (typeof str == 'string') {
        try {
            JSON.parse(str)
            return true;
        } catch (e) {
            console.log(e)
            return false;
        }
    }
}