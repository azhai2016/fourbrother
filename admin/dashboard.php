
<?php
/**********************************************************************
	Copyright (C) NotrinosERP.
	Released under the terms of the GNU General Public License, GPL, 
	as published by the Free Software Foundation, either version 3 
	of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
	See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
***********************************************************************/

$page_security = 'SA_OPEN';
$path_to_root = '..';

include_once($path_to_root . '/includes/session.php');
include_once($path_to_root . '/includes/ui.inc');
include_once($path_to_root . '/reporting/includes/class.graphic.inc');
include_once($path_to_root . '/dashboard/includes/dashboard_classes.inc');
include_once($path_to_root.'/includes/date_functions.inc');

include_once $path_to_root . '/customers/db/leads_db.inc';


$js = '';
if ($SysPrefs->use_popup_windows)
	$js .= get_js_open_window(800, 500);

page(_($help_context = '仪表盘'), false, false, '', $js);

if (isset($_GET['sel_app'])) {

	$selected_app = $_GET['sel_app'];


	if (!$_SESSION['wa_current_user']->check_application_access($selected_app))
		return;


	$dir = company_path().'/pdf_files';
    if (!is_dir($dir)){
       mkdir($dir,0777);
	}

	if ($d = @opendir($dir)) {
	
		while (($file = readdir($d)) !== false) {
	
			if (!is_file($dir.'/'.$file) || $file == 'index.php') continue;
			$ftime = filemtime($dir.'/'.$file);

			if (time()-$ftime > 180)
				unlink($dir.'/'.$file);
		}
		closedir($d);
	}
	
	
	$dashboard = new Dashboard;

	$dashboard->addDashboard(_('联系人'), DA_CUSTOMER);
	$dashboard->addWidget(DA_CUSTOMER, 101, WIDGET_HALF);
	$dashboard->addWidget(DA_CUSTOMER, 102, WIDGET_HALF);
 
	$dashboard->addDashboard(_('分析'), DA_STATISTICE);
	$dashboard->addWidget(DA_STATISTICE, 704, WIDGET_HALF);
	$dashboard->addWidget(DA_STATISTICE, 705, WIDGET_HALF);
	
	add_custom_dashboards($dashboard);
	echo $dashboard->display();


	
}
else {
	display_error(_('没有内容'));
}

end_page();


?>