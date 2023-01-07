

var configCustomProvider = (function () {

    return function (options) {
        var addElementTypes = function (context) {
            context.addPrintElementTypes(
                "configModule", [
                new hiprint.PrintElementTypeGroup("属性", [
                    { tid: 'TestModule.professional', title: '专业', field: 'professional', data: '信息管理与信息系统', type: 'text' },
                    { tid: 'configModule.university', title: '大学', field: 'university', data: '北京邮电大学', type: 'text' },
                    { tid: 'configModule.universityAddress', title: '地点', field: 'universityAddress', data: '北京', type: 'text' },
                    { tid: 'configModule.universityDate', title: '时间', field: 'universityDate', data: '2008', type: 'text' },
                    { tid: 'configModule.tech', title: '技能', field: 'tech', data: 'MYSQL,HIVE(数据仓库工具),SPSS(统计产品已服务解决方案),数据挖掘，跨部门沟通能力，业务理解能力，数据解读分析。', type: 'longText' }
                ]),
            ]
            );
        };

        return {
            addElementTypes: addElementTypes
        }
    };
})();