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
$page_security = 'SA_SETUPDISPLAY';
$path_to_root = '..';
include $path_to_root . '/includes/session.php';

page(_($help_context = '基本设置'));

include_once $path_to_root . '/includes/date_functions.inc';
include_once $path_to_root . '/includes/ui.inc';

include_once $path_to_root . '/admin/db/company_db.inc';

//-------------------------------------------------------------------------------------------------

if (isset($_POST['setprefs'])) {
    if (!is_numeric($_POST['query_size']) || ($_POST['query_size'] < 1)) {
        display_error($_POST['query_size']);
        display_error(_('内容必须是数字或大于零.'));
        set_focus('query_size');
    } else {
        $_POST['theme'] = clean_file_name($_POST['theme']);
        $chg_theme = user_theme() != $_POST['theme'];
        $chg_lang = 'English';//$_SESSION['language']->code != $_POST['language'];
        $chg_date_format = user_date_format() != $_POST['date_format'];
        $chg_date_sep = user_date_sep() != $_POST['date_sep'];

        set_user_prefs(get_post(
            array('prices_dec', 'qty_dec', 'rates_dec', 'percent_dec', 'date_format', 'date_sep', 'tho_sep', 'dec_sep', 'print_profile', 'theme', 'page_size', 'language', 'startup_tab', 'query_size' => 10, 'transaction_days' => 30, 'save_report_selections' => 0, 'def_print_destination' => 0, 'def_print_orientation' => 0)));

        set_user_prefs(check_value(
            array('show_gl', 'show_codes', 'show_hints', 'rep_popup', 'graphic_links', 'sticky_doc_date', 'use_date_picker')));

        if ($chg_lang) {
            $_SESSION['language']->set_language('English');
        }

        // refresh main menu

        flush_dir(company_path() . '/js_cache');

        if ($chg_theme && $SysPrefs->allow_demo_mode) {
            $_SESSION['wa_current_user']->prefs->theme = $_POST['theme'];
        }

        if ($chg_theme || $chg_lang || $chg_date_format || $chg_date_sep) {
            meta_forward($_SERVER['PHP_SELF']);
        }

        if ($SysPrefs->allow_demo_mode) {
            display_warning(_('显示设置内容已更新。'));
        } else {
            display_notification_centered(_('显示设置内容已更新.'));
        }

    }
}

start_form();

start_outer_table(TABLESTYLE2);

table_section(1);
table_section_title(_('小数点设置'));

number_list_row(_('保留位数:'), 'prices_dec', user_price_dec(), 0, 10);
number_list_row(_('数量位数:'), 'qty_dec', user_qty_dec(), 0, 10);
number_list_row(_('汇率:'), 'rates_dec', user_exrate_dec(), 0, 10);
number_list_row(_('百分比:'), 'percent_dec', user_percent_dec(), 0, 10);

table_section_title(_('日期格式'));

dateformats_list_row(_('日期格式:'), 'date_format', user_date_format());

dateseps_list_row(_('日期间隔符:'), 'date_sep', user_date_sep());

/* The array $dateseps is set up in config.php for modifications
possible separators can be added by modifying the array definition by editing that file */

thoseps_list_row(_('千位符间隔:'), 'tho_sep', user_tho_sep());

/* The array $thoseps is set up in config.php for modifications
possible separators can be added by modifying the array definition by editing that file */

decseps_list_row(_('小数点分割符:'), 'dec_sep', user_dec_sep());

/* The array $decseps is set up in config.php for modifications
possible separators can be added by modifying the array definition by editing that file */

check_row(_('使用日期选项'), 'use_date_picker', user_use_date_picker());

if (!isset($_POST['language'])) {
    $_POST['language'] = $_SESSION['language']->code;
}

table_section_title(_('报表'));

text_row_ex(_('保存报表天数:'), 'save_report_selections', 5, 5, '', user_save_report_selections());

yesno_list_row(_('默认打印格式:'), 'def_print_destination', user_def_print_destination(), $name_yes = _('Excel'), $name_no = _('PDF/Printer'));

yesno_list_row(_('默认打印方向:'), 'def_print_orientation', user_def_print_orientation(),
    $name_yes = _('横向'), $name_no = _('纵向'));

table_section(2);

table_section_title(_('杂项'));

check_row(_('是否为新用户显示提示:'), 'show_hints', user_hints());

check_row(_('显示财务信息'), 'show_gl', user_show_gl_info());

check_row(_('显示项目代码:'), 'show_codes', user_show_codes());

themes_list_row(_('主题:'), 'theme', user_theme());

/* The array $themes is set up in config.php for modifications
possible separators can be added by modifying the array definition by editing that file */

pagesizes_list_row(_('默认纸张大小:'), 'page_size', user_pagesize());

tab_list_row(_('默认标签起始页'), 'startup_tab', user_startup_tab());

/* The array $pagesizes is set up in config.php for modifications
possible separators can be added by modifying the array definition by editing that file */

if (!isset($_POST['print_profile'])) {
    $_POST['print_profile'] = user_print_profile();
}

print_profiles_list_row(_('打印配置文件') . ':', 'print_profile', null, _('浏览器打印支持'));

check_row(_('使用弹出窗显示打印报表:'), 'rep_popup', user_rep_popup(), false, _('确保浏览器支持弹窗'));

check_row(_('使用图标代替文字:'), 'graphic_links', user_graphic_links(), false, _('使用图标链接'));

check_row(_('记住文件最后的日期:'), 'sticky_doc_date', sticky_doc_date(), false, _('如果后续文档中记住了设置的文档日期，则默认为当前日期'));

text_row_ex(_('报表每页显示数:'), 'query_size', 5, 5, '', user_query_size());

//text_row_ex(_('交易天数:'), 'transaction_days', 5, 5, '', user_transaction_days());

//table_section_title(_('语言'));

//languages_list_row(_('语言:'), 'language', $_POST['language']);

end_outer_table(1);

submit_center('setprefs', _('更新'), true, '', 'default');

end_form(2);

end_page();
