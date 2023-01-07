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

$pg = new graph();


$weeks = 4;

//$today = now();

$result = array(
    array('week_name'=>1,'sales'=>100,'costs'=>2.30),
    array('week_name'=>2,'sales'=>50,'costs'=>21.30),
    array('week_name'=>3,'sales'=>150,'costs'=>121.30),
    array('week_name'=>4,'sales'=>350,'costs'=>221.30)

); //gl_performance($today, $weeks);

$title = sprintf(_("前 %s 周市场分布图"), $weeks);

$i = 0;
//while ($myrow = $result) {
foreach ($result as $myrow) {   
	$pg->x[$i] = $myrow['week_name']; 
	$pg->y[$i] = $myrow['sales'];
	$pg->z[$i] = $myrow['costs'];
	$i++;
}

$widget = new Widget();
$widget->setTitle($title);
$widget->Start();

if($widget->checkSecurity('SA_CUSTOMER_LEADS')) {
    source_graphic($title, _('周'), $pg, _('销售'), _('计划'), 3);
}

$widget->End();