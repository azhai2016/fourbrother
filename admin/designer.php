<?php
/**********************************************************************
Copyright (C) FrontAccounting, LLC.
Released under the terms of the GNU General Public License, GPL,
as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
 ***********************************************************************/
$page_security = 'SA_DESIGNER';

$path_to_root = '..';
include_once $path_to_root . '/includes/session.php';

include_once $path_to_root . '/includes/ui.inc';

include_once $path_to_root . '/admin/db/designer_db.inc';


$items_id = $_GET['id'];

log_b($_GET);

function getJson()
{
    $json = file_get_contents('../hiprint/test.json');
    return json_encode($json);
}

function getPrintData()
{
    $json = file_get_contents('../hiprint/data.json');
    return json_encode($json);
}

function get_auto_save()
{

    $json = file_get_contents('../hiprint/autosave.json');
    return empty($json) ? '"{}"' : json_encode($json);
}

function get_attrib_json()
{
    $json = file_get_contents('../hiprint/attrib.json');
    return empty($json) ? '"[]"' : json_encode($json);
}

simple_page_mode(true);
start_form();
?>
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta http-equiv="Content-Type" Content="text/html;charset=utf8"/>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="keywords" content="hiprint">

    <title>hinnn-hiprint</title>
    <!-- hiprint -->
    <link href="<?php echo $path_to_root; ?>/hiprint/css/bootstrap.min.css" rel="stylesheet">
    <link href="<?php echo $path_to_root; ?>/hiprint/css/hinnn.css" rel="stylesheet" />
    <link href="<?php echo $path_to_root; ?>/hiprint/css/document.css" rel="stylesheet" />
    <link href="<?php echo $path_to_root; ?>/hiprint/css/hiprint.css?<?php echo time(); ?>" rel="stylesheet">
    <link href="<?php echo $path_to_root; ?>/hiprint/css/print-lock.css?<?php echo time(); ?>" rel="stylesheet">
    <link href="<?php echo $path_to_root; ?>/hiprint/css/print-lock.css" media="print" rel="stylesheet">
    <link href="<?php echo $path_to_root; ?>/hiprint/css/dom.css?<?php echo time(); ?>" rel="stylesheet">
    
    <style>
        .d_panel td{
           background:#fff;
        }
        .d_panel button{
            width:80px;
        }

        #save_progress {
           position:absolute;
           right:0px;
           top:2px;
           padding: 0 50px 0 px;
           color:red;
           text-align:left;
           font-size:12px;
        }
    </style>
</head>

<body>



   <table class="d_panel" style="width:100%;background: #DADADA;">
    <tr><td colspan="3" style="text-align:center;padding:10px 10px;"> 
        <div class="hiprint-toolbar" style="margin-top:15px;position:relative">
            <ul>
              <li><a class="hiprint-toolbar-item" id="save">保存</a></li>
              <li><a class="hiprint-toolbar-item" id="cancel">取消</a></li>
              <li><span class="hiprint-toolbar-item" style="height:30px;border-left:1px #ccc solid;position: relative;
    float: left;">&nbsp;</span></li>
              <li><a class="hiprint-toolbar-item" id="review">预览</a></li>
              <li><a class="hiprint-toolbar-item" id="print" onclick="print_data()">打印</a></li>
            </ul>
          </td>
        </div>
    </tr>
    <tr><td colspan="3" style="text-align:center;padding:10px 10px;">
     
    <div class="hiprint-toolbar" style="margin-top:15px;position:relative">
    <div id="save_progress" ></div>
                                <ul>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('A3')">A3</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('A4')">A4</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('A5')">A5</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('B3')">B3</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('B4')">B4</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="setPaper('B5')">B5</a></li>

                                    <li><a class="hiprint-toolbar-item"><input type="text" id="customWidth" style="width: 50px;height: 19px;border: 0px;" placeholder="宽/mm" /></a></li>
                                    <li><a class="hiprint-toolbar-item"><input type="text" id="customHeight" style="width: 50px;height: 19px;border: 0px;" placeholder="高/mm" /></a></li>

                                    <li><a class="hiprint-toolbar-item" onclick="setPaper($('#customWidth').val(),$('#customHeight').val())">自定义</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="rotatePaper()">旋转</a></li>
                                    <li><a class="hiprint-toolbar-item" onclick="clearTemplate()">清空</a></li>



                                </ul>
    </div>
</td></tr>
    <tr>
   <td style=" vertical-align: top;width:100px;margin-left:30px;"> <div class="small-printElement-types hiprintEpContainer"></div></td>
   <td style="vertical-align: top;width:80%">
        <div id="hiprint-printTemplate" class="hiprint-printTemplate" style="
        text-align:center;margin-top:10px;margin-left:50px;border:1px #000 solid;background:#ccc;min-height:1000px;min-width:800px;" >
   </td>
   <td style="vertical-align: top;width:200px;"><div id="PrintElementOptionSetting" style="margin-top:10px;"></div></td>
</tr>

   </table>


        <div class="modal fade" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel">
            <div class="modal-dialog modal-lg" role="document" style="display: inline-block; width: auto;">
                <div class="modal-content">
                    <div class="modal-header">
                        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                        <h4 class="modal-title" id="myModalLabel">打印预览</h4>
                    </div>
                    <div class="modal-body">

                        <button type="button" class="btn btn-danger" id="print" >打印</button>
                        <div class="prevViewDiv">
                        <table style="margin-bottomm:150px;"  class="barcodes"></table>
                        </div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>

                    </div>
                </div>
            </div>
        </div>

        <div>


        <textarea style="display:none" id="json_text"></textarea>
        <textarea style="height:500px;overflow:auto;word-break:break-all;width:100%" id="json_text_json"></textarea>

        </div>


    <!-- jQuery (hiprint 的所有 JavaScript 插件都依赖 jQuery，所以必须放在前边) -->
    <script src="<?php echo $path_to_root; ?>/hiprint/content/jquery.js"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/content/jquery.nicescroll.min.js"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/plugins/bootstrap.min.js"></script>
    <!-- 加载 hiprint 的所有 JavaScript 插件。你也可以根据需要只加载单个插件。 -->
    <!-- hiprint 打印初始化，更多参数请查看文档 -->


    <!-- polyfill.min.js解决浏览器兼容性问题v6.26.0-->
    <script src="<?php echo $path_to_root; ?>/hiprint/polyfill.min.js"></script>
    <!-- hiprint 核心js-->
    <script src="<?php echo $path_to_root; ?>/hiprint/content/hiprint.bundle.js?t=<?php echo time(); ?>"></script>

    <!-- 条形码生成组件-->
    <script src="<?php echo $path_to_root; ?>/hiprint/content/JsBarcode.all.min.js"></script>
    <!-- 二维码生成组件-->
    <script src="<?php echo $path_to_root; ?>/hiprint/content/qrcode.js"></script>
    <!-- 调用浏览器打印窗口helper类，可自行替换-->
    <script src="<?php echo $path_to_root; ?>/hiprint/content/jquery.hiwprint.js"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/content/config-etype-provider.js?<?php echo time(); ?>"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/content/config-print-json.js"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/content/jquery.minicolors.min.js"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/content/json_format.js?<?php echo time(); ?>"></script>
    <script src="<?php echo $path_to_root; ?>/hiprint/js/filesaveas.js"></script>

    <script type="text/javascript">
    var hiprintTemplate;
    var attrib_json;

    function showModel(){
			var wW=$(window).width();  //浏览器可视区域宽度和高度
			var wH=$(window).height();
			var oW=$(".setup_model").innerWidth(); //获取类叫model的宽度和高度
			var oH=$(".setup_model").innerHeight();
			$(".setup_model").show().css({"top":(wH-oH)/2+"px","left":(wW-oW)/2+"px"});
			$(".mask").fadeIn();
	}

    var save_db_path = '<?php echo $path_to_root; ?>/hiprint/includes/save.php';
    var save_path = '<?php echo $path_to_root; ?>/hiprint/includes/save_json.php';

    function getJson(){
        return <?php getJson();?>
    }

    function sleep(ms) {
      return new Promise(function(resolve, reject) {
        attrib_json = JSON.parse(<?php echo get_attrib_json(); ?>);
        setTimeout(resolve, ms);
      })
   }


    $(document).ready(function() {

        var printData =''; //JSON.parse(<?php //echo getPrintData(); ?>);

        var configPrintJson = '';//JSON.parse(<?php //echo getJson(); ?>);
        <?php 
            $rs = get_print_template_id($items_id);
     
            $json =  $rs['json'];
            $page_height = '';
            $page_width = '';
            if ($json=='{}') {
               
                $json = array('panels'=>array(
                  'index'=>0,
                  'paperType'=>"A4",
                  'height'=> 297,
                  'width'=>210,
                  'paperHeader'=>45,
                  'paperFooter'=>780,
                  'printElements'=>array(),
                  'paperNumberLeft'=>565,
                  'paperNumberTop'=>819
               )); 

               $json = json_encode($json);
         

            }
            else {
               $json = htmlspecialchars_decode($json);
               $json_data = json_decode($json);
               $save_file = $path_to_root.'/hiprint/includes/save_json.php';
               $sf = file_put_contents($save_file,$json);
             

               if (count((array)$json_data) > 0) {
                 $page_height = $json_data->panels[0]->height;
                 $page_width = $json_data->panels[0]->width;
               }
            }
        

            $data_sql = get_print_template_sql($rs['sql_txt']);
            $fields = [];
            $column = [];
            foreach($data_sql as $key=>$value) {
                $field['tid']= 'configModule.'.$value->orgname;
                $field['title']= $value->name;
                $field['field']= $value->name;
                $field['data']= 'test';
                $field['customText']= '自定义文本';
                $field['custom']='true';
                $field['type']= 'text';
                $fields[]=  $field;
                $column = array(
                    'title'=> $value->name,
                    'field'=>$value->orgname,
                    'width'=>85,
                    'fixed'=>false,
                    'rowspan'=>1,
                    'colspan'=>1,
                    'columnId'=>$value->orgname
                );

                $columns[] = $column;
            }
            $field=[];
            $field['tid'] = 'configModule.table1';
            $field['title']= '表格';
            $field['field']= 'table';
            $field['type']= 'table';
            $field['columns'][] = $columns;
            $fields[]=$field;
            $cusEle = json_encode($fields);
        ?>

       // configPrintJson =JSON.parse(<?php //echo htmlspecialchars_decode(db_escape($json)); ?>);

        const  cusEle= <?php echo $cusEle; ?>;
        console.log(cusEle);
        hiprint.init({
             providers: [new configElementTypeProvider(cusEle)]
        }); 

        hiprint.PrintElementTypeManager.build('.hiprintEpContainer', 'configModule');
        
        $.getJSON(save_path, function(json) {
             console.log(json);
             load_data(json);       
        });
      

        $('#customWidth').val(<?php echo $page_width;?>);
        $('#customHeight').val(<?php echo $page_height;?>);
      

        $(".print-content").niceScroll();
            //初始化打印插件

        $('#print').click(function() {
           // for(var i=0;i<5;i++) {
              print_data();
           // }
        });

        
        $('#review').click(function() {
    
            preview();
        });


        $('#new').click(function() {
            hiprintTemplate.clear();
            showModel();
        });

        $('#open').click(function () {
            $('#hiprint-printTemplate').html('');
            load_data(configPrintJson);
        });

        $('#save').click(function () {
            auto_save();
      
            var name=$('input[name="current_print_name"]').val();
            var desc=$('input[name="print_description"]').val();

            var json = $('#json_text').val();

            $.ajax({
              type: "POST",
              dataType : 'json',
              async: false,
              url: save_db_path,
              data: {id:<?php echo $items_id ?>,json},
              beforeSend:function(){
                 $('#save_progress').html('正在保存中...');
              },
              success: function(a){
               
              },
              error: function(e){
       
             
              }
         });
            //var file = new File([json], "手机号.txt", { type: "text/plain;charset=utf-8" });
           // saveAs(file);
        });


        auto_save();

        $(document).keydown(function(ev){
			if(ev.keyCode==27){  
				$(".close").trigger("click");
			}
		})

        $(window).resize(function(){
			if($(".setup_model").is(":visible")){ 
				showModel();
			}
		});

	   $(".close").click(function(){
			$(".setup_model").hide();
			$(".mask").fadeOut();
	    });

    });

   


    var auto_save = function(){

            save_json();
            var json = $('#json_text').val();

            $.ajax({
              type: "POST",
              dataType : 'json',
              async: false,
              url: save_path,
              data: { data: json },
              beforeSend:function(){
                 $('#save_progress').html('保存中...');
              },
              success: function(){
                setTimeout(() => {
                $('#save_progress').html('');
               }, 3000);

              },
              error: function(a){
               setTimeout(() => {
                $('#save_progress').html('');
               }, 3000);

              }
         });


        setTimeout(() => {
            auto_save();
        }, 30000);
    }




    var save_json = function(){
        var json = JSON.stringify(hiprintTemplate.getJsonTid());

        $("#json_text").val(json);
        
        formatJson('json_text','json_text_json');
    }

    var _batch_data;

    var print_data = function() {

     //   _printData = JSON.parse(<?php //echo getPrintData(); ?>);

           
               let _batch_data = [];
               _printData = JSON.parse(<?php echo getPrintData(); ?>);

            let _data = _printData['mytable'];
     
            for (let i=0;i<_data.length-1;i++) {
          
                var _t = [];
                var _o = _data[i];
                
                _batch_data.push(_t);
            }  
           
           /* let _data = _printData['mytable'];
            $('#myModal .modal-body .barcodes').html('');
            for (var i=0;i<_data.length-1;i++) {
                var rows = _data[i];
                var tb = '<tr>';
                tb += '<td>'+rows['name']+'</td>';
                tb += '<td>'+rows['like']+'</td>';
                tb +='<td>'+rows['gender']+'</td>';

                var barcode= String(rows['barcode']);
                tb +='<td><img id="barcode_row_'+i+'" /></td></tr>';
                //$('.barcodes').append(tb);
                $('#myModal .modal-body .barcodes').append(tb);
                JsBarcode("#barcode_row_"+i,barcode,{height:20});
               // $('#myModal .modal-body .prevViewDiv').append(tb);
            } 
            hiprintTemplate.printByHtml($('#myModal .modal-body .barcodes'));  */
            hiprintTemplate.print(_printData);
    }

    var load_data = function(json) {

         hiprintTemplate = new hiprint.PrintTemplate({
            template: json,
            settingContainer: '#PrintElementOptionSetting',
            paginationContainer: '.hiprint-printPagination'}
        );

        hiprintTemplate.design('#hiprint-printTemplate');
        var _json = JSON.stringify(json);

       $('#json_text').val(_json);
       formatJson('json_text','json_text_json');
    }

      var setPaper = function (paperTypeOrWidth, height) {
             hiprintTemplate.setPaper(paperTypeOrWidth, height);
     }

      var preview = function () {
           _printData = JSON.parse(<?php echo getPrintData(); ?>);

            $('#myModal .modal-body').html(hiprintTemplate.getHtml(_printData));
            $('#myModal').modal('show');
        }

    </script>



     <div class="setup_model" style="height:200px;">
		<div class="model-header" >
			<h5>新建</h5>
			<span class="close">×</span> 
		</div>
        <form name="p_panel">
         
        <div class="model-body"> 
           
            <label for="print_name">名称：</label>
            <input class="print_name" name="print_name" value="" />
            <br/>
            <label for="print_description">说明：</label>
            <input class="print_description" name="print_description" value="" />
         
        </div>
		 <div class="model-footer">
            <button class="btn btn-primary" type="submit" name="save_btn" value="save">保存</button>
         </div>
        </form>
	</div>
	<div class="mask"></div>

</body>

</html>

<?php

if (isset($_POST['save_btn'])) {
   // $_SESSION['print_name']= get_post('print_name');
    hidden('current_print_name',$_POST['print_name']);
    hidden('current_print_json',$_POST['print_description']);
}



br(2);
end_form();
end_page();
?>