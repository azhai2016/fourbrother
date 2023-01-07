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
class SetupApp extends application
{
    public function __construct()
    {
        parent::__construct('system', _($this->help_context = '设置'));

        $this->add_module(_('基本设置'));
        $this->add_lapp_function(0, _('项目设置(&C)'), 'admin/company_preferences.php?', 'SA_SETUPCOMPANY', MENU_SETTINGS);
        $this->add_lapp_function(0, _('用户管理(&U)'), 'admin/users.php?', 'SA_USERS', MENU_SETTINGS);
        $this->add_lapp_function(0, _('权限设置(&R)'), 'admin/security_roles.php?', 'SA_SECROLES', MENU_SETTINGS);
       
        $this->add_module(_('杂项'));
        $this->add_lapp_function(1, _('报表设计器'), 'admin/view/view_print.php', 'SA_PRINT_DESIGNER', MENU_SETTINGS);
        $this->add_lapp_function(1, _('创建帐套'), 'admin/create_coy.php?', 'SA_CREATECOMPANY', MENU_SETTINGS);
        
        $this->add_lapp_function(1, _('系统检测'), 'admin/system_diagnostics.php?', 'SA_SOFTWAREUPGRADE', MENU_SYSTEM);

        $this->add_lapp_function(1, _('数据库备份和还原(&B)'), 'admin/backups.php?', 'SA_BACKUP', MENU_SYSTEM);
        
        //$this->add_rapp_function(1, _('主题管理(&T)'), 'admin/inst_theme.php?', 'SA_CREATEMODULES', MENU_UPDATE);
        $this->add_rapp_function(1, _('安装扩展(&E)'), 'admin/inst_module.php?', 'SA_CREATEMODULES', MENU_UPDATE);
		
        //$this->add_rapp_function(2, _('系统更新升级'), 'admin/inst_upgrade.php?', 'SA_SOFTWAREUPGRADE', MENU_UPDATE);
        //*/
        $this->add_rapp_function(1, _('显示设置(&D)'), 'admin/display_prefs.php?', 'SA_SETUPDISPLAY', MENU_SETTINGS);
        $this->add_rapp_function(1, _('模块打印设置(&P)'), 'admin/print_profiles.php?', 'SA_PRINTPROFILE', MENU_MAINTENANCE);
        $this->add_rapp_function(1, _('打印机设置(&P)'), 'admin/printers.php?', 'SA_PRINTERS', MENU_MAINTENANCE);
       
        $this->add_extensions();
    }
}
