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
include_once($path_to_root . '/includes/ui/simple_crud_class.inc');


/*
	View/Edit class for contacts
*/

class PrintManager extends simple_crud
{
    private $id;
    private $entity;
    private $sub_class;
    private $class;
    private $search_types;
    private $formula_id;
    private $values = array();
    private $assembly = array();
    private $agrCreate;
    private $dataloaded = false;


    function __construct($name, $id, $class, $search_types = null, $formula_id = null)
    {
        global $Ajax;

        $fields = array(
            'bind_id', 'name', 'description', 'sql_txt', 'sum_field', 'mod_id', 'jasper_file', 'inactive'
        );
        parent::__construct($name, $fields);
        $this->class = $class;
        $this->entity = $id;
        $this->search_types = $search_types;
    }


    function list_view()
    {

        $result = get_print_template();

        start_form(true);
        start_table(TABLESTYLE, "width='30%'");
        $th = array(_('编号'), _('报表名称'), _('说明'), '', '', '');

        inactive_control_column($th);

        table_header($th);
        $k = 0; //row colour counter


        while ($myrow = db_fetch($result)) {

            alt_table_row_color($k);
            label_cell($myrow['bind_id']);
            label_cell($myrow['name']);
            label_cell($myrow['description']);
            inactive_control_cell($myrow['id'], $myrow['inactive'], 'name', 'id');
            //print_button_cell2('Printer'.$myrow['id'], _('设置'));
            edit_button_cell("{$this->name}Edit[{$myrow['id']}]", _('编辑'));
            delete_button_cell2("{$this->name}Delete[{$myrow['id']}]", _('删除'));
            end_row();
        }

        inactive_control_row($th);

        end_table();
        br(2);
    }

    function editor_view()
    {
        global $path_to_root;

        start_table(TABLESTYLE2);

        text_row(_('编号:'), 'bind_id', null, 30, 30);
        print_mod_list('报表ID', 'mod_id');
        text_row(_('报表别名:'), 'name', null, 30, 30);
        text_row(_('说明:'), 'description', null, 30, 30);
        textarea_row(_('SQL语句:'), 'sql_txt', null, 30, 9);
        if ($this->selected_id)
            label_row(_('文件:'), get_post('jasper_file'));
        file_row(_('jasper文件'), 'jasper_file', 'jasper_file');
        text_row(_('汇总字段:'), 'sum_field', null, 30, 30);
        check_row(_('激活:'), 'inactive');

        end_table();
        br(2);

        submit_js_confirm("{$this->entity}ADD", "是否提交数据？");
    }

    function db_insert()
    {
        if (isset($_FILES[$this->entity])) {
            $jasper_file = get_jasper_upload_file($this->entity);
            $_POST['jasper_file'] = $jasper_file;

            $data = array(
                $_POST['bind_id'],
                $_POST['name'],
                $_POST['description'],
                $_POST['sql_txt'],
                $_POST['sum_field'],
                $_POST['mod_id'],
                $_POST['jasper_file'],
                check_value('inactive')
            );

            add_print_content($data);
            display_notification(_('新内容已添加'));
        }

        return true;
    }

    function insert_check()
    {
        if (isset($_FILES[$this->entity]) && $_FILES[$this->entity]['error'] > 0) {
            display_warning(_('文件上传错误？请检查文件'));
            return false;
        }
        return true;
    }

    function db_read()
    {
        return get_print_template_id($this->selected_id);
    }

    function delete_check()
    {
        return true;
    }
    //
    //	Delete all contacts for person in current class/entity
    //
    function db_delete()
    {
        $rs = delete_print_data($this->selected_id);
        return $rs > 0; //delete_agr_items_data($this->selected_id)
    }



    function import_check()
    {
        return true;
    }

    function db_import()
    {
        $rs = add_mssql_ddi_data('stock');

        if ($rs > 0) {
            $this->db_delete();
        }
        return $rs;
    }
}
