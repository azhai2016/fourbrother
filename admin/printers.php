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
$page_security = 'SA_PRINTERS';
$path_to_root = '..';
include $path_to_root . '/includes/session.php';

page(_($help_context = '打印机设置'));

include $path_to_root . '/admin/db/printers_db.inc';
include $path_to_root . '/includes/ui.inc';

simple_page_mode(true);

//-------------------------------------------------------------------------------------------

if ($Mode == 'ADD_ITEM' || $Mode == 'UPDATE_ITEM') {

    $error = 0;

    if (empty($_POST['name'])) {
        $error = 1;
        display_error(_('打印机名称不能为空'));
        set_focus('name');
    } elseif (empty($_POST['host'])) {
        display_notification_centered(_('您已选择在用户IP上打印到服务器。'));
    } elseif (!check_num('tout', 0, 60)) {
        $error = 1;
        display_error(_('超时时间不能小于零，也不能超过60秒。'));
        set_focus('tout');
    }

    if ($error != 1) {
        write_printer_def($selected_id, get_post('name'), get_post('descr'), get_post('queue'), get_post('host'), input_num('port', 0), input_num('tout', 0));

        display_notification_centered($selected_id == -1 ? _('已创建新的打印机设置') : _('选定的打印机设置已更新'));
        $Mode = 'RESET';
    }
}

if ($Mode == 'Delete') {
    // PREVENT DELETES IF DEPENDENT RECORDS IN print_profiles

    if (key_in_foreign_table($selected_id, 'print_profiles', 'printer')) {
        display_error(_('无法删除此打印机配置，因为已使用它创建了打印配置文件。'));
    } else {
        delete_printer($selected_id);
        display_notification(_('已删除此打印机配置。'));
    }
    $Mode = 'RESET';
}

if ($Mode == 'RESET') {
    $selected_id = -1;
    unset($_POST);
}

//-------------------------------------------------------------------------------------------------

$result = get_all_printers();
start_form();
start_table(TABLESTYLE);
$th = array(_('名称'), _('说明'), _('服务地址'), _('打印队列'), '', '');
table_header($th);

$k = 0; //row colour counter
while ($myrow = db_fetch($result)) {
    alt_table_row_color($k);

    label_cell($myrow['name']);
    label_cell($myrow['description']);
    label_cell($myrow['host']);
    label_cell($myrow['queue']);
    edit_button_cell('Edit' . $myrow['id'], _('编辑'));
    delete_button_cell('Delete' . $myrow['id'], _("删除"));
    end_row();
}

end_table();
end_form();
echo '<br>';

//-------------------------------------------------------------------------------------------------

start_form();

start_table(TABLESTYLE2);

if ($selected_id != -1) {
    if ($Mode == 'Edit') {
        $myrow = get_printer($selected_id);
        $_POST['name'] = $myrow['name'];
        $_POST['descr'] = $myrow['description'];
        $_POST['queue'] = $myrow['queue'];
        $_POST['tout'] = $myrow['timeout'];
        $_POST['host'] = $myrow['host'];
        $_POST['port'] = $myrow['port'];
    }
    hidden('selected_id', $selected_id);
} else {
    if (!isset($_POST['host'])) {
        $_POST['host'] = 'localhost';
    }

    if (!isset($_POST['port'])) {
        $_POST['port'] = '515';
    }

}

text_row(_('打印机名称') . ':', 'name', null, 20, 20);
text_row(_('打印机说明') . ':', 'descr', null, 40, 60);
text_row(_('打印服务器IP') . ':', 'host', null, 30, 40);
text_row(_('端口') . ':', 'port', null, 5, 5);
text_row(_('打印队列') . ':', 'queue', null, 20, 20);
text_row(_('超时') . ':', 'tout', null, 5, 5);

end_table(1);

submit_add_or_update_center($selected_id == -1, '', 'both');

end_form();

end_page();
