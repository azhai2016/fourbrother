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

// Type of requirement for positive test result
$test_level = array(
	0 => _('信息'),
	1 => _('选项'),
	2 => _('推荐'),
	3 => _('必须')
);

// test mysql server version
function tst_mysql() {
	$test['descr'] = _('MySQL version'). ' >=4.1';
	$test['type'] = 3;
	$test['test'] = db_get_version();
	if (!($test['result'] = ($test['test']>='4.1'))) {
		$db_str = explode('-', $test['test']);
		$test['result'] = ($db_str[1] == 'MariaDB');
	}
	$test['comments'] = _('Mysql版本大于4.1或更高');

	return $test;
}
// test php mysql extension
function tst_phpmysql() {
	$test['descr'] = _('PHP MySQL扩展');
	$test['type'] = 3;
	$test['result'] = db_extension_exists();
	$test['test'] = $test['result'] ? _('Yes'): _('No');
	
	$test['comments'] = _('必须安装PHP支持MySQL的扩展.');
	return $test;
}

function tst_php() {
	$test['descr'] = _('PHP版本').' >=5.0.0';
	$test['type'] = 3;
	$test['test'] = phpversion();
	$test['result'] = $test['test']>='5.0.0';
	$test['comments'] = _('更新至 5.0.0或更高');

	return $test;
}

function tst_system() {
	$test['descr'] = _('服务系统');
	$test['type'] = 0;
	$test['test'] = PHP_OS;
	$test['result'] = true;

	return $test;
}

function tst_sessionpath() {
	$test['descr'] = _('Session保存路径');
	$test['type'] = 0;
	$test['test'] = session_save_path();
    $istcp = strpos(session_save_path(),'tcp:')>-1;

	$test['result'] =  $istcp?'': is_writable(session_save_path());

	$test['comments'] =  !$istcp && !is_writable(session_save_path()) ? _(session_save_path().' 文件权限'):'OK';

	return $test;
}

function tst_install() {
	global $path_to_root;

	$test['descr'] = _('删除安装路向导');
	$test['type'] = 2;
	$test['result'] = !is_dir($path_to_root.'/install');
	$test['test'] = _('Not removed');
	$test['comments'] = _('为了安全请删除安装向导目录.');

	return $test;
}

function tst_browser() {
	$test['descr'] = _('浏览器类型');
	$test['type'] = 0;
	$test['test'] = $_SERVER['HTTP_USER_AGENT'];
	$test['result'] = true;
	$test['comments'] = _('支持所有浏览器');

	return $test;
}

function tst_server() {
	$test['descr'] = _('Http server类型');
	$test['test'] = $_SERVER['SERVER_SOFTWARE'];
	$test['type'] = 0;
	$test['result'] = true;
	$test['comments'] = _('支持所有服务');

	return $test;
}

function tst_gettext() {
	$test['descr'] = _('原生获取文本');
	$test['test'] = function_exists('gettext') ? _('Yes'): _('No');
	$test['type'] = 1;
	$test['result'] = true;
	$test['comments'] = _('case支持文本');

	return $test;
}

function tst_debug() {
	global $SysPrefs;
	$test['descr'] = _('调试模式');
	$test['type'] = 0;
	$test['test'] = $SysPrefs->go_debug ? _("Yes") : _("No");
	$test['result'] = $SysPrefs->go_debug != 0;
	$test['comments'] = _('配置设置文件，打开设置');

	return $test;
}

function tst_logging() {
	global $SysPrefs;
	
	$error_logfile = $SysPrefs->error_logfile;
	$test['descr'] = _('错误日志');
	$test['type'] = 2;
	// if error lgging is on, but log file does not exists try write
	if ($error_logfile && !is_file($error_logfile)) 
		@fclose(@fopen($error_logfile, 'w'));
	$test['result'] = @$error_logfile != '' && is_writable($error_logfile);
	$test['test'] = @$error_logfile == '' ? _("禁用") : $error_logfile;
	
	if (@$error_logfile == '')
		$test['comments'] = _('配置设置 $error_logging');
	else
	if (!is_writable($error_logfile))
		$test['comments'] = _('日志文件权限');
	
	return $test;
}
//
//	Installed FA database structure version
//
function tst_dbversion() {
	global $db_version;
	$test['descr'] = _('本地数据库版本');
	$test['type'] = 3;
	$test['test'] = get_company_pref('version_id');
	$test['result'] = $test['test'] == $db_version;
	$test['comments'] = _('数据库结构似乎未升级到当前版本')." ($db_version)";

	return $test;
}


function tst_subdirs($install=false) {
	global $db_connections;

	$comps = $install ? array('0') : array_keys($db_connections);

	$comp_subdirs = array('images', 'pdf_files', 'backup','js_cache');

	$test['descr'] = _('公司子目录相同');
	$test['type'] = 3;
	$test['test'] = array(company_path().'/*');
	foreach($comp_subdirs as $sub) {
		$test['test'][] = company_path().'/*/'.$sub;
	}
	$test['result'] = true;
	
	$comp_path = company_path();
	foreach ($comps as $n) {
		$path = company_path($n);
		if (!is_dir($path) || !is_writable($path) ) {
			$test['result'] = false;
			$test['comments'][] = sprintf(_("'%s'只读"), $path);
			continue;
		};
		foreach($comp_subdirs as $sub) {
			$spath = $path.'/'.$sub;
			if (!is_dir($spath) || !is_writable($spath) ) {
				$test['result'] = false;
				$test['comments'][] = sprintf(_("'%s' 只读"), $spath);
			}
			else {
				$dir = opendir($spath);
				while (false !== ($fname = readdir($dir))) {
					// check only *.js files. Manually installed package can contain other
					// non-writable files which are non-crucial for normal operations
					if (preg_match('/.*(\.js)/', $fname) && !is_writable("$spath/$fname")) {
						$test['result'] = false;
						$test['comments'][] = sprintf(_("'%s'只读"), "$spath/$fname");
					}
				}
			}
		}
	}
	return $test;
}

function tst_tmpdir() {
	global $path_to_root;
	
	$test['descr'] = _('临时目录');
	$test['type'] = 3;
	$test['test'] = VARLIB_PATH;
	$test['result'] = is_dir($test['test']) && is_writable($test['test']);
	$test['comments'][] = sprintf(_("'%s'只读"), $test['test']);
	return $test;
}

function tst_langs($install) {
	global $installed_languages, $path_to_root, $GetText;

	$test['descr'] = _('语言配置');
	$test['type'] = 3;
	$test['result'] = true;
	$test['comments'] = array();

	$fname =  $path_to_root.'/lang';
	$test['test'] = $fname;
	if (!(is_dir($fname) && is_writable($fname))) {
		$test['result'] = false;
		$test['comments'][] = _("语言文件夹应是可写的.");
		return $test;
	}
	
	if (!$install) {
		$fname =  $path_to_root.'/lang/installed_languages.inc';
		$test['test'] = $fname;
		if (!(is_file($fname) && is_writable($fname))) {
			$test['result'] = false;
			$test['comments'][] = _("语言文件夹应是可写的.");
			return $test;
		}
	}

	$langs = array();
	
	foreach ($installed_languages as $lang) {

		if ($lang['code'] == 'C') continue; // no translation (English)
		$langs[] = $lang['code'];

		$file = $path_to_root.'/lang/'.$lang['code'].'/LC_MESSAGES/'.$lang['code'];
		if (@$lang['version'])
			$file .= '-'.$lang['version'];
		$file .= function_exists('gettext') ? '.mo' : '.po';

		if (!is_file($file)) {
			$test['result'] = false;
			$test['comments'][] = sprintf( _('缺少 %s 翻译文件.'), $file);
		}
		if (!$GetText->check_support($lang['code'], $lang['encoding'])) {
			$test['result'] = false;
			$test['comments'][] = sprintf(_('缺少系统区域设置: %s'), $lang['code'].".".$lang['encoding']);
		};
	}

	$test['test'] = $langs;

	return $test;
}

function tst_config($install) {
	global $path_to_root;

	$test['descr'] = _('主配置文件');
	$test['test'] = $path_to_root.'/config.php';
	if ($install) {
		$test['type'] = 3;
		$writable = check_write($test['test']);
		$test['result'] = $writable==1;
		$test['comments'][] = $writable == 0 ?
			sprintf(_("无法写入“%s”文件。检查FA目录写入权限。"), $test['test'])
			: sprintf(_("'%s'已存在"), $test['test']);
	}
	else {
		$test['type'] = 2;
		$test['result'] = is_file($test['test']) && !is_writable($test['test']);
		$test['comments'][] = sprintf(_("'%s' 文件应为只读"), $test['test']);
	}
	return $test;
}

function tst_dbconfig($install) {
	global $path_to_root;

	$test['descr'] = _('数据库身份验证文件');
	$test['test'] = $path_to_root.'/config_db.php';

	if ($install) {
		$test['type'] = 3;
		$writable = check_write($test['test']);
		$test['result'] = $writable==1;
		$test['comments'][] = $writable == 0 ?
			sprintf(_("无法写入“%s”文件。检查FA目录写入权限."), $test['test'])
			: sprintf(_("'%s'已存在."), $test['test']);
	}
	else {
		$test['type'] = 2;
		$test['result'] = is_file($test['test']) && !is_writable($test['test']);
		$test['comments'][] = sprintf(_("'%s'如果不打算添加或更改公司，则该文件应为只读"), $test['test']);
	}
	return $test;
}

function tst_extconfig($install) {
	global $path_to_root, $db_connections;

	$comps = $install ? array('0') : array_keys($db_connections);
	
	$test['descr'] = _('扩展系统');
	$test['type'] = 3;
	$fname =  $path_to_root.'/installed_extensions.php';
	$test['test'][] = $fname;
	$test['result'] = ($install || is_file($fname)) && check_write($fname);
	$test['test'][] = company_path().'/*/installed_extensions.php';
	if (!$test['result'])
		$test['comments'][] = sprintf(_("'%s' 只读"), $fname);

	foreach ($comps as $n) {
		$path = company_path($n);
		if (!is_dir($path)) continue;

		$path .= "/installed_extensions.php";
		if ((!$install && !is_file($path)) || !check_write($path) ) {
			$test['result'] = false;
			$test['comments'][] = sprintf(_("'%s'只读"), $path);
			continue;
		};
	}
	foreach(array('modules', 'modules/_cache', 'themes', 'sql') as $dir) {
		$fname =  $path_to_root.'/'.$dir;
		$test['test'][] = $fname;
		$t = is_dir($fname) && is_writable($fname);
		if (!$t)
			$test['comments'][] = sprintf(_("'%s'只读"), $fname);
		$test['result'] &= $t;
	}
	foreach(array('Release', 'Themes', 'Languages', 'Extensions', 'Charts') as $file) {
		$fname = $path_to_root."/modules/_cache/".$file.".gz";
		$t = !file_exists($fname) || is_writable($fname);
		if (!$t)
			$test['comments'][] = sprintf(_("'%s'只读"), $fname);
		$test['result'] &= $t;
	}
	if(!$test['result'])
		$test['comments'][] = _('扩展配置文件和目录应该是可写的');

	$fname = $path_to_root."/themes";
	$themedir = opendir($fname);
	while (false !== ($fname = readdir($themedir))) {
		if ($fname!='.' && $fname!='..' && is_dir($path_to_root.'/themes/'.$fname) && !in_array($fname, array('canvas', 'default', 'dropdown'))) {
			$test['test'][] = $fname;
			$test['result'] = is_writable($path_to_root.'/themes/'.$fname);
			if (!$test['result']) {
				$test['comments'][] = sprintf(_("非标准主题目录“%s”不可写"), $fname);
				break;
			}
		}
	}
	closedir($themedir);

	$test['test'][] = 'OpenSSL PHP扩展';
	if (!extension_loaded('openssl')) {
		$test['result'] = false;
		$test['comments'][] = _('必须启用OpenSSL PHP扩展才能使用扩展存储库系统');
	}
	elseif (!function_exists('openssl_verify')) {
		$test['result'] = false;
		$test['comments'][] = _('要使用扩展存储库系统，必须在服务器上提供OpenSSL。');
	}
	return $test;
}

function display_system_tests($install = false) {
	global $test_level;

	if ($install)
		$system_tests = array('tst_php', 'tst_phpmysql', 'tst_system', 'tst_dbconfig', 
			'tst_config',
			'tst_subdirs', 'tst_langs', 'tst_tmpdir', 'tst_sessionpath', 'tst_extconfig'
		);
	else
		$system_tests = array('tst_mysql', 'tst_php', 'tst_server', 'tst_system', 'tst_browser',
			'tst_gettext', 'tst_debug', 'tst_logging',
			'tst_dbversion', 'tst_subdirs', 'tst_langs', 'tst_tmpdir', 'tst_sessionpath',
			'tst_install', 'tst_dbconfig', 'tst_config', 'tst_extconfig'
		);


	start_table(TABLESTYLE, "width='80%'");
	$th = array(_('项目'), _('类型'), _('值'), _('说明'));
	table_header($th);

	$ret = true;
	$k = 0; //row colour counter
	foreach ($system_tests as $test) {
		alt_table_row_color($k);
		$result = $test($install);
		if (!$result) continue;
		
		label_cell($result['descr']);
		label_cell($test_level[$result['type']]);

		$res = is_array(@$result['test']) ? implode('<br>', $result['test']) : $result['test'];
		label_cell($res);

		$comm = is_array(@$result['comments']) ? implode('<br>', $result['comments']) : @$result['comments'];
		$color = ($result['result'] ? 'green': ($result['type']==3 ? 'red' : ($result['type']==2 ? 'orange' : 'green')));
		label_cell("<span style='color:$color'>".($result['result'] ? _('Ok') : '<b>'.$comm.'</b>').'</span>');
		end_row();
		$ret = $ret && (($result['result']!=0) || ($result['type'] < 3));
	}
	end_table();

	return $ret;
}