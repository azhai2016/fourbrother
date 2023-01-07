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
$page_security = 'SA_PRINT_DESIGNER';
$path_to_root = '../..';

include($path_to_root . '/includes/session.php');

$help_context = '打印报表设置';

$js = '';
if ($SysPrefs->use_popup_windows && $SysPrefs->use_popup_search)
	$js .= get_js_open_window(1080, 968);

page(_($help_context), false, false, '', $js);

include_once($path_to_root . '/includes/ui.inc');
include_once($path_to_root . '/admin/db/designer_db.inc');

simple_page_mode(true);

//----------------------------------------------------------------------------------

if ($Mode=='ADD_ITEM' || $Mode=='UPDATE_ITEM') {

	//initialise no input errors assumed initially before we test
	$input_error = 0;

	if (strlen($_POST['name']) == 0) {
		$input_error = 1;
		display_error(_('名称不能为空.'));
		set_focus('name');
	}

	if ($input_error !=1) {
		if ($selected_id != -1) {

            $data = array(
			    $_POST['name'],
				$_POST['description'],
				$_POST['sql_txt'],
				$_POST['sum_field'],
				$_POST['mod_id'],
                check_value('inactive')  
            );
			update_print_data($selected_id, $data);
			display_notification(_('选择的内容已更新'));
		} 
		else {

            $data = array(
				$_POST['bind_id'],
                $_POST['name'],
				$_POST['description'],
				$_POST['sql_txt'],
				$_POST['sum_field'],
				$_POST['mod_id'],
                check_value('inactive')  
            );
			add_print_content($data);

			display_notification(_('新内容已添加'));
		}
		$Mode = 'RESET';
	}
}


if ($Mode == 'Printer') {
    $Mode = 'PRINTER';
}

//---------------------------------------------------------------------------------- 

if ($Mode == 'Delete') {

	// PREVENT DELETES IF DEPENDENT RECORDS IN 'stock_master'
	//if (key_in_foreign_table($selected_id, 'stock_master', 'category_id'))
	//	display_error(_('商品分类已经被使用，无法删除.'));
	//else {
		delete_print_data($selected_id);
		display_notification(_('选择的内容已经删除'));
	//}
	$Mode = 'RESET';
}

if ($Mode == 'RESET') {
	$selected_id = -1;
	$sav = get_post('inactive');
	unset($_POST);
	$_POST['show_inactive'] = $sav;
	
}

if (list_updated('inactive'))
	$Ajax->activate('details');

//----------------------------------------------------------------------------------

$result = get_print_template();

start_form();
start_table(TABLESTYLE, "width='30%'");
$th = array(_('编号'),_('报表名称'), _('说明'), '', '','');

inactive_control_column($th);

table_header($th);
$k = 0; //row colour counter

while ($myrow = db_fetch($result)) {
	
	alt_table_row_color($k);
	label_cell($myrow['bind_id']);
	label_cell($myrow['name']);
	label_cell($myrow['description']);
	inactive_control_cell($myrow['id'], $myrow['inactive'], 'name', 'id');
    //print_button_cell2('Printer'.$myrow['id'], _('设置'));
    $link = $path_to_root."/admin/designer.php?id=".$myrow['id'];
    echo '<td style="width:30px;text-align:center;">
       <div title="设计" class="search_btn_container" id="Printer" 
       onclick="javascript:lookupWindow(&quot;'.$link.'&quot;, &quot;&quot;);">
       <span class="search_btn_txt"></span>
       <i class="fas fa-search"></i></div></td>';
	edit_button_cell2('Edit'.$myrow['id'], _('编辑'));
	delete_button_cell2('Delete'.$myrow['id'], _('删除'));
	end_row();

}

inactive_control_row($th);

end_table();

br(1);

//----------------------------------------------------------------------------------

div_start('details');
start_table(TABLESTYLE2);

if ($selected_id != -1) {
	if ($Mode == 'Edit') {
		$myrow = get_print_template_id($selected_id);
		$_POST['bind_id'] = $myrow['bind_id'];
		$_POST['mod_id'] = $myrow['mod_id'];
		$_POST['name'] = $myrow['name'];
		$_POST['description']  = $myrow['description'];
		$_POST['sql_txt']  = $myrow['sql_txt'];
		$_POST['sum_field']  = $myrow['sum_field'];
        $_POST['inactive']  = $myrow['inactive'];

	} 
	hidden('selected_id', $selected_id);
	hidden('id');
}
else if ($Mode != 'CLONE') {
	$_POST['name'] = '';
	$_POST['description'] = '';
	$_POST['sql_txt']  = '';
	$_POST['sum_field']  = '';
	$_POST['inactive']  = 0;
	$company_record = get_company_prefs();

}
if ($selected_id==-1) {
  text_row(_('编号:'), 'bind_id', null, 30, 30);  
}
else 
  label_row(_('编号:'), get_post('bind_id'));  

print_mod_list('报表ID','mod_id');

text_row(_('报表别名:'), 'name', null, 30, 30);  
text_row(_('说明:'), 'description', null, 30, 30);  
textarea_row(_('SQL语句:'), 'sql_txt', null, 30, 9);
text_row(_('汇总字段:'), 'sum_field', null, 30, 30);  
check_row(_('激活:'), 'inactive');

end_table();
div_end();
br(2);

submit_add_or_update_center($selected_id == -1, '', 'both', true);

end_form();
br(1);

end_page();