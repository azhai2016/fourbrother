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
$page_security = 'SA_CREATEMODULES';
$path_to_root = '..';
include_once($path_to_root.'/includes/session.php');
include_once($path_to_root.'/includes/packages.inc');

if ($SysPrefs->use_popup_windows)
	$js = get_js_open_window(900, 500);

page(_($help_context = '安装扩展'), false, false, '', $js);

include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/admin/db/company_db.inc');
include_once($path_to_root.'/admin/db/maintenance_db.inc');
include_once($path_to_root.'/includes/ui.inc');

simple_page_mode(true);

//---------------------------------------------------------------------------------------------

function local_extension($id) {
	global $next_extension_id, $Ajax, $path_to_root;

	$exts = get_company_extensions();
	$exts[$next_extension_id++] = array(
		'package' => $id,
		'name' => $id,
		'version' => '-',
		'available' => '',
		'type' => 'extension',
		'path' => 'modules/'.$id,
		'active' => false
	);

	$local_module_path = $path_to_root.'/modules/'.clean_file_name($id);
	$local_config_file = $local_module_path.'/_init/config';
	$local_hook_file = $local_module_path.'/hooks.php';

	if (file_exists($local_config_file)) {
		$ctrl = get_control_file($local_config_file);
	
		if (key_exists('Name', $ctrl)) $exts[$next_extension_id-1]['name'] = $ctrl['Name'];
		if (key_exists('Version', $ctrl)) $exts[$next_extension_id-1]['version'] = $ctrl['Version'];
	}
	if (file_exists($local_hook_file))
		include_once($local_hook_file);

	$hooks_class = 'hooks_'.$id;
	if (class_exists($hooks_class, false)) {
		$hooks = new $hooks_class;
		$hooks->install_extension(false);
	}
	$Ajax->activate('ext_tbl'); // refresh settings display
	if (!update_extensions($exts))
		return false;
	return true;
}

function handle_delete($id) {
	global $path_to_root;
	
	$extensions = get_company_extensions();
	$ext = $extensions[$id];
	if ($ext['version'] != '-') {
		if (!uninstall_package($ext['package']))
			return false;
	}
	else {
		@include_once($path_to_root.'/'.$ext['path'].'/hooks.php');
		$hooks_class = 'hooks_'.$ext['package'];
		if (class_exists($hooks_class)) {
			$hooks = new $hooks_class;
			$hooks->uninstall_extension(false);
		}
	}
	unset($extensions[$id]);
	if (update_extensions($extensions))
		display_notification(_("Selected extension has been successfully deleted"));
	
	return true;
}
//
// Helper for formating menu tabs/entries to be displayed in extension table
//
function fmt_titles($defs) {
	if (!$defs) return '';
	foreach($defs as $def) {
		$str[] = access_string($def['title'], true);
	}
	return implode('<br>', array_values($str));
}

//---------------------------------------------------------------------------------------------
//
// Display list of all extensions - installed and available from repository
//
function display_extensions($mods) {
	global $installed_extensions;
	
	div_start('ext_tbl');
	start_table(TABLESTYLE);

	$th = array(_('名称'),_('模块说明'), _('选项'), _('版本'), _('激活'),  '安装', '删除');
	table_header($th);

	$k = 0;

	foreach($mods as $pkg_name => $ext) {
		$available = @$ext['available'];
		$installed = isset($ext['version']) ? $ext['version'] : '';
		$id = @$ext['local_id'];

		$entries = fmt_titles(@$ext['entries']);
		$tabs = fmt_titles(@$ext['tabs']);
		$description = $ext['description'];

		alt_table_row_color($k);

		label_cell($available ? get_package_view_str($pkg_name, $ext['name']) : $ext['name']);
		label_cell($description);
		label_cell($entries);
		label_cell($id === null ? _('无') : (($installed && ($installed != '-' || $installed != '')) ? $installed : _('Unknown')));
		label_cell($available ? $available : _('未激活'));

		if (!$available && $ext['type'] == 'extension')	{// third-party plugin
			if (!$installed)
				button_cell('Local'.$ext['package'], _("安装"), _('安装第三方扩展.'), ICON_DOWN);
			else
				label_cell('');
		}
		elseif (check_pkg_upgrade($installed, $available)) // outdated or not installed extension in repo
			button_cell('Update'.$pkg_name, $installed ? _('更新') : _('安装'), _('上传并安装扩展表'), ICON_DOWN);
		else
			label_cell('');

		if ($id !== null) {
			delete_button_cell('Delete'.$id, _('删除'));
			submit_js_confirm('Delete'.$id, 
				sprintf(_("您将要删除包 \'%s\'.\n是否要继续"), $ext['name']));
		}
		else
			label_cell('');

		end_row();
	}

	end_table(1);

	submit_center_first('Refresh', _("更新"), '', null);

	div_end();
}

//---------------------------------------------------------------------------------
//
// Get all installed extensions and display
// with current status stored in company directory.
//
function company_extensions($id) {
	start_table(TABLESTYLE);
	
	$th = array(_('名称'),_('模块'), _('选项'), _('激活'));
	
	$mods = get_company_extensions();
	$exts = get_company_extensions($id);
	foreach($mods as $key => $ins) {
		foreach($exts as $ext)
			if ($ext['name'] == $ins['name']) {
				$mods[$key]['active'] = @$ext['active'];
				continue 2;
			}
	}
	$mods = array_natsort($mods, null, 'name');

	table_header($th);
	$k = 0;
	foreach($mods as $i => $mod) {
		if ($mod['type'] != 'extension') continue;
		alt_table_row_color($k);
		label_cell($mod['name']);
		$entries = fmt_titles(@$mod['entries']);
		$tabs = fmt_titles(@$mod['tabs']);
		label_cell($tabs);
		label_cell($entries);

		check_cells(null, 'Active'.$i, @$mod['active'] ? 1:0, false, false, "align='center'");
		end_row();
	}

	end_table(1);
	submit_center('Refresh', _('更新'), true, false, 'default');
}

//---------------------------------------------------------------------------------------------

if ($Mode == 'Delete') {
	handle_delete($selected_id);
	$Mode = 'RESET';
}

if (get_post('Refresh')) {
	$comp = get_post('extset');
	$exts = get_company_extensions($comp);
    log_b($exts);
	$result = true;
	foreach($exts as $i => $ext) {
		if ($ext['package'] && ($ext['active'] ^ check_value('Active'.$i))) {
			if (check_value('Active'.$i) && !check_src_ext_version($ext['version'])) {
				display_warning(sprintf(_("扩展 '%s'与当前应用程序版本不兼容，无法激活.\n")._('检查“安装/激活”页面以获取更新的软件包版本。'), $ext['name']));
				continue;
			}
			$activated = activate_hooks($ext['package'], $comp, !$ext['active']);	// change active state

			if ($activated !== null)
				$result &= $activated;
			if ($activated || ($activated === null))
				$exts[$i]['active'] = check_value('Active'.$i);
		}
	}
	write_extensions($exts, get_post('extset'));
	if (get_post('extset') == user_company())
		$installed_extensions = $exts;
	
	if(!$result) {
		display_error(_('某些扩展的状态更改失败。'));
		$Ajax->activate('ext_tbl'); // refresh settings display
	}
	else
		display_notification(_('当前活动扩展集已保存。'));
}

if ($id = find_submit('Update', false))
	install_extension($id);
if ($id = find_submit('Local', false))
	local_extension($id);

if ($Mode == 'RESET') {
	$selected_id = -1;
	unset($_POST);
}

//---------------------------------------------------------------------------------------------

start_form(true);
if (list_updated('extset'))
	$Ajax->activate('_page_body');

$set = get_post('extset', -1);

echo '<center>' . _('扩展:') . '&nbsp;&nbsp;';
echo extset_list('extset', null, true);
echo '</center><br>';

if ($set == -1) {
	$mods = get_extensions_list('extension');

	if (!$mods)
		display_note(_('当前没有可选的扩展模块可用。'));
	else
		display_extensions($mods);

}
else 
	company_extensions($set);

end_form();
end_page();