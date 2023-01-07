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
$page_security = 'SA_CHGPASSWD';
$path_to_root = '..';
include_once $path_to_root . '/includes/session.php';

page(_($help_context = '修改密码'));

include_once $path_to_root . '/includes/date_functions.inc';
include_once $path_to_root . '/includes/ui.inc';

include_once $path_to_root . '/admin/db/users_db.inc';

function can_process()
{

    $Auth_Result = hook_authenticate($_SESSION['wa_current_user']->username, $_POST['cur_password']);

    if (!isset($Auth_Result)) // if not used external login: standard method
    {
        $Auth_Result = get_user_auth($_SESSION['wa_current_user']->username, md5($_POST['cur_password']));
    }

    if (!$Auth_Result) {
        display_error(_('输入的密码无效.'));
        set_focus('cur_password');
        return false;
    }
    if (strlen($_POST['password']) < 4) {
        display_error(_('输入的密码长度必须至少为4个字符'));
        set_focus('password');
        return false;
    }
    if (strstr($_POST['password'], $_SESSION['wa_current_user']->username) != false) {
        display_error(_('密码不能包含用户登录名'));
        set_focus('password');
        return false;
    }
    if ($_POST['password'] != $_POST['passwordConfirm']) {
        display_error(_('输入的密码不一样。'));
        set_focus('password');
        return false;
    }

    return true;
}

if (isset($_POST['UPDATE_ITEM']) && check_csrf_token()) {

    if (can_process()) {
        if ($SysPrefs->allow_demo_mode) {
            display_warning(_('无法在演示模式下更改密码。'));
        } else {
            update_user_password($_SESSION['wa_current_user']->user, $_SESSION['wa_current_user']->username, md5($_POST['password']));
            display_notification(_('您的密码已更新。'));
        }
        $Ajax->activate('_page_body');
    }
}

start_form();

start_table(TABLESTYLE);

$myrow = get_user($_SESSION['wa_current_user']->user);

label_row(_('用户名:'), $myrow['user_id']);

$_POST['cur_password'] = '';
$_POST['password'] = '';
$_POST['passwordConfirm'] = '';

password_row(_('原密码:'), 'cur_password', $_POST['cur_password']);
password_row(_('新密码:'), 'password', $_POST['password']);
password_row(_('再输入密码:'), 'passwordConfirm', $_POST['passwordConfirm']);

table_section_title(_('输入新密码'));

end_table(1);

submit_center('UPDATE_ITEM', _('更新密码'), true, '', 'default');
end_form();
end_page();
