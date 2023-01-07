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

define('DA_CUSTOMER', 'customer');
define('DA_STATISTICE', 'get_statistice');
define('DA_INVENTORY', 'stock');
define('DA_MANUFACTURE', 'manuf');
define('DA_DIMENSIONS', 'proj');
define('DA_GL', 'GL');
define('DA_FIXEDASSETS', 'assets');
define('DA_HRM', 'FrontHrm');
define('DA_SETUP', 'system');

define('WIDGET_FULL', 'widget_full');
define('WIDGET_HALF', 'widget_half');

class Dashboard {

	var $ar_dashboards;
	var $ar_widgets;

	function __construct() {
		$this->ar_dashboards = array();
	}

	function addDashboard($dashboard_name, $dashboard_id) {
		if (!$dashboard_id)
			$dashboard_id = 'orders';
	
		$this->ar_dashboards[$dashboard_id] = $dashboard_name;

		return $dashboard_id;
	}

	function addWidget($dashboard, $widget_id, $width=WIDGET_FULL) {

		unset($this->ar_widgets[$dashboard][$widget_id]);
		$this->ar_widgets[$dashboard][$widget_id] = array('width'=>$width);
	}

	function display($class=null) {
		global $path_to_root;

		if (isset($_GET['sel_app']))
			$cur_app = $_GET['sel_app'];
		elseif(isset($_GET['application']))
			$cur_app = $_GET['application'];
		else
			$cur_app = user_startup_tab();

 
		echo "<div class='widgets_container'>";

		foreach($this->ar_dashboards as $dashboard_id => $name) {
		
			if($cur_app == $dashboard_id) {

				foreach($this->ar_widgets[$dashboard_id] as $widget_id => $widget) {
					$widg = preg_replace('/[^a-z_0-9]/i', '', $widget_id);
					$widget_file = find_custom_file("/dashboard/widget$widg.php");

					if($widget['width'] == WIDGET_FULL)
						$class = 'widget_full';
					elseif($widget['width'] == WIDGET_HALF)
						$class = 'widget_half';

					echo "<div class = 'widget $class'>";

				

					if($widget_file !== null)
						require($widget_file);
					else
						echo "<br/><br/><div class = 'err_msg'>"._('Cannot find widget file').'&nbsp;'."'$widg'"."</div>";
					
					echo '</div>';
				}
			}
		}
		echo '</div>';
	}
}

class Widget {

	var $title;

	function __construct($title='') {
		$this->title = $title;
	}

	function setTitle($title) {
		$this->title = $title;
	}

	function checkSecurity($area) {

		if (!$_SESSION['wa_current_user']->can_access_page($area)) {
			echo '<center><br><br><br><b>';
			echo _('The security settings on your account do not permit you to access this function');
			echo '</b>';
			echo '<br><br><br><br></center>';

			return false;
		}
		return true;
	}

	function Start() {
		$title = $this->title;

		echo "<div class = 'widget_title'>";
		echo '<span>'.$title.'</span>';
		echo '</div>';
		echo "<div class = 'widget_content'>";
	}

	function End() {
		echo '</div>';
	}
}

function add_custom_dashboards($dashboard) {
	global $installed_extensions, $path_to_root;

	if (count($installed_extensions) > 0) {

		$extensions = $installed_extensions;
		foreach ($extensions as $ext) {
			if (($ext['active'] && $ext['type'] == 'extension')) {
				$file = $path_to_root.'/'.$ext['path'].'/dashboard/custom_dashboards.php';
				if (file_exists($file)) {
					set_ext_domain($ext['path']);
					include_once($file);
					set_ext_domain();
				}
			}
		}
	}
	$file = company_path().'/dashboard/custom_dashboards.php';
	if (file_exists($file))
		include_once($file);
}


function source_graphic($title, $x_axis, $pg, $graphic1, $graphic2='', $type=2) {

	$pg->axis_x    = $x_axis;
	$pg->axis_y    = _('金额');
	$pg->graphic_1 = empty($graphic1) ? '' : $graphic1;
	$pg->graphic_2 = empty($graphic2) ? '' : $graphic2;
	$pg->type      = $type;
	$pg->skin      = 1;
	$pg->built_in  = false;
	$filename = company_path().'/pdf_files/'.random_id().'.png';
	$pg->display($filename, true);

	echo "<center><img src='$filename' border='0' alt='$title'></center>";
}

