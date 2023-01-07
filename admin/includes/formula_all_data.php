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

$page_security = 'SA_FORMULA';
$path_to_root = '../..';
include_once($path_to_root.'/includes/session.php');
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/admin/db/formula_db.inc');

$js = get_js_select_combo_item();

page(_($help_context = '计算配置列表'), true, false, '', $js);

if(get_post('search'))
	$Ajax->activate('item_tbl');

start_form(false, $_SERVER['PHP_SELF'].'?'.$_SERVER['QUERY_STRING']);

start_table(TABLESTYLE_NOBORDER);
start_row();

text_cells(_('名称'), 'findname');
submit_cells('查询', _('查询'), '', _('查询商品'), 'default');

end_row();
end_table();

end_form();

div_start('formula_tbl');
start_table(TABLESTYLE);

$th = array('', _('编号'), _('名称'), _('类型'));
table_header($th);

$k = 0;
$result = get_formula_data(get_post('findname'));

while ($myrow = mssqldb_fetch_assoc($result)) {

	alt_table_row_color($k);

	ahref_cell(_('选择'), 'javascript:void(0)', '', 'selectComboItem(window.opener.document, "'.$_GET['client_id'].'", "'.$myrow['Id'].'")');
	label_cell($myrow['Id']);
	label_cell($myrow['FormulaId']);
	label_cell($myrow['FormulaName']);
	end_row();
}

end_table(1);

div_end();
end_page(true);
