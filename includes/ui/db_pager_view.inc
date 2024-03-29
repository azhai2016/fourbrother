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
//--------------------------------------------------------------------------------------------------
function pager_link($link_text, $url, $icon = false)
{
    global $path_to_root;

    if (user_graphic_links() && $icon) {
        $link_text = set_icon($icon, $link_text);
    }

    $href = $path_to_root . $url;
    return "<a href='" . $href . "'>" . $link_text . "</a>";
}

function navi_button($name, $value, $enabled = true, $icon = false)
{
    global $path_to_root;

    return "<button " . ($enabled ? '' : 'disabled') . " class='navibutton' type='submit' name='" . $name . "' id='" . $name . "' value='" . $value . "'>" . ($icon ? "<img src='" . $path_to_root . '/themes/' . user_theme() . '/images/' . $icon . "'>" : '') . "<span>" . $value . "</span></button>\n";
}

function navi_button_cell($name, $value, $enabled = true, $align = 'left')
{
    label_cell(navi_button($name, $value, $enabled), "align='" . $align . "'");
}

//-----------------------------------------------------------------------------
//
//    Sql paged table view. Call this function inside form.
//
function display_db_pager(&$pager)
{
    global $path_to_root;

    $pager->select_records();

    div_start("_{$pager->name}_span");
    $headers = array();
    foreach ($pager->columns as $num_col => $col) {
        // record status control column is displayed only when control checkbox is on

        if (isset($col['head']) && ($col['type'] != 'inactive' || get_post('show_inactive'))) {
            if (!isset($col['ord'])) {
                $headers[] = $col['head'];
            } else {
                $icon = (($col['ord'] == 'desc') ? 'sort_desc.gif' : ($col['ord'] == 'asc' ? 'sort_asc.gif' : 'sort_none.gif'));
                $headers[] = navi_button($pager->name . '_sort_' . $num_col, $col['head'], true, $icon);
            }
        }
        else
        if ($col['insert']==1) {
            $headers[]= '';  
        }

    }
    // show a table of records returned by the sql
    start_table(TABLESTYLE, "width='$pager->width'");
    table_header($headers);

    if ($pager->header_fun) { // if set header handler
        start_row("class='{$pager->header_class}'");
        $fun = $pager->header_fun;
        if (method_exists($pager, $fun)) {
            $h = $pager->$fun($pager);
        } elseif (function_exists($fun)) {
            $h = $fun($pager);
        }

        foreach ($h as $c) { // draw header columns
            $pars = isset($c[1]) ? $c[1] : '';
            label_cell($c[0], $pars);
        }
        end_row();
    }

    $c = 0; //row colour counter
    foreach ($pager->data as $line_no => $row) {

        if ($pager->row_fun) {
            if (function_exists($pager->row_fun)) {
                $fn = $pager->row_fun;
                $row = $fn($row);
            }
        }

        $row_class = false;

        foreach ($pager->marker as $marker) {
            if ($marker['func'] && $marker['func']($row)) {
                $row_class = $marker['class'];
            }

        }
        if ($row_class) {
            start_row("class='" . $row_class . "'");
        } else {
            alt_table_row_color($c);
        }

        foreach ($pager->columns as $k => $col) {
            $coltype = $col['type'];
            $cell = isset($col['name']) ? $row[$col['name']] : '';
 

            if (isset($col['fun'])) { // use data input function if defined
                $fun = $col['fun'];
                if (method_exists($pager, $fun)) {
                    $cell = $pager->$fun($row, $cell);
                } elseif (function_exists($fun)) {
                    $cell = $fun($row, $cell);
                } else {
                    $cell = '';
                }
                
            }

            switch ($coltype) { // format column
                case 'tiny_width':
                    label_cell($cell, "width='30px'");
                    break;
                case 'small_width':
                    label_cell($cell, "width='50px'");
                    break;
                case 'middle_width':
                        label_cell($cell, "width='100px'");
                        break;    
                case 'small_center_width':
                        label_cell($cell, "align='center' nowrap width='50px'");
                        break;    
                case 'large_width':
                     label_cell($cell, "width='200px'");
                        break;
                case 'time':
                    label_cell($cell, "width='80px'");
                    break;
                case 'date':
                    label_cell(substr(sql2date($cell),0,10), "align='center' nowrap");
                    break;
                case 'dstamp': // time stamp displayed as date
                    label_cell(sql2date(substr($cell, 0, 10)), "align='center' style='width:120px;' nowrap");
                    break;
                case 'tstamp': // time stamp - FIX user format
                    label_cell(sql2date(substr($cell, 0, 10)) . ' ' . substr($cell, 10), "align='center'");
                    break;
                case 'percent':
                    percent_cell($cell);
                    break;
                case 'amount':
                    ($cell == '') ? label_cell('') : amount_cell($cell, false);
                    break;
                case 'qty':
                    ($cell == '') ? label_cell('') : qty_cell($cell, false, isset($col['dec']) ? $col['dec'] : null);
                    break;
                case 'email':
                    email_cell($cell, isset($col['align']) ? "align='" . $col['align'] . "'" : null);
                    break;
                case 'rate':
                    label_cell(number_format2($cell, user_exrate_dec()), "align=center");
                    break;
                case 'active':
                    if (get_post('show_inactive')) {
                        $pager->inactive_control_cell($row);
                    }
                    break;
        
                case 'select': 
                     $col_value = isset($col['col_value']) ? $col['col_value'] : '-1';
                     $client_id = isset($_GET['client_id'])?$_GET['client_id']:get_post('client_id');
                     ahref_cell(_('选择'), 'javascript:void(0)', '', 'selectTableItem(this,window.opener.document, "'.$client_id.'",'.$col_value.')');
                    break;
                default:
                    (isset($col['align'])) ? label_cell($cell, "align='" . $col['align'] . "'") : label_cell($cell);
                case 'skip': // column not displayed
            }
        }
        end_row();
    }

    if ($pager->footer_fun) { // if set footer handler
        start_row("class='{$pager->footer_class}'");
        $fun = $pager->footer_fun;
        if (method_exists($pager, $fun)) {
            $h = $pager->$fun($pager);
        } elseif (function_exists($fun)) {
            $h = $fun($pager);
        }

        foreach ($h as $c) { // draw footer columns
            $pars = isset($c[1]) ? $c[1] : '';
            label_cell($c[0], $pars);
        }
        end_row();
    }

    start_row("class='navibar'");
    $colspan = count($pager->columns);
    $inact = @$pager->inactive_ctrl == true ? ' ' . checkbox(null, 'show_inactive', null, true) . _('Show also Inactive') : '';
    if ($pager->rec_count) {
        echo "<td colspan=$colspan class='navibar' style='border:none;padding:3px;'>";
        echo "<div style='float:right;'>";
        $but_pref = $pager->name . '_page_';
        start_table();
        start_row();
        if (@$pager->inactive_ctrl) {
            submit('Update', _('Update'), true, '', null);
        }
        // inactive update
        echo navi_button_cell($but_pref . 'first', _('<<'), $pager->first_page, 'right');
        echo navi_button_cell($but_pref . 'prev', _('<'), $pager->prev_page, 'right');
        echo number_list_cells(null, 'pager_list', null, 1, $pager->max_page, false, true, 'nosearch');
        echo navi_button_cell($but_pref . 'next', _('>'), $pager->next_page, 'right');
        echo navi_button_cell($but_pref . 'last', _('>>'), $pager->last_page, 'right');
        end_row();
        end_table();
        echo "</div>";
        $from = ($pager->curr_page - 1) * $pager->page_len + 1;
        $to = $from + $pager->page_len - 1;
        if ($to > $pager->rec_count) {
            $to = $pager->rec_count;
        }

        $all = $pager->rec_count;
        echo sprintf(_('本页 %d-%d 记录，总数：%d 条'), $from, $to, $all);
        echo $inact;
        echo "</td>";
    } else {
        label_cell(_('没有数据') . $inact, "colspan=$colspan class='navibar'");
    }

    end_row();
    end_table();

    foreach ($pager->marker as $marker) {
        if (isset($marker['txt'])) {
            display_note($marker['txt'], 0, 0, "class='" . $marker['txt_class'] . "'");
        }

    }
    br();
    div_end();

    return true;
}
