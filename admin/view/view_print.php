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
include_once($path_to_root . '/admin/view/view_print_manager.php');

//----------------------------------------------------------------------------------

function get_jasper_upload_file($name)
{

	$filename = basename($_FILES[$name]['name']);

	$extension = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
	if ($filename && !in_array($extension, array('jrxml', 'jasper'))) {
		display_error(_('只能上传 jrxml 文件'));
	} elseif (!isset($_FILES[$name])) {
		display_error(_('选择询价文件'));
	} elseif (($_FILES[$name]['error'] > 0)) {
		if ($_FILES[$name]['error'] == UPLOAD_ERR_INI_SIZE) {
			display_error(_('文件大小超出限制'));
		} else {
			display_error(_('选择附件文件.'));
		}
	} elseif (strlen($filename) > 60) {
		display_error(_('文件名长度大于60个字符'));
	} else {

		$tmpname = $_FILES[$name]['tmp_name'];
		$dir = company_path() . '/reports';

		if (!file_exists($dir)) {
			mkdir($dir, 0777);
			$index_file = "<?php\nheader(\"Location: ../index.php\");\n";
			$fp = fopen($dir . '/index.php', 'w');
			fwrite($fp, $index_file);
			fclose($fp);
		}

		$filesize = $_FILES[$name]['size'];
		$filetype = $_FILES[$name]['type'];

		$unique_name = $name; //random_id();
		$file_path = $dir . '/' . $unique_name . '.' . $extension;

		move_uploaded_file($tmpname, $file_path);
		return $file_path;
	}
}


//---------------------------------------------------------------------------------- 


start_form(true);

//----------------------------------------------------------------------------------
//div_start('details');

$printManager = new PrintManager('print_manager', 'jasper_file', '');
$printManager->show();

//div_end();


//submit_add_or_update_center($selected_id == -1, '', 'both', true);

end_form();
br(2);

end_page();
