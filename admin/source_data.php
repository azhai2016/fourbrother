<?php

$page_security = 'SA_SOURCEDATA';
$path_to_root = '..';

include_once($path_to_root.'/includes/db_pager.inc');
include_once $path_to_root . '/includes/session.php';

$js = '';
if ($SysPrefs->use_popup_windows) {
    $js .= get_js_open_window(900, 500);
}

if (user_use_date_picker()) {
    $js .= get_js_date_picker();
}

page(_($help_context = '源数据配置'), @$_REQUEST['popup'], false, '', $js);

include_once $path_to_root . '/includes/date_functions.inc';
include_once $path_to_root . '/includes/ui.inc';
include_once $path_to_root . '/includes/references.inc';
include_once $path_to_root . '/admin/db/sources_db.inc';
include_once($path_to_root . '/includes/ui/source_target_view.inc');



$selected_id = get_post('id', '');

//--------------------------------------------------------------------------------------------

function can_process()
{
    if (strlen($_POST['SourceName']) == 0) {
        display_error(_('源数据名称不能为空'));
        set_focus('SourceName');
        return false;
    }
    if (strlen('SourceTableName') == 0) {
        display_error(_('源数据数据表名称'));
        set_focus('SourceTableName');
        return false;
    }
    if (strlen('SourceTableMasterKey')==0) {
        display_error(_('主键不能为空'));
        set_focus('SourceTableMasterKey');
        return false;
    }
    if (strlen('sqlScript')==0) {
        display_error(_('SQL脚本不能为空'));
        set_focus('sqlScript');
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------

function handle_submit(&$selected_id)
{
    global $path_to_root, $Ajax, $SysPrefs, $Refs;

    if (!can_process()) {
        return;
    }

    $isActive = empty(get_post('isActive'))?0:1;
    $isJudge = empty(get_post('isJudge'))?0:1;
    $isCalculation = empty(get_post('isCalculation'))?0:1;

    if ($selected_id) {
        mssql_transaction();   
        //更新数据、
        
        update_source_data($selected_id,$_POST['SourceId'],$_POST['SourceName'],$_POST['SourceTableName'],$_POST['SourceTableMasterKey'],
        $isActive,$isJudge,$isCalculation,$_POST['sqlScript']); 
        mssql_commit();

        $Ajax->activate('_page_body'); // in case of status change

        display_notification(_('源数据信息已更新'));

    } else { //it is a new data

        mssql_transaction();
  
       // 写入数据
        $sourceId= $Refs->get_next(ST_SOURCE_ID,1);
        add_source_data($sourceId,$_POST['SourceName'],$_POST['SourceTableName'],$_POST['SourceTableMasterKey'],
        $isActive,$isJudge,$isCalculation,$_POST['sqlScript']); 

        mssql_commit();

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
        delete_source_data($_POST['SourceId']);

        display_notification(_('所选源数据已被删除。'));
        unset($_POST['id']);
        $selected_id = '';
        $Ajax->activate('_page_body');
    } //end if Delete Customer

}

function clear_data()
{
    unset($_POST['SourceId']);
    unset($_POST['SourceName']);
    unset($_POST['SourceTableName']);
    unset($_POST['SourceTableMasterKey']);
    unset($_POST['isActive']);
    unset($_POST['isJudge']);
    unset($_POST['isCalculation']);
    unset($_POST['sqlScript']);
}


//----------------------------------------------
function source_settings($selected_id)
{
    global $SysPrefs, $path_to_root, $page_nested,$Refs;

    
     
    if (!$selected_id) {
        clear_data();
    } else {
   
        $myrow = get_source_id_data($selected_id);
 
        $_POST['SourceId'] = $myrow['SourceId'];
        $_POST['SourceName'] = $myrow['SourceName'];
        $_POST['SourceTableName'] = $myrow['SourceTableName'];
        $_POST['SourceTableMasterKey'] = $myrow['SourceTableMasterKey'];
        $_POST['isJudge'] = $myrow['isJudge'];
        $_POST['isActive'] = $myrow['isActive'];
        $_POST['isCalculation'] = $myrow['isCalculation'];
        $_POST['sqlScript'] = $myrow['sqlScript'];

    }

    start_outer_table(TABLESTYLE2);
    table_section(1);
    table_section_title(_('源名称内容'));

    //text_row(_('源数据编号:'), 'SourceId',isset($_POST['SourceId']) ? $_POST['SourceId'] : null, 30, 30);
    if ($selected_id)
    hidden('SourceId',$_POST['SourceId']);
    //label_row(_('源数据编号:'),!$selected_id? '' : $_POST['SourceId']);
    text_row(_('名称:'), 'SourceName', isset($_POST['SourceName']) ? $_POST['SourceName']:null, 40, 80);
    text_row(_('数据库表名:'), 'SourceTableName', isset($_POST['SourceTableName']) ?$_POST['SourceTableName']:null, 30, 30);
    text_row(_('主键:'), 'SourceTableMasterKey', isset($_POST['SourceTableMasterKey']) ?$_POST['SourceTableMasterKey']:null, 30, 30);

    check_row(_('有效:'), 'isActive');
    check_row(_('判断类型:'), 'isJudge');
    check_row(_('计算类型:'), 'isCalculation');

    textarea_row(_('SQL脚本:'), 'sqlScript',isset($_POST['sqlScript']) ? $_POST['sqlScript']:null, 60, 15);
 
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


//--------------------------------------------------

function edit_link($row) {
    global $page_nested;

	if ($page_nested)
		return '';

	// allow only free hand credit notes and not voided edition
	return  trans_editor_link(ST_SOURCEDATA, $row['targetId']);
}

function delete_link($row) {}


function target_setting($selected_id){
    div_start('item_tbl_target_data');
start_form();

start_table(TABLESTYLE_NOBORDER);


//------------------------------------------------------------------------------------------------
$sqlwhere = " and sourceId=".$selected_id;
$showcol = '*';  
$sql = 'select * from baseinfoProduct'; 
//------------------------------------------------------------------------------------------------

$cols = array(
	_('参数ID') => array('name'=>'targetId', 'align'=>'right'), 
	_('参数名称') => array('name'=>'targetName', 'align'=>'right'), 
	_('参数类型') => array('name'=>'targetType', 'align'=>'right'), 
	_('参数数据类型') => array('name'=>'targetDataType', 'align'=>'right'), 
	_('脚本') => array('name'=>'sqlScript', 'align'=>'right'), 
    _('数据表名') => array('name'=>'tablename', 'align'=>'right'), 
    _('默认值') => array('name'=>'defaultValue', 'align'=>'right'), 
	array('insert'=>true, 'fun'=>'edit_link'),
    array('insert'=>true, 'fun'=>'delete_link')
);

$table = &new_db_pager('item_tbl_target_data', $sql, $cols,null,'targetId');


$table->width = '85%';

display_db_pager($table);

end_form();


start_table(TABLESTYLE);

amount_row(_('单价:'), 'price', null, '', '元', $dec2);
text_row(_('计量单位:'), 'suppliers_uom', null, 50, 51);

amount_row(_('单位换算系数:'), 'conversion_factor', null, null, null, 'max');
text_row(_("备注:"), 'supplier_description', null, 50, 50);

end_table(1);

submit_add_or_update_center($selected_id == 1, '', 'both');

div_end();

}

//--------------------------------------------------------------------------------------------

check_db_has_sales_types(_('没有定义任何销售类型。在添加客户之前，请至少定义一种销售类型。'));

start_form(true);


if (db_has_sources()) {
    start_table(TABLESTYLE_NOBORDER);
    start_row();
    source_data_list_cells(_('选择源数据: '), 'id', null,  _('新增数据源'), true, null);
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
        source_settings($selected_id);
        break;
    case 'target':
        //$_GET['source_id'] = $selected_id;
        //include_once $path_to_root . '/admin/includes/target_data.php'; 

         $sourceTargets = new sourceTargets('target_data', $selected_id, 'target_data');
         $sourceTargets->show(); 
        //target_setting($selected_id);
        break;
   
};
br();
tabbed_content_end();

end_form();
end_page(@$_REQUEST['popup']);
