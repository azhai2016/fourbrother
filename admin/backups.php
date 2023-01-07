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
$page_security = 'SA_BACKUP';

$path_to_root = '..';
include_once $path_to_root . '/includes/session.php';
include_once $path_to_root . '/includes/ui.inc';
include_once $path_to_root . '/admin/db/maintenance_db.inc';

if (get_post('view')) {
    if (!get_post('backups')) {
        display_error(_('选择备份文件'));
    } else {
        $filename = $SysPrefs->backup_dir() . clean_file_name(get_post('backups'));
        if (in_ajax()) {
            $Ajax->popup($filename);
        } else {
            header('Content-type: text/plain');
            header('Content-Length: ' . filesize($filename));
            header('Content-Disposition: inline; filename=' . basename($filename));

            if (substr($filename, -3, 3) == '.gz') {
                header('Content-Encoding: gzip');
            }

            if (substr($filename, -4, 4) == '.zip') {
                echo db_unzip('', $filename);
            } else {
                readfile($filename);
            }

            exit();
        }
    }
}

if (get_post('download')) {
    if (get_post('backups')) {
        download_file($SysPrefs->backup_dir() . clean_file_name(get_post('backups')));
        exit;
    } else {
        display_error(_('选择备份文件'));
    }

}

page(_($help_context = '备份和还原'));

check_paths();

function check_paths()
{
    global $SysPrefs;

    if (!file_exists($SysPrefs->backup_dir())) {
        display_error(_('备份路径设置不正确.') . _('请联系系统管理员.') . '<br>' . _('没有发现备份路径') . ' - ' . $SysPrefs->backup_dir() . '<br>');
        end_page();
        exit;
    }
}

function generate_backup($conn, $ext = 'no', $comm = '')
{
    global $SysPrefs;

    $filename = db_backup($conn, $ext, $comm, $SysPrefs->backup_dir());
    if ($filename) {
        display_notification(_('成功备份.') . ' ' . _('文件名') . ': ' . $filename);
    } else {
        display_error(_('备份失败.'));
    }

    return $filename;
}

function get_backup_file_combo()
{
    global $path_to_root, $Ajax, $SysPrefs;

    $ar_files = array();
    default_focus('backups');
    $dh = opendir($SysPrefs->backup_dir());
    while (($file = readdir($dh)) !== false) {
        $ar_files[] = $file;
    }

    closedir($dh);

    rsort($ar_files);
    $opt_files = '';
    foreach ($ar_files as $file) {
        if (preg_match("/.sql(.zip|.gz)?$/", $file)) {
            $opt_files .= "<option value='" . $file . "'>" . $file . "</option>";
        }
    }

    $selector = "<select class='nosearch' name='backups' size=2 style='height:160px;min-width:235px'>" . $opt_files . "</select>";

    $Ajax->addUpdate('backups', '_backups_sel', $selector);
    $selector = "<span id='_backups_sel'>" . $selector . "</span>\n";

    return $selector;
}

function compress_list_row($label, $name, $value = null)
{
    $ar_comps = array('no' => _('No'));

    if (function_exists('gzcompress')) {
        $ar_comps['zip'] = 'zip';
    }

    if (function_exists('gzopen')) {
        $ar_comps['gzip'] = 'gzip';
    }

    echo "<tr><td class='label'>" . $label . '</td><td>';
    echo array_selector('comp', $value, $ar_comps);
    echo '</td></tr>';
}

function download_file($filename)
{
    if (empty($filename) || !file_exists($filename)) {
        display_error(_('请选择备份文件.'));
        return false;
    }
    $saveasname = basename($filename);
    header('Content-type: application/octet-stream');
    header('Content-Length: ' . filesize($filename));
    header('Content-Disposition: attachment; filename="' . $saveasname . '"');
    readfile($filename);

    return true;
}

$conn = $db_connections[user_company()];
$backup_name = clean_file_name(get_post('backups'));
$backup_path = $SysPrefs->backup_dir() . $backup_name;

if (get_post('creat')) {
    generate_backup($conn, get_post('comp'), get_post('comments'));
    $Ajax->activate('backups');
    $SysPrefs->refresh(); // re-read system setup
}
;

if (get_post('restore')) {
    if ($backup_name) {
        if (db_import($backup_path, $conn, true, false, check_value('protect'))) {
            display_notification(_('还原数据库成功'));
        }

        $SysPrefs->refresh(); // re-read system setup
    } else {
        display_error(_('选择备份文件.'));
    }

}

if (get_post('deldump')) {
    if ($backup_name) {
        if (unlink($backup_path)) {
            display_notification(_('文件成功删除.') . ' ' . _('文件名') . ': ' . $backup_name);
            $Ajax->activate('backups');
        } else {
            display_error(_("不能删除备份文件"));
        }

    } else {
        display_error(_('选择备份文件'));
    }

}

if (get_post('upload')) {
    $tmpname = $_FILES['uploadfile']['tmp_name'];
    $fname = trim(basename($_FILES['uploadfile']['name']));

    if ($fname) {
        if (!preg_match("/\.sql(\.zip|\.gz)?$/", $fname)) {
            display_error(_('只能上传 sql 格式文件'));
        } elseif ($fname != clean_file_name($fname)) {
            display_error(_('文件名包含禁止的字符。请重命名文件并重试。'));
        } elseif (is_uploaded_file($tmpname)) {
            rename($tmpname, $SysPrefs->backup_dir() . $fname);
            display_notification(_('文件上传到备份路径'));
            $Ajax->activate('backups');
        } else {
            display_error(_('文件不能上传'));
        }

    } else {
        display_error(_('选择备份文件.'));
    }

}

//-------------------------------------------------------------------------------

start_form(true);
start_outer_table(TABLESTYLE2);
table_section(1);
table_section_title(_('备份'));
textarea_row(_('说明:'), 'comments', null, 30, 9);
compress_list_row(_('是否压缩:'), 'comp');
vertical_space("height='30px'");
submit_row('creat', _('生成备份'), false, "colspan=2 align='center'", '', 'process');
table_section(2);
table_section_title(_('备份脚本维护'));

start_row();
echo "<td style='padding-left:20px' align='left'>" . get_backup_file_combo() . "</td>";
echo "<td style='padding-left:20px' valign='top'>";
start_table();
submit_row('view', _('查看备份文件'), false, '', '', false);
submit_row('download', _('下载备份文件'), false, '', '', 'download');
submit_row('restore', _('还原备份'), false, '', '', 'process');
submit_js_confirm('restore', _("是否还原数据库文件。\n是否继续?"));

submit_row('deldump', _('删除备份'), false, '', '', true);
// don't use 'delete' name or IE js errors appear
submit_js_confirm('deldump', sprintf(_("是否删除备份文件.\n 是否继续 ?")));
end_table();
echo '</td>';
end_row();
start_row();
echo "<td style='padding-left:20px' colspan=2>" . radio(_('上传安全设置'), 'protect', 0) . '<br>' . radio(_('保护安全设置'), 'protect', 1, true) . '</td>';
end_row();
start_row();
echo "<td style='padding-left:20px' align='left'><input name='uploadfile' type='file'></td>";
submit_cells('upload', _('上传文件'), "style='padding-left:20px'", '', true);
end_row();
end_outer_table();

end_form();

end_page();
