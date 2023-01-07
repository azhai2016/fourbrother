
var configElementTypeProvider = (function () {

    return function (options) {

        var addElementTypes = function (context) {
            context.addPrintElementTypes(

                "configModule",
                [
                    new hiprint.PrintElementTypeGroup("表属性", options),
                    new hiprint.PrintElementTypeGroup("常规", [
                        { tid: 'customText', title: '文本', data: '', type: 'text' },
                        {
                            tid: 'image', text: '图片', data: '../hiprint/image/hi.png', type: 'image',
                            "options": { "src": '../hiprint/image/hi.png' }
                        },
                        { tid: 'mytable', text: '表格', data: '', type: 'tableCustom' },
                        { tid: 'pages', title: '页码', data: '', type: 'text' },
                        { tid: 'total', title: '总计', data: '', type: 'text' },
                        { tid: 'subtotal', title: '小计', data: '', type: 'text' },


                    ]),
                    new hiprint.PrintElementTypeGroup("条码", [
                        { tid: 'configModule.barcode', title: '条形码', data: '', barcodeMode: "CODE128C", textType: 'barcode' },
                        { tid: 'configModule.qrcode', title: '二维码', data: '', textType: 'qrcode' },
                    ]),

                    new hiprint.PrintElementTypeGroup("辅助", [
                        {
                            tid: 'configModule.hline',
                            title: '横线',
                            type: 'hline'
                        },
                        {
                            tid: 'configModule.vline',
                            title: '竖线',
                            type: 'vline'
                        },
                        {
                            tid: 'configModule.rect',
                            title: '矩形',
                            type: 'rect'
                        },
                        {
                            tid: 'configModule.oval',
                            title: '椭圆',
                            type: 'oval'
                        }
                    ]),

                ]
            );
        };

        return {
            addElementTypes: addElementTypes
        };

    };
})();