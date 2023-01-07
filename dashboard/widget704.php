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


$weeks = 10;


if ($_SESSION['wa_current_user']->sale_area_code);
$sale_area_code = $_SESSION['wa_current_user']->sale_area_code;

//$today = now();
$sql = "select c.`code`,c.`name`,sum(m.amount) as amount,
ifnull((select sum(amount) from ".TB_PREF."customers_plan p 
        where p.customer_id=m.customer_id GROUP BY customer_id),0) as sale_plan
from ".TB_PREF."sales_orders_master m left join 
     ".TB_PREF."customers c on c.`id`=m.`customer_id` 
where 1=1 ";

if ($_SESSION['wa_current_user']->sale_area_code) {
	$sale_area_code = $_SESSION['wa_current_user']->sale_area_code;
 	$sql.="c.`sale_area_code`=".$sale_area_code;
}

$sql.= "group by c.`code`,c.`name`
order by sum(m.amount) desc 
limit 0,$weeks";
$result = db_query($sql);

/* array(
    array('week_name'=>1,'sales'=>100,'costs'=>2.30),
    array('week_name'=>2,'sales'=>50,'costs'=>21.30),
    array('week_name'=>3,'sales'=>150,'costs'=>121.30),
    array('week_name'=>4,'sales'=>850,'costs'=>221.30)

); */ //gl_performance($today, $weeks);

$title = sprintf(_("前 %s 家销售示意图"), $weeks);

$i = 0;
//while ($myrow = $result) {
foreach ($result as $myrow) {   
	$pg->x[$i] = $myrow['name']; 
	$pg->y[$i] = $myrow['amount'];
	$pg->z[$i] = $myrow['sale_plan'];
	$i++;
}

$widget = new Widget();
$widget->setTitle($title);
$widget->Start();

if($widget->checkSecurity('SA_CUSTOMER_LEADS')) {
    source_graphic($title, _('客户'), $pg, _('销售'), _('计划'), 1);
}

$widget->End();