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
/*
Security sections groups various areas on both functionality and privilege levels.
Often analytic inquires are available only for management, and configuration
for administration or management staff. This is why we have those three
section type inside near every FA module.

Section codes 0-99 are reserved for core FA functionalities.
Every security section can contain up to 256 different areas.
External modules can extend security roles system by adding rows to
$security_sections and $security_areas using section codes >=100.
Security areas and sections created by extension modules/plugins
have dynamically assigned 3-byte integer codes. The highest byte is zero
for sections/areas defined in this file, and extid+1 for those defined
by extensions
 */
define('SS_SADMIN', 1 << 8); // 系统管理
define('SS_SETUP', 2 << 8); // 设置
define('SS_SPEC', 3 << 8); // 属性

$security_sections = array(
    SS_SADMIN => _('系统管理'),
    SS_SETUP => _('功能设置'),
    SS_SPEC => _('其他设置'),   
);

$security_areas = array(
//
    //    Site administration
    //
    'SA_CREATECOMPANY' => array(SS_SADMIN | 1, _('安装组件')),
    'SA_CREATELANGUAGE' => array(SS_SADMIN | 2, _('安装语言')),
    'SA_CREATEMODULES' => array(SS_SADMIN | 3, _('安装更新')),
    'SA_SOFTWAREUPGRADE' => array(SS_SADMIN | 4, _('系统升级')),
    
    //
    //    Company setup
    //

    'SA_SETUPCOMPANY' => array(SS_SETUP | 1, _('项目设置')),
    
    'SA_SECROLES' => array(SS_SETUP | 2, _('角色权限')),
    'SA_USERS' => array(SS_SETUP | 3, _('用户设置')),
    'SA_PRINTERS' => array(SS_SETUP | 4, _('打印机设置')),
    'SA_PRINTPROFILE' => array(SS_SETUP | 5, _('打印属性')),
    
    //
    // Special and common functions
    //

    'SA_BACKUP' => array(SS_SPEC | 1, _('备份/还原')),
    'SA_SETUPDISPLAY' => array(SS_SPEC | 2, _('显示属性')),
    'SA_CHGPASSWD' => array(SS_SPEC | 3, _('更换密码')),
    'SA_DESIGNER' => array(SS_SPEC | 4, _('报表设计器')),
    'SA_ATTACHDOCUMENT'=>array(SS_SPEC | 5, _('附件管理')),
    'SA_PRINT_DESIGNER' => array(SS_SPEC | 7, _('报表设置')),
   
);

if (!@$SysPrefs->allow_gl_reopen) {
    unset($security_areas['SA_GLREOPEN']);
}

/*
This function should be called whenever we want to extend core access level system
with new security areas and/or sections i.e.:
. on any page with non-standard security areas
. in security roles editor
The call should be placed between session.php inclusion and page() call.
Up to 155 security sections and 155 security areas for any extension can be installed.
 */
function add_access_extensions()
{
    global $security_areas, $security_sections, $installed_extensions;

    foreach ($installed_extensions as $extid => $ext) {
        $accext = hook_invoke($ext['package'], 'install_access', $dummy);
        if ($accext == null) {
            continue;
        }

        $scode = 100;
        $acode = 100;
        $extsections = $accext[1];
        $extareas = $accext[0];
        $extcode = $extid << 16;

        $trans = array();
        foreach ($extsections as $code => $name) {
            $trans[$code] = $scode << 8;
            // reassign section codes
            $security_sections[$trans[$code] | $extcode] = $name;
            $scode++;
        }
        foreach ($extareas as $code => $area) {
            $section = $area[0] & 0xff00;
            // extension modules:
            // if area belongs to nonstandard section
            // use translated section codes and
            // preserve lower part of area code
            if (isset($trans[$section])) {
                $section = $trans[$section];
            }

            // otherwise assign next available
            // area code >99
            $area[0] = $extcode | $section | ($acode++);
            $security_areas[$code] = $area;
        }
    }
}

function check_edit_access($name)
{
    global $input_security;

    $access = @$input_security[$name];

    if (!$access) {
        $access = @$input_security[''];
    }
    // default access level

    if (!$access) {
        return true;
    }
    // if constraint is not defined edit access is allowed

    return user_check_access($access);
}
/*
Returns POST value or null if edit access to $name control is forbidden.
 */
function access_post($name, $dflt = null)
{
    if (!check_edit_access($name)) {
        return $dflt;
    } else {
        return get_post($name, $dflt);
    }

}

/*
Returns numeric input value or null if edit access to $name control is forbidden.
 */
function access_num($name, $dflt = null)
{
    if (!check_edit_access($name)) {
        return $dflt;
    } else {
        return input_num($name, $dflt);
    }

}
