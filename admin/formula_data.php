<?php

$page_security = 'SA_FORMULA';
$path_to_root = '..';

include_once($path_to_root.'/includes/db_ms_pager.inc');
include_once $path_to_root . '/includes/session.php';

$key =  'temp_source_list_data';

$js = '';
if ($SysPrefs->use_popup_windows) {
    $js .= get_js_open_window(900, 500);
}

if (user_use_date_picker()) {
    $js .= get_js_date_picker();
}


page(_($help_context = '计算配置'), @$_REQUEST['popup'], false, '', $js);

include_once $path_to_root . '/includes/date_functions.inc';
include_once $path_to_root . '/includes/ui.inc';
include_once $path_to_root . '/includes/references.inc';
include_once $path_to_root . '/admin/db/formula_db.inc';
include_once($path_to_root . '/includes/ui/formula_view.inc');



$selected_id = get_post('id', '');


//--------------------------------------------------------------------------------------------

function can_process()
{
    if (strlen('FormulaName') == 0) {
        display_error(_('源数据数据表名称'));
        set_focus('SourceTableName');
        return false;
    }
    if (strlen('FormulaContent')==0) {
        display_error(_('内容不能为空'));
        set_focus('FormulaContent');
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------

function handle_submit(&$selected_id)
{
    global $path_to_root, $Ajax, $SysPrefs, $Refs,$key;

    if (!can_process()) {
        return;
    }

    $isActive = empty(get_post('isActive'))?0:1;

   
    if ($selected_id) {
        mssql_transaction();   
        //更新数据、

        update_formula_data($selected_id,$_POST['FormulaId'],$_POST['FormulaName'],$_POST['SourceId'],$_POST['FormulaType'],$_POST['FormulaStatus'],$_POST['FormulaContent'],$_POST['Description'], $isActive); 
        mssql_commit();

        unset($_SESSION[$key]);

        $Ajax->activate('_page_body'); // in case of status change

        display_notification(_('源数据信息已更新'));

    } else { //it is a new data

        mssql_transaction();
 
       // 写入数据
        $formulaId= $Refs->get_next(ST_FORMULA_ID,1);

        add_formula_data($formulaId,$_POST['FormulaName'],$_POST['SourceId'],$_POST['FormulaType'],$_POST['FormulaStatus'],$_POST['FormulaContent'],$_POST['Description'], $isActive); 

        mssql_commit();

        unset($_SESSION[$key]);

        display_notification(_('新源数据信息已添加'));

        $Ajax->activate('_page_body');
    }
}
//--------------------------------------------------------------------------------------------

if (isset($_POST['submit'])) {
    handle_submit($selected_id);
}

//--------------------------------------------------------------------------------------------

if (isset($_POST['delete'])) {

    $cancel_delete = 0;

    if ($cancel_delete == 0) { //ie not cancelled the delete as a result of above tests

        //删除内容
        delete_formula_data($_POST['FormulaId']);

        display_notification(_('所选源数据已被删除。'));
        unset($_POST['FormulaId']);
        $selected_id = '';
        $Ajax->activate('_page_body');
    } //end if Delete Customer

}

function clear_data()
{
    unset($_POST['FormulaId']);
    unset($_POST['FormulaName']);
    unset($_POST['FormulaType']);
    unset($_POST['FormulaContent']);
    unset($_POST['isActive']);
    unset($_POST['FormulaStatus']);
    unset($_POST['Description']);

}



//--------------------------------------------------

function edit_link($row) {
    global $page_nested;

	if ($page_nested)
		return '';

	// allow only free hand credit notes and not voided edition
	return  trans_editor_link(ST_SOURCEDATA, $row['targetId']);
}

function delete_link($row) {}



function formula_settings($selected_id)
{
    global $SysPrefs, $path_to_root, $page_nested,$Refs,$key;
     
    if (!$selected_id) {
        clear_data();
    } else {
    
   
        if (!isset($_SESSION[$key]) || ($selected_id != $_SESSION[$key]['_selected_id'])) {
          $myrow = get_formula_id_data($selected_id);
          $myrow['_selected_id'] = $selected_id;
          $_SESSION[$key] = $myrow;  
       }
       else {

          $myrow =  $_SESSION[$key];
          $myrow['SourceId'] = isset($_POST['SourceId'])?$_POST['SourceId']:'';
          $myrow['_selected_id'] = $selected_id;

          $_SESSION[$key]=$myrow;

        }
   
        $_POST['FormulaId'] = $myrow['FormulaId'];
        $_POST['FormulaName'] = $myrow['FormulaName'];
        $_POST['SourceId'] = $myrow['SourceId'];
        $_POST['FormulaType'] = $myrow['FormulaType'];
        $_POST['FormulaContent'] = $myrow['FormulaContent'];
        $_POST['FormulaStatus'] = $myrow['FormulaStatus'];
        $_POST['isActive'] = $myrow['IsActive'];
        $_POST['Description'] = $myrow['Description'];
             
    }


    

    start_outer_table(TABLESTYLE2);
    table_section(1);
    table_section_title(_('基本设置'));

 
    if ($selected_id)   
       hidden('FormulaId',$_POST['FormulaId']);



    $source =  isset($_POST['SourceId'])?$_POST['SourceId']:'';
    
    source_data_list_cells(_('选择源数据: '), 'SourceId',$source,false,true,true);
    text_row(_('名称:'), 'FormulaName', isset($_POST['FormulaName']) ? $_POST['FormulaName']:null, 40, 80);
    formula_type_list_row(_('计算类型:'),'FormulaType',isset($_POST['FormulaType']) ? $_POST['FormulaType']:'');
    check_row(_('有效:'), 'isActive');

    table_section(2);
    table_section_title(_('计算内容'));
    source_data_target_list_row(_('选择参数: '), 'TargetId',$source);

    textarea_row(_('内容:'), 'FormulaContent',isset($_POST['FormulaContent']) ? $_POST['FormulaContent']:null, 50, 5);

    text_row(_('说明:'), 'FormulaStatus', isset($_POST['FormulaStatus']) ? $_POST['FormulaStatus']:null, 43, 80);
    textarea_row(_('描述:'), 'Description',isset($_POST['Description']) ? $_POST['Description']:null, 50, 2);
    

    end_outer_table(1);

    div_start('controls');
    if (@$_REQUEST['popup']) {
        hidden('popup', 1);
    }



    if (!$selected_id) {
        submit_center('submit', _('保存源数据'), true, '', false);
    } else {
        submit_center_first('submit', _('更新源数据'), _('更新源数据'), $page_nested ? true : false);
        submit_return('select', $selected_id, _('选择此客商并返回文档'));
        submit_center_last('delete', _('删除源数据'), _('删除从未使用过的客商数据'), true);
    }

    div_end();


}

//--------------------------------------------------------------------------------------------

check_db_has_sales_types(_('没有定义任何销售类型。在添加客户之前，请至少定义一种销售类型。'));

start_form(true);


if (db_has_formulas()) {
    start_table(TABLESTYLE_NOBORDER);
    start_row();
    formula_data_list_cells(_('选择计算配置: '), 'id', null,  _('新增计算配置'), true, null);
    end_row();
    end_table();

} else {
    hidden('id');
}

if (!$selected_id) {
    unset($_POST['_tabs_sel']);
}

tabbed_content_start('tabs', array(
    'source' => array(_('源数据'), $selected_id),
    'target' => array(_('参数'), $selected_id),
 ));

switch (get_post('_tabs_sel')) {
    default:
    case 'source':
        formula_settings($selected_id);
        break;
    case 'target':
        //$_GET['source_id'] = $selected_id;
        //include_once $path_to_root . '/admin/includes/target_data.php'; 

         $formulas = new Formulas('formula_data', $selected_id, 'formula_data');
         $formulas->show(); 
        //target_setting($selected_id);
        break;
   
};
br();
tabbed_content_end();


end_form();


end_page(@$_REQUEST['popup']);
