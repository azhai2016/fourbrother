<?php
/**********************************************************************
Copyright (C) CbSMS.
Released under the terms of the GNU General Public License, GPL,
as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
 ***********************************************************************/
//
//    Controler part of database table pager with column sort.
//    To display actual html object call display_db_pager($name) inside
//  any form.
//
//    View definition you will find in the following file:
include_once $path_to_root . '/includes/ui/db_pager_view.inc';

class db_ms_pager
{
    public $sql;
    public $name;
    public $columns; // column definitions (head, type, order)
    public $tbname;
    public $marker = array(); // marker check function
    public $width; // table width (default '95%')
    public $header_fun; // additional row between title and body
    public $header_class;
    public $row_fun; // Function for row preprocessing
    public $footer_fun;
    public $footer_class;
    public $data = array();

    public $curr_page,
    $max_page,
    $last_page,
    $prev_page,
    $next_page,
        $first_page;

    public $page_len,
        $rec_count;

    public $select,
    $where,
    $from,
    $group,
        $order;
    public $extra_where = array();

    public $ready = false; // this var is false after change in sql before first
    // and before first query.
    public $inactive_ctrl = false;
    public $main_tbl; // table and key field name for inactive ctrl and edit/delete links
    public $key; // key field name

    public $show_columns="*";
    public $sql_where = "";
    //  db_pager constructor
    //  accepts $sql query either as:
    //  a. string in form 'SELECT field_list FROM table_joins [WHERE conditions [GROUP group_list [ORDER order_list]]]'
    //        - if WHERE keyword is used in table_joins, WHERE conditions is obligatory
    //  b. associative array using select, where, group and order keys ex :
    //      array('select' => 'SUM(quantity)', 'from' => TB_PREF."stock_moves", 'group' => 'location')
    //
    //    $name is base name for pager controls
    public function __construct($sql, $name, $table = null, $page_len = 0)
    {
        $this->width = '95%';
        if ($page_len == 0) {
            $page_len = user_query_size();
        }

        $this->name = $name;
        $this->page_len = $page_len;
        $this->set_sql($sql);
    }
    //
    //    Parse base sql select query    or use an associative array.
    //
    public function set_sql($sql)
    {
        global $SysPrefs;

        if ($sql != $this->sql) {
            $this->sql = $sql;
            $this->ready = false;
        } 
    }
    //
    //    Set additional constraint on record set
    //
    public function set_where($where = null)
    {
        if ($where) {
            if (!is_array($where)) {
                $where = array($where);
            }

            if (count($where) == count($this->extra_where) &&
                !count(array_diff($this->extra_where, $where))) {
                return;
            }

        }
        $this->extra_where = $where;
        $this->ready = false;
    }
    //
    //    Set query result page
    //
    public function change_page($page = null)
    {
        $this->set_page($page);
        $this->query();
        return true;
    }
    //
    //    Change sort column direction
    //    in order asc->desc->none->asc
    //
    public function sort_table($col)
    {
        $max_priority = 0;
        foreach ($this->columns as $id => $_col) {
            if (!isset($_col['ord_priority'])) {
                continue;
            }

            $max_priority = max($max_priority, $_col['ord_priority']);
        };

        $ord = $this->columns[$col]['ord'];
        $this->columns[$col]['ord_priority'] = $max_priority + 1; // set priority , higher than anything else
        $ord = ($ord == '') ? 'asc' : (($ord == 'asc') ? 'desc' : '');
        $this->columns[$col]['ord'] = $ord;
        $this->set_page(1);
        $this->query();
        return true;
    }
    //
    // Query database
    //
    public function query()
    {
        global $Ajax;

        $Ajax->activate("_{$this->name}_span");
        $this->data = array();

        if (!$this->_init()) {
            return false;
        }

        if ($this->rec_count == 0) {
            return true;
        }

        $sql = $this->_sql_gen(false);

        $result = mssql_db_query($sql, 'Error browsing database: ' . $sql);

        if ($result) {
            $c = 0;
            while ($row = mssqldb_fetch_assoc($result)) {
                $this->data[] = $row;
            }
        } else {
            return false;
        }

        return true;
    }
    //
    //    Calculates page numbers for html controls.
    //
    public function set_page($to)
    {
    
        switch ($to) {
            case 'next':
                $page = $this->curr_page + 1;
                break;
            case 'prev':
                $page = $this->curr_page - 1;
                break;
            case 'last':
                $page = $this->last_page;
                break;
            default:
                if (is_numeric($to)) {
                    $page = $to;
                    break;
                }
            case 'first':
                $page = 1;
                break;
        }
        if ($page < 1) {
            $page = 1;
        }

        $max = $this->max_page;
 

        if ($page > $max) {
            $page = $max;
        }

        $_POST['pager_list'] = $page;
        $this->curr_page = $page;
        $this->next_page = ($page < $max) ? $page + 1 : null;
        $this->prev_page = ($page > 1) ? ($page - 1) : null;
        $this->last_page = ($page < $max) ? $max : null;
        $this->first_page = ($page != 1) ? 1 : null;
    }
    //
    //    Set column definitions
    //  $flds: array( fldname1, fldname2=>type,...)
    public function set_columns($flds)
    {
        $this->columns = array();
        if (!is_array($flds)) {
            $flds = array($flds);
        }

        foreach ($flds as $colnum => $coldef) {
            if (is_string($colnum)) { // 'colname'=>params
                $h = $colnum;
                $c = $coldef;
            } else { //  n=>params
                if (is_array($coldef)) {
                    $h = '';
                    $c = $coldef;
                } else {
                    $h = $coldef;
                    $c = 'text';
                }
            }
            if (is_string($c)) // params is simple column type
            {
                $c = array('type' => $c);
            }

            if (!isset($c['type'])) {
                $c['type'] = 'text';
            }

            switch ($c['type']) {
                case 'active':
                    $this->inactive_ctrl = true;
                case 'insert':
                default:
                    $c['head'] = $h;
                    break;
                case 'skip': // skip the column (no header)
                    unset($c['head']);
                    break;
            }
            $this->columns[] = $c;
        }
    }
    //
    // Generate db query from base sql
    // $count==false - for current page data retrieval
    // $count==true  - for total records count
    //
    public function _sql_gen($count = false)
    {
        $ord = array();
        $columns = array();
        foreach ($this->columns as $col) {
            if (isset($col['ord_priority'])) {
                $columns[$col['ord_priority']] = $col;
            }

        }
        krsort($columns);

        foreach ($columns as $col) {
           
            if (isset($col['ord'])) {
                if ($col['ord'] != '' && isset($col['name'])) {
                    $ord[] = $col['name'] . ' ' . $col['ord'];
                }

            }
        }


        if (count($ord)) {
             $orderBy = implode(',', $ord);
        } else {
            if ($this->orderBy) {
              $orderBy = $this->orderBy;
            }
        }

        $page_len = $this->page_len;
        $offset =  empty($this->curr_page)?1:$this->curr_page;
    
        $sql = $this->sql.sprintf(",@OrderBy='%s',@pagenumber=%d,@pagesize=%d",$orderBy,$offset,$page_len);

        return $sql;
    }
    //
    //    Initialization after changing record set
    //
    public function _init()
    {
        global $SysPrefs;
        if ($this->ready == false) {

            $sql = $this->_sql_gen(true);
            $result = mssql_db_query($sql,0,'sqlserver 查询错误');
  
            if ($result == false) {
                return false;
            }

            $row = mssqldb_fetch_assoc($result);
         
            if (!$row) {
                return false;
            }
            $this->rec_count = $row['TotalRows'];
          
            $this->max_page = $row['TotalPages'];
            $this->set_page(1);
            $this->ready = true;
        }
        return true;
    }
    //
    //    Set current page in response to user control.
    //
    public function select_records()
    {
        global $Ajax;

        $page = find_submit($this->name . '_page_', false);
        $sort = find_submit($this->name . '_sort_', true);
        if (list_updated('pager_list')) {
            $page = $_POST['pager_list'];
        }

        if ($page) {
            $this->change_page($page);
            if (!$this->next_page || !$this->last_page) {
                set_focus($this->name . '_page_prev');
            }

            if (!$this->prev_page || !$this->first_page) {
                set_focus($this->name . '_page_next');
            }

        } elseif ($sort != -1) {
            $this->sort_table($sort);
        } else {
            $this->query();
        }

    }
    //
    //    Set check function to mark some rows.
    //
    public function set_marker($func, $notice = '', $markercl = 'overduebg', $msgclass = 'overduefg')
    {
      
        // add a marker only when it is not exist
        if (array_search($func, array_column($this->marker, 'func')) === false) {
            $this->marker[] = array('func' => $func, 'txt' => $notice, 'class' => $markercl, 'txt_class' => $msgclass);
        }

    }
    //
    //    Set handler to display additional row between titles and pager body.
    //    Return array of column contents.
    //
    public function set_header($func, $headercl = 'inquirybg')
    {
        $this->header_fun = $func;
        $this->header_class = $headercl;
    }
    //
    //    Set handler to display additional row between pager body and navibar.
    //    Return array of column contents.
    //
    public function set_footer($func, $footercl = 'inquirybg')
    {
        $this->footer_fun = $func;
        $this->footer_class = $footercl;
    }
    //
    //    Setter for table editors with inactive cell control.
    //
    public function set_inactive_ctrl($table, $key)
    {
        $this->inactive_ctrl = array('table' => $table, 'key' => $key);
    }
    //
    //    Helper for display inactive control cells
    //
    public function inactive_control_cell(&$row)
    {
        if ($this->inactive_ctrl) {

            global $Ajax;

            $key = $this->key ? $this->key : $this->columns[0]['name']; // TODO - support for complex keys
            $id = $row[$key];
            $table = $this->main_tbl;
            $name = "Inactive" . $id;
            $value = $row['inactive'] ? 1 : 0;

            if (check_value('show_inactive')) {
                if (isset($_POST['LInact'][$id]) && (get_post('_Inactive' . $id . '_update') || get_post('Update')) && (check_value('Inactive' . $id) != $value)) {
                    update_record_status($id, !$value, $table, $key);
                    $value = !$value;
                }
                echo '<td align="center">' . checkbox(null, $name, $value, true, '') . hidden("LInact[$id]", $value, false) . '</td>';
            }
        } else {
            return '';
        }

    }
};
//-----------------------------------------------------------------------------
//    Creates new db_pager $_SESSION object on first page call.
//  Retrieves from $_SESSION var on subsequent $_POST calls
//
//  $name - base name for pager controls and $_SESSION object name
//  $sql  - base sql for data inquiry. Order of fields implies
//        pager columns order.
//    $coldef - array of column definitions. Example definitions
//        Column with title 'User name' and default text format:
//                'User name'
//        Skipped field from sql query. Data for the field is not displayed:
//                'dummy' => 'skip'rec_count
//        Column without title, data retrieved form row data with function func():
//                 array('fun'=>'func')
//         Inserted column with title 'Some', formated with function rowfun().
//      formated as date:
//                'Some' => array('type'=>'date, 'insert'=>true, 'fun'=>'rowfun')
//         Column with name 'Another', formatted as date,
// sortable with ascending start order (available orders: asc,desc, '').
//                'Another' => array('type'=>'date', 'ord'=>'asc')
//
//    All available column format types you will find in db_pager_view.inc file.
//        If query result has more fields than count($coldef), rest of data is ignored
//  during display, but can be used in format handlers for 'spec' and 'insert'
//    type columns.

function &new_mssql_db_pager($name, $sql, $coldef, $table = null,$key = null, $page_len = 0)
{
   
    if (isset($_SESSION[$name]) && ($_SERVER['REQUEST_METHOD'] == 'GET' || $_SESSION[$name]->sql != $sql)) {
        unset($_SESSION[$name]);
    }

   
    if (!isset($_SESSION[$name])) {
        $_SESSION[$name] = new db_ms_pager($sql, $name, $table, $page_len);
        $_SESSION[$name]->main_tbl = $table;
        $_SESSION[$name]->orderBy = $key;
        $_SESSION[$name]->set_sql($sql);
        $_SESSION[$name]->set_columns($coldef);
    }

 
    return $_SESSION[$name];
}

//
//    Force pager initialization.
//
function refresh_ms_pager($name)
{
    if (isset($_SESSION[$name])) {
        $_SESSION[$name]->ready = false;
    }

}
