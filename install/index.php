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
$page_security = 'SA_OPEN';
$path_to_root = '..';

if (file_exists($path_to_root . '/config_db.php')) {
    header("Location: $path_to_root/index.php");
}


include $path_to_root.'/install/isession.inc';



page(_('程序安装向导'), true, false, '', '', false, 'stylesheet.css');


include $path_to_root . '/includes/ui.inc';
include $path_to_root . '/includes/system_tests.inc';
include $path_to_root . '/admin/db/maintenance_db.inc';
include $path_to_root . '/includes/packages.inc';

if (file_exists($path_to_root . '/installed_extensions.php')) {
    include $path_to_root . '/installed_extensions.php';
}

//-------------------------------------------------------------------------------------------------

function subpage_title($txt)
{
    global $path_to_root;

    echo '<center><img src="' . $path_to_root . '/themes/default/images/fourbro.png" width="250" alt="Logo"></center>';

    $page = @$_POST['Page'] ? $_POST['Page'] : 1;

    display_heading(
        $page == 6 ? $txt :
        _('程序安装') . '<br>'
        . sprintf(_('第一步 %d: %s'), $page, $txt));
    br();
}

function display_coas()
{
    start_table(TABLESTYLE);
    $th = array(_('Chart of accounts'), _('编码'), _('说明'), _('安装'));
    table_header($th);

    $k = 0;
    $charts = get_charts_list();

    foreach ($charts as $pkg_name => $coa) {
        $available = @$coa['available'];
        $installed = @$coa['version'];
        $id = @$coa['local_id'];

        alt_table_row_color($k);
        label_cell($coa['name']);
        label_cell($coa['encoding']);
        label_cell(is_array($coa['Descr']) ? implode('<br>', $coa['Descr']) : $coa['Descr']);
        label_cell($installed ? _('安装') : checkbox(null, 'coas[' . $coa['package'] . ']'), "align='center'");

        end_row();
    }
    end_table(1);
}

function display_langs()
{
    start_table(TABLESTYLE);
    $th = array(_('Language'), _('Encoding'), _('Description'), _('Install'));
    table_header($th);

    $k = 0;
    $langs = get_languages_list();

    foreach ($langs as $pkg_name => $lang) {
        $available = @$lang['available'];
        $installed = @$lang['version'];
        $id = @$lang['local_id'];
        if (!$available) {
            continue;
        }

        alt_table_row_color($k);
        label_cell($lang['name']);
        label_cell($lang['encoding']);
        label_cell(is_array($lang['Descr']) ? implode('<br>', $lang['Descr']) : $lang['Descr']);
        label_cell($installed ? _('Installed') : checkbox(null, 'langs[' . $lang['package'] . ']'), "align='center'");
        end_row();
    }
    end_table(1);
}

function instlang_list_row($label, $name, $value = null)
{
    global $inst_langs;

    $langs = array();
    foreach ($inst_langs as $n => $lang) {
        $langs[$n] = $lang['name'];
    }

    echo "<td>" . $label . "</td>\n" . "<td>\n"
    . array_selector($name, $value, $langs,
        array(
            'select_submit' => true,
            'async' => true,
        )) . "</td>\n";
}

function install_connect_db()
{
    global $db;

    $conn = $_SESSION['inst_set'];

    $db = db_create_db($conn);
    if (!$db) {
        display_error(_('无法连接到数据库。用户或密码无效，或者您没有创建数据库的权限。'));
    } else {
        if (strncmp(db_get_version(), "5.6", 3) >= 0) {
            db_query("SET sql_mode = ''");
        }

    }
    return $db;
}

function do_install()
{

    global $path_to_root, $db_connections, $def_coy, $installed_extensions, $tb_pref_counter, $dflt_lang, $installed_languages;

    $coa = $_SESSION['inst_set']['coa'];
    if (install_connect_db() && db_import($path_to_root . '/sql/' . $coa, $_SESSION['inst_set'])) {
        $con = $_SESSION['inst_set'];
        $table_prefix = $con['tbpref'];

        $def_coy = 0;
        $tb_pref_counter = 0;
        $db_connections = array(0 => array(
            'name' => $con['name'],
            'host' => $con['host'],
            'port' => $con['port'],
            'dbname' => $con['dbname'],
            'collation' => $con['collation'],
            'tbpref' => $table_prefix,
            'dbuser' => $con['dbuser'],
            'dbpassword' => $con['dbpassword'],
        ));

        $_SESSION['wa_current_user']->cur_con = 0;

        update_company_prefs(array('coy_name' => $con['name']));
        $admin = get_user_by_login('admin');
        update_user_prefs($admin['id'], array(
            'language' => $con['lang'],
            'password' => md5($con['pass']),
            'user_id' => $con['admin']));

        if (!copy($path_to_root . '/config.default.php', $path_to_root . '/config.php')) {
            display_error(_("不能保存系统配置文件 'config.php'."));
            return false;
        }

        $err = write_config_db($table_prefix != '');

        if ($err == -1) {
            display_error(_("无法打开'config_db.php'配置文件."));
            return false;
        } else if ($err == -2) {
            display_error(_("无法写 'config_db.php' 配置文件."));
            return false;
        } else if ($err == -3) {
            display_error(_("配置文件'config_db.php' 无法写入. 授权后重新安装."));
            return false;
        }
        // update default language
        if (file_exists($path_to_root . '/lang/installed_languages.inc')) {
            include_once $path_to_root . '/lang/installed_languages.inc';
        }

        $dflt_lang = $_POST['lang'];
        write_lang();
        return true;
    }
    return false;
}

if (!isset($_SESSION['inst_set'])) // default settings

{
    $_SESSION['inst_set'] = array(
        'host' => 'localhost',
        'port' => '', // 3306
        'dbuser' => 'root',
        'dbpassword' => '',
        'username' => 'admin',
        'tbpref' => '0_',
        'admin' => 'admin',
        'inst_lang' => 'C',
        'collation' => 'xx',
    );
}

if (!@$_POST['Tests']) {
    $_POST['Page'] = 1;
}
// set to start page

if (isset($_POST['back']) && (@$_POST['Page'] > 1)) {
    if ($_POST['Page'] == 5) {
        $_POST['Page'] = 2;
    } else {
        $_POST['Page']--;
    }

} elseif (isset($_POST['continue'])) {
    $_POST['Page'] = 2;
} elseif (isset($_POST['db_test'])) {
    if (get_post('host') == '') {
        display_error(_('Host name cannot be empty.'));
        set_focus('host');
    } elseif ($_POST['port'] != '' && !is_numeric($_POST['port'])) {
        display_error(_('数据库端口不能为空.'));
        set_focus('port');
    } elseif ($_POST['dbuser'] == '') {
        display_error(_('数据库用户不能为空'));
        set_focus('dbuser');
    } elseif ($_POST['dbname'] == '') {
        display_error(_('数据库名字不能为空'));
        set_focus('dbname');
    } else {
        $_SESSION['inst_set'] = array_merge($_SESSION['inst_set'], array(
            'host' => $_POST['host'],
            'port' => $_POST['port'],
            'dbuser' => $_POST['dbuser'],
            'dbpassword' => @html_entity_decode($_POST['dbpassword'], ENT_QUOTES, $_SESSION['language']->encoding == 'iso-8859-2' ? 'ISO-8859-1' : $_SESSION['language']->encoding),
            'dbname' => $_POST['dbname'],
            'tbpref' => $_POST['tbpref'] ? '0_' : '',
            'sel_langs' => check_value('sel_langs'),
            'sel_coas' => check_value('sel_coas'),
            'collation' => $_POST['collation'],
        ));

        if (install_connect_db()) {
            $_POST['Page'] = check_value('sel_langs') ? 3 : (check_value('sel_coas') ? 4 : 5);
        }

    }
    if (!file_exists($path_to_root . '/lang/installed_languages.inc')) {
        $installed_languages = array(
            0 => array('code' => 'C', 'name' => 'English', 'encoding' => 'utf-8'));
        $dflt_lang = 'C';
        write_lang();
    }
} elseif (get_post('install_langs')) {
    $ret = true;
    if (isset($_POST['langs'])) {
        foreach ($_POST['langs'] as $package => $ok) {
            $ret &= install_language($package);
        }
    }

    if ($ret) {
        $_POST['Page'] = $_SESSION['inst_set']['sel_coas'] ? 4 : 5;
    }

} elseif (get_post('install_coas')) {
    $ret = true;
    $next_extension_id = 0;

    if (isset($_POST['coas'])) {
        foreach ($_POST['coas'] as $package => $ok) {
            $ret &= install_extension($package);
        }
    }

    if ($ret) {
        if (file_exists($path_to_root . '/installed_extensions.php')) {
            include $path_to_root . '/installed_extensions.php';
        }

        $_POST['Page'] = 5;
    }
} elseif (isset($_POST['set_admin'])) {
    // check company settings
    if (get_post('name') == '') {
        display_error(_('公司名称不能为空.'));
        set_focus('name');
    } elseif (get_post('admin') == '') {
        display_error(_('管理员名称不能为空.'));
        set_focus('admin');
    } elseif (get_post('pass') == '') {
        display_error(_('密码不能为空.'));
        set_focus('pass');
    } elseif (get_post('pass') != get_post('repass')) {
        display_error(_('两次密码输入不一致.'));
        unset($_POST['pass'], $_POST['repass']);
        set_focus('pass');
    } else {

        $_SESSION['inst_set'] = array_merge($_SESSION['inst_set'], array(
            'coa' => $_POST['coa'],
            'pass' => $_POST['pass'],
            'name' => $_POST['name'],
            'admin' => $_POST['admin'],
            'lang' => $_POST['lang'],
        ));
        if (do_install()) {
            $_POST['Page'] = 6;
        }

    }
}

if (list_updated('inst_lang')) {
    $_SESSION['inst_set']['inst_lang'] = get_post('inst_lang');
    $Ajax->setEncoding($inst_langs[get_post('inst_lang')]['encoding']);
    $Ajax->activate('welcome');
}

start_form();
switch (@$_POST['Page']) {
    default:
    case '1':
        div_start('welcome');
        subpage_title(_('系统诊断'));
        start_table();
        instlang_list_row(_('选择安装向导语言:'), 'inst_lang', $_SESSION['inst_set']['inst_lang']);
        end_table(1);
        $_POST['Tests'] = display_system_tests(true);
        br();
        if (@$_POST['Tests']) {
            display_notification(_('所有应用程序的初步要求似乎都是正确的。请按下面的继续按钮'));
            submit_center('continue', _('继续 >>'));
        } else {
            display_error(_('所有应用程序的初步要求似乎都是正确的。请按下面的继续按钮。'));
            submit_center('refresh', _('刷新'));
        }
        div_end();
        break;
    case '2':
        if (!isset($_POST['host'])) {
            foreach ($_SESSION['inst_set'] as $name => $val) {
                $_POST[$name] = $val;
            }

        }
        subpage_title(_('数据库设置'));
        start_table(TABLESTYLE);
        text_row_ex(_('服务器地址:'), 'host', 30, 60);
        text_row_ex(_('服务器端口:'), 'port', 30, 60);
        text_row_ex(_('数据库名称:'), 'dbname', 30);
        text_row_ex(_('用户名:'), 'dbuser', 30);
        password_row(_('密码:'), 'dbpassword', '');
        collations_list_row(_('排序规则:'), 'collation');
        yesno_list_row(_("表名前缀'0_':"), 'tbpref', 1, _('Yes'), _('No'), false);
        check_row(_('安装语言包:'), 'sel_langs');
        check_row(_('安装总账数据:'), 'sel_coas');
        end_table(1);
        display_note(_('使用具有权限的数据库用户/密码创建新数据库，或对以前创建的空数据库使用适当的凭据'));
        display_note(_('选择要使用的排序规则。如果您不确定或将使用多种语言，请选择unicode排序规则'));
        display_note(_('如果使用同一排序规则为多个公司共享选定的数据库，请使用表前缀。'));
        display_note(_('如果您现在没有工作的互联网连接，请不要选择其他LANG或COA。您可以稍后安装。'));
        display_note(_('如果无法使用默认端口3306，则仅设置端口值。'));
        submit_center_first('back', _('<< 返回'));
        submit_center_last('db_test', _('继续 >>'));
        break;
    case '3': // select langauges
        subpage_title(_('用户界面语言选择'));
        display_langs();
        submit_center_first('back', _('<< 返回'));
        submit_center_last('install_langs', _('继续 >>'));
        break;
    case '4': // select COA
        subpage_title(_('科目表选择'));
        display_coas();
        submit_center_first('back', _('<< 返回'));
        submit_center_last('install_coas', _('继续 >>'));
        break;
    case '5':
        if (!isset($_POST['name'])) {
            foreach ($_SESSION['inst_set'] as $name => $val) {
                $_POST[$name] = $val;
            }

            set_focus('name');
        }
        if (!isset($installed_extensions)) {
            $installed_extensions = array();
            update_extensions($installed_extensions);
        }
        subpage_title(_('公司设置'));
        start_table(TABLESTYLE);
        text_row_ex(_('名称:'), 'name', 30);
        text_row_ex(_('管理员名称:'), 'admin', 30);
        password_row(_('管理员密码:'), 'pass', @$_POST['pass']);
        password_row(_('重输密码:'), 'repass', @$_POST['repass']);
        coa_list_row(_('选择科目表:'), 'coa');
        languages_list_row(_('选择语言:'), 'lang');
        end_table(1);
        submit_center_first('back', _('<< 返回'));
        submit_center_last('set_admin', _('安装'), _('开始安装数据'), 'default nonajax');
        break;
    case '6': // final screen
        subpage_title(_('CbSMS 系统安装完成.'));
        display_note(_('请删除按安装目录.'));
        session_unset();
        session_destroy();
        hyperlink_no_params($path_to_root . '/index.php', _('开始使用.'));
        break;
}

hidden('Tests');
hidden('Page');
end_form(1);

end_page(false, false, true);
