<?php
if (!isset($path_to_root) || isset($_GET['path_to_root']) || isset($_POST['path_to_root'])) {
    die('Restricted access');
}

include_once $path_to_root . '/applications/application.php';
include_once $path_to_root . '/applications/setup.php';
include_once $path_to_root . '/installed_extensions.php';

class AppsBase
{
    public $user;
    public $settings;
    public $applications;
    public $selected_application;

    public $menu;

    public function add_application($app)
    {
        if ($app->enabled) {
            $this->applications[$app->id] = $app;
        }

    }
    public function get_application($id)
    {
        if (isset($this->applications[$id])) {
            return $this->applications[$id];
        }

        return null;
    }
    public function get_selected_application()
    {
        if (isset($this->selected_application)) {
            return $this->applications[$this->selected_application];
        }

        foreach ($this->applications as $application) {
            return $application;
        }

        return null;
    }
    public function display()
    {
        global $path_to_root;

        include_once $path_to_root . '/themes/' . user_theme() . '/renderer.php';

        $this->init();
        $rend = new renderer();
        $rend->wa_header();

        $rend->display_applications($this);

        $rend->wa_footer();
        $this->renderer = &$rend;
    }
    public function init()
    {
        global $SysPrefs;

        $this->menu = new menu(_('主菜单'));
        $this->menu->add_item(_('主菜单'), 'index.php');
        $this->menu->add_item(_('注销'), '/account/access/logout.php');
        $this->applications = array();
    }
}
