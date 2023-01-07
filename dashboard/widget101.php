<?php
$width = 100;
$limit = 10;

$title = sprintf(_("最近更新的前 5 位"), $limit);

$widget = new Widget();
$widget->setTitle($title);
$widget->Start();

$result = get_leads_top_five_data();

if($widget->checkSecurity('SA_CUSTOMER_LEADS')) {
	$th = array(_('姓名'));
	start_table(TABLESTYLE, "width='$width%'");
	table_header($th);
	$k = 0; //row colour counter
	while ($myrow = db_fetch($result)) {
		alt_table_row_color($k);
		$name = $myrow['name'];
		label_cell($name);
		end_row();
	}
	end_table();
}

$widget->End();

?>