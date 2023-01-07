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
$path_to_root = '..';
$page_security = 'SA_ATTACHDOCUMENT';

include_once $path_to_root . '/includes/db_pager.inc';
include_once $path_to_root . '/includes/session.php';

include_once $path_to_root . '/includes/date_functions.inc';
include_once $path_to_root . '/includes/ui.inc';
include_once $path_to_root . '/includes/data_checks.inc';
include_once $path_to_root . '/admin/db/attachments_db.inc';
include_once $path_to_root . '/admin/db/transactions_db.inc';

if (isset($_GET['vw'])) {
    $view_id = $_GET['vw'];
} else {
    $view_id = find_submit('view');
}

if ($view_id != -1) {

    $row = get_attachment($view_id);

    if ($row['filename'] != '') {
        if (in_ajax()) {
            $Ajax->popup($_SERVER['PHP_SELF'] . '?vw=' . $view_id);
        } else {
            $type = ($row['filetype']) ? $row['filetype'] : 'application/octet-stream';
            header('Content-type: ' . $type);
            header('Content-Length: ' . $row['filesize']);
            header('Content-Disposition: inline');
            echo file_get_contents(company_path() . '/attachments/' . $row['unique_name']);
            exit();
        }
    }
}
if (isset($_GET['dl'])) {
    $download_id = $_GET['dl'];
} else {
    $download_id = find_submit('download');
}

if ($download_id != -1) {
    $row = get_attachment($download_id);
    if ($row['filename'] != '') {
        if (in_ajax()) {
            $Ajax->redirect($_SERVER['PHP_SELF'] . '?dl=' . $download_id);
        } else {
            $type = ($row['filetype']) ? $row['filetype'] : 'application/octet-stream';
            header('Content-type: ' . $type);
            header('Content-Length: ' . $row['filesize']);
            header('Content-Disposition: attachment; filename="' . $row['filename'] . '"');
            echo file_get_contents(company_path() . '/attachments/' . $row['unique_name']);
            exit();
        }
    }
}

$js = '';
if ($SysPrefs->use_popup_windows) {
    $js .= get_js_open_window(800, 500);
}

page(_($help_context = 'Attach Documents'), false, false, '', $js);

simple_page_mode(true);

//----------------------------------------------------------------------------------------

if (isset($_GET['filterType'])) // catch up external links
{
    $_POST['filterType'] = $_GET['filterType'];
}

if (isset($_GET['trans_no'])) {
    $_POST['trans_no'] = $_GET['trans_no'];
}

if ($Mode == 'ADD_ITEM' || $Mode == 'UPDATE_ITEM') {

    $filename = basename($_FILES['filename']['name']);
    if (!transaction_exists($_POST['filterType'], $_POST['trans_no'])) {
        display_error(_('选择文件已存在'));
    } elseif ($Mode == 'ADD_ITEM' && !in_array(strtoupper(substr($filename, strlen($filename) - 3)), array('JPG', 'PNG', 'GIF', 'PDF', 'DOC', 'ODT'))) {
        display_error(_('只能上传图片格式的文件'));
    } elseif ($Mode == 'ADD_ITEM' && !isset($_FILES['filename'])) {
        display_error(_('选择附件文件'));
    } elseif ($Mode == 'ADD_ITEM' && ($_FILES['filename']['error'] > 0)) {
        if ($_FILES['filename']['error'] == UPLOAD_ERR_INI_SIZE) {
            display_error(_('文件大小超出限制'));
        } else {
            display_error(_('选择附件文件.'));
        }

    } elseif (strlen($filename) > 60) {
        display_error(_('文件名长度大于60个字符'));
    } else {

        $tmpname = $_FILES['filename']['tmp_name'];
        $dir = company_path() . '/attachments';

        if (!file_exists($dir)) {
            mkdir($dir, 0777);
            $index_file = "<?php\nheader(\"Location: ../index.php\");\n";
            $fp = fopen($dir . '/index.php', 'w');
            fwrite($fp, $index_file);
            fclose($fp);
        }

        $filesize = $_FILES['filename']['size'];
        $filetype = $_FILES['filename']['type'];

        // file name compatible with POSIX
        // protect against directory traversal
        if ($Mode == 'UPDATE_ITEM') {
            $row = get_attachment($selected_id);
            if ($row['filename'] == '') {
                exit();
            }

            $unique_name = $row['unique_name'];
            if ($filename && file_exists($dir . '/' . $unique_name)) {
                unlink($dir . '/' . $unique_name);
            }

        } else {
            $unique_name = random_id();
        }

        //save the file
        move_uploaded_file($tmpname, $dir . '/' . $unique_name);

        if ($Mode == 'ADD_ITEM') {
            add_attachment($_POST['filterType'], $_POST['trans_no'], $_POST['description'], $filename, $unique_name, $filesize, $filetype);
            display_notification(_('文件上传成功'));
        } else {
            update_attachment($selected_id, $_POST['filterType'], $_POST['trans_no'], $_POST['description'], $filename, $unique_name, $filesize, $filetype);
            display_notification(_('附件文件已更新'));
        }
        reset_form();
    }
    refresh_pager('trans_tbl');
    $Ajax->activate('_page_body');
}

if ($Mode == 'Delete') {
    $row = get_attachment($selected_id);
    $dir = company_path() . '/attachments';
    if (file_exists($dir . '/' . $row['unique_name'])) {
        unlink($dir . '/' . $row['unique_name']);
    }

    delete_attachment($selected_id);
    display_notification(_('附件文件已删除'));
    reset_form();
}

if ($Mode == 'RESET') {
    reset_form();
}

function reset_form()
{
    global $selected_id;
    unset($_POST['trans_no']);
    unset($_POST['description']);
    $selected_id = -1;
}

function viewing_controls()
{
    global $selected_id;

    start_table(TABLESTYLE_NOBORDER);

    start_row();
    systypes_list_cells(_('类型:'), 'filterType', null, true);
    if (list_updated('filterType')) {
        reset_form();
    }

    end_row();
    end_table(1);
}

function trans_view($trans)
{
    return get_trans_view_str($trans['type_no'], $trans['trans_no']);
}

function edit_link($row)
{
    return button('Edit' . $row['id'], _('编辑'), _('编辑'), ICON_EDIT);
}

function view_link($row)
{
    return button('view' . $row['id'], _('查看'), _('查看'), ICON_VIEW);
}

function download_link($row)
{
    return button('download' . $row['id'], _('下载'), _('下载'), ICON_DOWN);
}

function delete_link($row)
{
    return button('Delete' . $row['id'], _('删除'), _('删除'), ICON_DELETE);
}

function display_rows($type, $trans_no)
{

    $sql = get_sql_for_attached_documents($type, $type == ST_SUPPLIER || $type == ST_CUSTOMER ? $trans_no : 0);

    $cols = array(
        _('#') => array('fun' => 'trans_view', 'ord' => ''),
        _('说明') => array('name' => 'description'),
        _('文件名') => array('name' => 'filename'),
        _('大小') => array('name' => 'filesize'),
        _('文件类型') => array('name' => 'filetype'),
        _('上传日期') => array('name' => 'tran_date', 'type' => 'date'),
        array('insert' => true, 'fun' => 'edit_link', 'align' => 'center'),
        array('insert' => true, 'fun' => 'view_link', 'align' => 'center'),
        array('insert' => true, 'fun' => 'download_link', 'align' => 'center'),
        array('insert' => true, 'fun' => 'delete_link', 'align' => 'center'),
    );

    if ($type == ST_SUPPLIER || $type == ST_CUSTOMER) {
        $cols[_('#')] = 'skip';
    }

    $table = &new_db_pager('trans_tbl', $sql, $cols);

    $table->width = '60%';

    display_db_pager($table);
}

//----------------------------------------------------------------------------------------

if (list_updated('filterType') || list_updated('trans_no')) {
    $Ajax->activate('_page_body');
}

start_form(true);

viewing_controls();

$type = get_post('filterType');

display_rows($type, get_post('trans_no'));

br(2);

start_table(TABLESTYLE2);

if ($selected_id != -1) {
    if ($Mode == 'Edit') {
        $row = get_attachment($selected_id);
        $_POST['trans_no'] = $row['trans_no'];
        $_POST['description'] = $row['description'];
        hidden('trans_no', $row['trans_no']);
        hidden('unique_name', $row['unique_name']);
        if ($type != ST_SUPPLIER && $type != ST_CUSTOMER) {
            label_row(_('Transaction #'), $row['trans_no']);
        }

    }
    hidden('selected_id', $selected_id);
}

text_row_ex(_('说明') . ':', 'description', 40);
file_row(_('附件文件') . ':', 'filename', 'filename');

end_table(1);

submit_add_or_update_center($selected_id == -1, '', 'process');

end_form();
end_page();
