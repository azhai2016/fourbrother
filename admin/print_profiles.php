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
$page_security = 'SA_PRINTPROFILE';
$path_to_root = '..';
include $path_to_root . '/includes/session.php';
include $path_to_root . '/admin/db/printers_db.inc';
include $path_to_root . '/includes/ui.inc';

page(_($help_context = '打印配置文件'));

$selected_id = get_post('profile_id', '');

//-------------------------------------------------------------------------------------------------
// Returns array of defined reports
//
function get_reports()
{
    global $path_to_root, $SysPrefs;

    if ($SysPrefs->go_debug || !isset($_SESSION['reports'])) {
        // to save time, store in session.
        $paths = array($path_to_root . '/reporting/', company_path() . '/reporting/');
        $reports = array('' => _('默认打印目标'));

        foreach ($paths as $dirno => $path) {
            $repdir = opendir($path);
            while (false !== ($fname = readdir($repdir))) {
                // reports have filenames in form rep(repid).php
                // where repid must contain at least one digit (reports_main.php is not ;)
                if (is_file($path . $fname) && preg_match('/rep(.*[0-9]+.*)[.]php/', $fname, $match)) {
                    $repno = $match[1];
                    $title = '';
                    $line = file_get_contents($path . $fname);

                    if (preg_match('/.*(FrontReport\()\s*_\([\'"]([^\'"]*)/', $line, $match)) {
                        $title = trim($match[2]);
                    }

                    // for any 3rd party printouts without FrontReport() class use
                    else if (preg_match('/.*(\$Title).*[\'"](.*)[\'"].+/', $line, $match)) {
                        $title = trim($match[2]);
                    }

                    $reports[$repno] = $title;
                }
            }
            closedir();
        }
        ksort($reports);
        $_SESSION['reports'] = $reports;
    }
    return $_SESSION['reports'];
}

function clear_form()
{
    global $selected_id, $Ajax;

    $selected_id = '';
    $_POST['name'] = '';
    $Ajax->activate('_page_body');
}

function check_delete($name)
{
    // check if selected profile is used by any user
    if ($name == '') {
        return 0;
    }
    // cannot delete system default profile
    return key_in_foreign_table($name, 'users', 'print_profile');
}

//-------------------------------------------------------------------------------------------

if (get_post('submit')) {

    $error = 0;

    if ($_POST['profile_id'] == '' && empty($_POST['name'])) {
        $error = 1;
        display_error(_('打印配置名不能为空'));
        set_focus('name');
    }
    if (!$error) {
        $prof = array('' => get_post('Prn')); // store default value/profile name
        foreach (get_reports() as $rep => $descr) {
            $val = get_post('Prn' . $rep);
            $prof[$rep] = $val;
        }
        if ($_POST['profile_id'] == '') {
            $_POST['profile_id'] = get_post('name');
        }

        update_printer_profile($_POST['profile_id'], $prof);
        if ($selected_id == '') {
            display_notification_centered(_('新打印配置文件创建成功'));
            clear_form($selected_id);
        } else {
            display_notification_centered(_('打印机配置信息已更新'));
        }

    }
}

if (get_post('delete')) {
    if (!check_delete(get_post('name'))) {
        delete_printer_profile($selected_id);
        display_notification(_('已删除选择的配置信息'));
        clear_form();
    }
}

if (get_post('_profile_id_update')) {
    $Ajax->activate('_page_body');
}

start_form();
start_table();
print_profiles_list_row(_('选择打印配置') . ':', 'profile_id', null, _('新打印配置'), true);
end_table();
echo '<hr>';
start_table();
if (get_post('profile_id') == '') {
    text_row(_('配置名称') . ':', 'name', null, 30, 30);
} else {
    label_cells(_('配置名称') . ':', get_post('profile_id'));
}

end_table(1);

$result = get_print_profile(get_post('profile_id'));
$prints = array();
while ($myrow = db_fetch($result)) {
    $prints[$myrow['report']] = $myrow['printer'];
}

start_table(TABLESTYLE);
$th = array(_('打印机ID'), _('说明'), _('打印机'));
table_header($th);

$k = 0;
$unkn = 0;
foreach (get_reports() as $rep => $descr) {
    alt_table_row_color($k);

    label_cell($rep == '' ? '-' : $rep, 'align=center');
    label_cell($descr == '' ? '???<sup>1)</sup>' : _($descr));
    $_POST['Prn' . $rep] = isset($prints[$rep]) ? $prints[$rep] : '';
    echo '<td>';
    echo printers_list('Prn' . $rep, null,
        $rep == '' ? _('浏览') : _('默认'));
    echo '</td>';
    if ($descr == '') {
        $unkn = 1;
    }

    end_row();
}
end_table();
if ($unkn) {
    display_note('<sup>1)</sup>&nbsp;-&nbsp;' . _('在此报表定义文件中找不到标题。'), 0, 1, '');
} else {
    echo '<br>';
}

div_start('controls');
if (get_post('profile_id') == '') {
    submit_center('submit', _('添加新配置'), true, '', 'default');
} else {
    submit_center_first('submit', _('更新配置'), _('更新打印机配置'), 'default');
    submit_center_last('delete', _('删除配置'), _('删除打印机配置信息'), true);
}
div_end();

end_form();
end_page();
