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
date_default_timezone_set("Asia/Shanghai");
 $path_to_root = '.';

if (!file_exists($path_to_root . '/config_db.php')) {
    header('Location: ' . $path_to_root . '/install/index.php');
}

$page_security = 'SA_OPEN';


require $path_to_root.'/vendor/autoload.php';

include_once $path_to_root.'/includes/route.php';

include_once $path_to_root.'/includes/calendar.php';

//增加路由功能
$route = new Route();

include_once 'includes/session.php';

$route_list = array(
    array('GET', '/system', 'system'),
);

add_access_extensions();

log_b($installed_extensions);

if (isset($installed_extensions)){
    
    foreach ($installed_extensions as $rows) {
          if ($rows['active']==1) {   
             $route_list[]=array('GET','/'.$rows['name'],$rows['name']);
          }
    }
}

$application = $route->Lite($route_list);

$app = &$_SESSION['App'];
//if (isset($_GET['application'])) {
$app->selected_application = $application;//$_GET['application'];
//}
$app->display();
