<?php
$width = 100;
$limit = 10;

$title = sprintf(_("最近未完成的的前 %d 项任务"), $limit);

$widget = new Widget();
$widget->setTitle($title);
$widget->Start();

$sql = "select title,begin_at,end_at from ".TB_PREF."tasks where isfinished=0 limit 0,".$limit;
$result = db_query($sql,'');

if($widget->checkSecurity('SA_CUSTOMER_LEADS')) {
	$th = array(_('活动内容 '),_('开始日期'),_('截止日期'));
	start_table(TABLESTYLE, "width='$width%'");
	table_header($th);
	$k = 0; //row colour counter
	while ($myrow = db_fetch($result)) {
		alt_table_row_color($k);
		$name = $myrow['title'];
		$begin_at = $myrow['begin_at'];
		$end_at = $myrow['end_at'];
		label_cell($name);
		label_cell(sql2date($begin_at),'style="width:100px;text-align:center;"');
		label_cell(sql2date($end_at),'style="width:100px;text-align:center;"');
		end_row();
	}
	end_table();
}

$widget->End();

?>