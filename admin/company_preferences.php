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
$page_security = 'SA_SETUPCOMPANY';
$path_to_root = '..';
include($path_to_root.'/includes/session.php');

page(_($help_context = '公司设置'));

include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/includes/ui.inc');

include_once($path_to_root.'/admin/db/company_db.inc');
include_once($path_to_root.'/reporting/includes/tcpdf.php');

//-------------------------------------------------------------------------------------------------

if (isset($_POST['update']) && $_POST['update'] != '') {
	$input_error = 0;
	if (!check_num('login_tout', 10)) {
		display_error(_('登录超时数必须不小于10。'));
		set_focus('login_tout');
		$input_error = 1;
	}
	if (strlen($_POST['coy_name'])==0) {
		$input_error = 1;
		display_error(_('必须输入公司名称。'));
		set_focus('coy_name');
	}
	//if (!check_num('tax_prd', 1)) {
	//	display_error(_('纳税期限必须为正数。'));
	//	set_focus('tax_prd');
	//	$input_error = 1;
	//}
	//if (!check_num('tax_last', 1)) {
//		display_error(_('最后纳税期必须为正数。'));
//		set_focus('tax_last');
//		$input_error = 1;
//	}
	if (!check_num('round_to', 1)) {
		display_error(_('四舍五入计算字段必须为正数。'));
		set_focus('round_to');
		$input_error = 1;
	}
	if (!check_num('max_days_in_docs', 1)) {
		display_error(_('文档中的最大日范围必须为正数。'));
		set_focus('max_days_in_docs');
		$input_error = 1;
	}
	//if ($_POST['add_pct'] != '' && !is_numeric($_POST['add_pct'])) {
	//	display_error(_('“标准成本”字段中的“添加价格”必须为数字'));
	//	set_focus('add_pct');
	//	$input_error = 1;
	//}	
	if (isset($_FILES['pic']) && $_FILES['pic']['name'] != '') {
		if ($_FILES['pic']['error'] == UPLOAD_ERR_INI_SIZE) {
			display_error(_('文件大小超过了允许的最大值。'));
			$input_error = 1;
		}
		elseif ($_FILES['pic']['error'] > 0) {
			display_error(_('上载徽标文件时出错。'));
			$input_error = 1;
		}
		$result = $_FILES['pic']['error'];
		$filename = company_path().'/images';
		if (!file_exists($filename))
			mkdir($filename);

		$filename .= '/'.clean_file_name($_FILES['pic']['name']);

		 //But check for the worst
		if (!in_array( substr($filename,-4), array('.jpg', '.JPG', '.png', '.PNG'))) {
			display_error(_('只支持jpg和png文件——文件扩展名为.jpg或.png'));
			$input_error = 1;
		}
		elseif ( $_FILES['pic']['size'] > ($SysPrefs->max_image_size * 1024)) { //File Size Check
			display_error(_('文件大小超过了允许的最大值。允许的最大大小（KB）为').' '.$SysPrefs->max_image_size);
			$input_error = 1;
		}
		elseif ( $_FILES['pic']['type'] == 'text/plain' ) {  //File type Check
			display_error( _('只能上传图形文件'));
			$input_error = 1;
		}
		elseif (file_exists($filename)) {
			$result = unlink($filename);
			if (!$result) {
				display_error(_('无法删除现有图片'));
				$input_error = 1;
			}
		}

		if ($input_error != 1) {
			$result  =  move_uploaded_file($_FILES['pic']['tmp_name'], $filename);
			$_POST['coy_logo'] = clean_file_name($_FILES['pic']['name']);
			if(!$result) {
				display_error(_('上传徽标文件时出错'));
				$input_error = 1;
			}
			else {
				$msg = check_image_file($filename);
				if ( $msg) {
					display_error( $msg);
					unlink($filename);
					$input_error = 1;
				}
			}
		}
	}
	if (check_value('del_coy_logo')) {
		$filename = company_path().'/images/'.clean_file_name($_POST['coy_logo']);
		if (file_exists($filename)) {
			$result = unlink($filename);
			if (!$result) {
				display_error(_('无法删除当前的图像'));
				$input_error = 1;
			}
		}
		$_POST['coy_logo'] = '';
	}
	//if ($_POST['add_pct'] == '')
	//	$_POST['add_pct'] = -1;
	//if ($_POST['round_to'] <= 0)
	//	$_POST['round_to'] = 1;
	if ($input_error != 1) {
		update_company_prefs(
			get_post(
				array('coy_name', 'coy_no', 'gst_no', 'tax_prd', 'tax_last', 'postal_address', 'phone', 'fax', 'email', 'coy_logo', 'domicile', 'use_dimension', 'curr_default', 'f_year', 'shortname_name_in_list', 'no_customer_list'=>0, 'no_supplier_list'=>0, 'base_sales', 'ref_no_auto_increase'=>0, 'dim_on_recurrent_invoice'=>0, 'long_description_invoice'=>0, 'max_days_in_docs'=>180, 'time_zone'=>0, 'company_logo_report'=>0, 'barcodes_on_stock'=>0, 'print_dialog_direct'=>0, 'add_pct', 'round_to', 'login_tout', 'auto_curr_reval', 'bcc_email', 'alternative_tax_include_on_docs', 'suppress_tax_rates', 'use_manufacturing', 'use_fixed_assets')
			)
		);

		$_SESSION['wa_current_user']->timeout = $_POST['login_tout'];
		display_notification_centered(_('公司信息已更新'));
		set_focus('coy_name');
		$Ajax->activate('_page_body');
	}
}

start_form(true);



$myrow = get_company_prefs();

$_POST['coy_name'] = $myrow['coy_name'];
$_POST['gst_no'] = $myrow['gst_no'];
$_POST['tax_prd'] = $myrow['tax_prd'];
$_POST['tax_last'] = $myrow['tax_last'];
$_POST['coy_no']  = $myrow['coy_no'];
$_POST['postal_address']  = $myrow['postal_address'];
$_POST['phone']  = $myrow['phone'];
$_POST['fax']  = $myrow['fax'];
$_POST['email']  = $myrow['email'];
$_POST['coy_logo']  = $myrow['coy_logo'];
$_POST['domicile']  = $myrow['domicile'];
$_POST['use_dimension']  = $myrow['use_dimension'];
$_POST['base_sales']  = $myrow['base_sales'];

if (!isset($myrow['shortname_name_in_list'])) {
	set_company_pref('shortname_name_in_list', 'setup.company', 'tinyint', 1, '0');
	$myrow['shortname_name_in_list'] = get_company_pref('shortname_name_in_list');
}

$_POST['shortname_name_in_list']  = $myrow['shortname_name_in_list'];
$_POST['no_customer_list']  = $myrow['no_customer_list'];
$_POST['no_supplier_list']  = $myrow['no_supplier_list'];
$_POST['curr_default']  = $myrow['curr_default'];
$_POST['f_year']  = $myrow['f_year'];
$_POST['time_zone']  = $myrow['time_zone'];

if (!isset($myrow['max_days_in_docs'])) {
	set_company_pref('max_days_in_docs', 'setup.company', 'smallint', 5, '180');
	$myrow['max_days_in_docs'] = get_company_pref('max_days_in_docs');
}
$_POST['max_days_in_docs']  = $myrow['max_days_in_docs'];
if (!isset($myrow['company_logo_report'])) {
	set_company_pref('company_logo_report', 'setup.company', 'tinyint', 1, '0');
	$myrow['company_logo_report'] = get_company_pref('company_logo_report');
}
$_POST['company_logo_report']  = $myrow['company_logo_report'];
if (!isset($myrow['ref_no_auto_increase'])) {
	set_company_pref('ref_no_auto_increase', 'setup.company', 'tinyint', 1, '0');
	$myrow['ref_no_auto_increase'] = get_company_pref('ref_no_auto_increase');
}
$_POST['ref_no_auto_increase']  = $myrow['ref_no_auto_increase'];
if (!isset($myrow['barcodes_on_stock'])) {
	set_company_pref('barcodes_on_stock', 'setup.company', 'tinyint', 1, '0');
	$myrow['barcodes_on_stock'] = get_company_pref('barcodes_on_stock');
}
$_POST['barcodes_on_stock']  = $myrow['barcodes_on_stock'];
if (!isset($myrow['print_dialog_direct'])) {
	set_company_pref('print_dialog_direct', 'setup.company', 'tinyint', 1, '0');
	$myrow['print_dialog_direct'] = get_company_pref('print_dialog_direct');
}
$_POST['print_dialog_direct']  = $myrow['print_dialog_direct'];
if (!isset($myrow['dim_on_recurrent_invoice'])) {
	set_company_pref('dim_on_recurrent_invoice', 'setup.company', 'tinyint', 1, '0');
	$myrow['dim_on_recurrent_invoice'] = get_company_pref('dim_on_recurrent_invoice');
}
$_POST['dim_on_recurrent_invoice']  = $myrow['dim_on_recurrent_invoice'];
if (!isset($myrow['long_description_invoice'])) {
	set_company_pref('long_description_invoice', 'setup.company', 'tinyint', 1, '0');
	$myrow['long_description_invoice'] = get_company_pref('long_description_invoice');
}
$_POST['long_description_invoice']  = $myrow['long_description_invoice'];
$_POST['version_id']  = $myrow['version_id'];
$_POST['add_pct'] = $myrow['add_pct'];
$_POST['login_tout'] = $myrow['login_tout'];
if ($_POST['add_pct'] == -1)
	$_POST['add_pct'] = '';
$_POST['round_to'] = $myrow['round_to'];	
$_POST['auto_curr_reval'] = $myrow['auto_curr_reval'];	
$_POST['del_coy_logo']  = 0;
$_POST['bcc_email']  = $myrow['bcc_email'];
$_POST['alternative_tax_include_on_docs']  = $myrow['alternative_tax_include_on_docs'];
$_POST['suppress_tax_rates']  = $myrow['suppress_tax_rates'];
$_POST['use_manufacturing']  = $myrow['use_manufacturing'];
$_POST['use_fixed_assets']  = $myrow['use_fixed_assets'];

start_outer_table(TABLESTYLE2);

table_section(1);
table_section_title(_('一般设置'));

text_row_ex(_('公司名称（显示在报告上）:'), 'coy_name', 50, 50);
textarea_row(_('地址:'), 'postal_address', $_POST['postal_address'], 40, 2);
text_row_ex(_('经营范围:'), 'domicile', 25, 55);

text_row_ex(_('移动电话:'), 'phone', 25, 55);
text_row_ex(_('传真:'), 'fax', 25);
email_row_ex(_('Email:'), 'email', 50, 55);

//email_row_ex(_('BCC Address for all outgoing mails:'), 'bcc_email', 50, 55);

text_row_ex(_('办公电话:'), 'coy_no', 25);
//text_row_ex(_('GSTNo:'), 'gst_no', 25);
//currencies_list_row(_('本币:'), 'curr_default', $_POST['curr_default']);

//label_row(_('公司Logo:'), $_POST['coy_logo']);
//file_row(_('新公司Logo (.jpg)') . ':', 'pic', 'pic');
//check_row(_('删除的公司Logo:'), 'del_coy_logo', $_POST['del_coy_logo']);

//check_row(_('时区:'), 'time_zone', $_POST['time_zone']);
//check_row(_('报表中显示logo'), 'company_logo_report', $_POST['company_logo_report']);
//check_row(_('报表中显示条形码'), 'barcodes_on_stock', $_POST['barcodes_on_stock']);
//check_row(_('自动增加报表的单号:'), 'ref_no_auto_increase', $_POST['ref_no_auto_increase']);
//check_row(_('发票上显示需求内容:'), 'dim_on_recurrent_invoice', $_POST['dim_on_recurrent_invoice']);
//check_row(_('发票上显示说明内容:'), 'long_description_invoice', $_POST['long_description_invoice']);


table_section(2);

//table_section_title(_('财务相关'));
//fiscalyears_list_row(_('会计年度:'), 'f_year', $_POST['f_year']);
//text_row_ex(_('税期: '), 'tax_prd', 10, 10, '', null, null, _('月'));
//text_row_ex(_('税收去年同期:'), 'tax_last', 10, 10, '', null, null, _('几个月前'));
//check_row(_('是否显示免税:'), 'alternative_tax_include_on_docs', null);
//check_row(_('降低单据的税率:'), 'suppress_tax_rates', null);
//check_row(_('自动重估货币账户:'), 'auto_curr_reval', $_POST['auto_curr_reval']);

table_section_title(_('其他项目'));
label_row(_('数据库版本:'), $_POST['version_id']);
//sales_types_list_row(_('自动计算相应的价格:'), 'base_sales', $_POST['base_sales'], false, _('没有基础价格表') );

//text_row_ex(_('新增成本价格从std:'), 'add_pct', 10, 10, '', null, null, '%');
//hidden('add_pct');

//$curr = '';//get_currency($_POST['curr_default']);
text_row_ex(_('小数点保留:'), 'round_to', 10, 10, '', null, null, 0);
label_row('', '&nbsp;');


//table_section_title(_('可选'));
//check_row(_('生产:'), 'use_manufacturing', null);
//check_row(_('资产').':', 'use_fixed_assets', null);
//number_list_row(_('用户需求:'), 'use_dimension', null, 0, 2);

table_section_title(_('用户界面选项'));

//check_row(_('列表中显示短名称:'), 'shortname_name_in_list', $_POST['shortname_name_in_list']);
//check_row(_('打开直接打印报告对话框:'), 'print_dialog_direct', null);
//check_row(_('查询客户列表:'), 'no_customer_list', null);
//check_row(_('查询供货商列表:'), 'no_supplier_list', null);
text_row_ex(_('登录超时:'), 'login_tout', 10, 10, '', null, null, _('秒'));
text_row_ex(_('文档中的最大日范围：'), 'max_days_in_docs', 10, 10, '', null, null, _('天'));

end_outer_table(1);

hidden('coy_logo', $_POST['coy_logo']);
submit_center('update', _('更新信息'), true, '',  'default');

end_form(2);

end_page();
