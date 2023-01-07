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


$weeks = 0;

//$today = now();

$sql = "select d.item_code,i.item_name,sum(qty*sale_price) as amount
          from  ".TB_PREF."sales_orders_master m left join 
		  ".TB_PREF."sales_orders_detail d on m.order_no=d.order_no LEFT JOIN 
		  ".TB_PREF."item_codes i on i.id=d.item_id left join 
		  ".TB_PREF."customers c on c.id= m.customer_id 
         where 1=1 ";

if ($_SESSION['wa_current_user']->sale_area_code) {
	  $sale_area_code = $_SESSION['wa_current_user']->sale_area_code;
	  $sql.="c.`sale_area_code`=".$sale_area_code;
}		 	

$sql.="group by d.item_code,i.item_name";


$result = db_query($sql);

$title = sprintf(_("商品销售市场占比图"), $weeks);

$i = 0;
//while ($myrow = $result) {
foreach ($result as $myrow) {   
	$pg->x[$i] = $myrow['item_name']; 
	$pg->y[$i] = $myrow['amount'];
	$pg->z[$i] = mb_substr($myrow['item_name'],0,5); 
	$i++;
}

$widget = new Widget();
$widget->setTitle($title);
$widget->Start();

if($widget->checkSecurity('SA_CUSTOMER_LEADS')) {
    source_graphic($title, _(''), $pg, _('销售'), _('计划'), 5);
}

$widget->End();