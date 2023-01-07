<?php
define('VARLIB_PATH', $path_to_root . '/tmp');
define('VARLOG_PATH', $path_to_root . '/tmp');
define('LOG_PATH', $path_to_root . '/log');
define('SECURE_ONLY', null);

class SessionManager
{
    public function sessionStart($name, $limit = 0, $path = '/', $domain = null, $secure = null)
    {

        session_name($name);


        $https = isset($secure) ? $secure : (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] !== 'off');


        if (version_compare(PHP_VERSION, '5.2', '<')) {
            session_set_cookie_params($limit, $path, $domain, $https);
        } else {
            session_set_cookie_params($limit, $path, $domain, $https, true);
        }

        session_start();


        if ($this->validateSession()) {

            if (!$this->preventHijacking()) {
      
                $_SESSION = array();
                $_SESSION['IPaddress'] = $_SERVER['REMOTE_ADDR'];
                $_SESSION['userAgent'] = @$_SERVER['HTTP_USER_AGENT'];
                $this->regenerateSession();

         
            } elseif (rand(1, 100) <= 5) {
                $this->regenerateSession();
            }

        } else {
            $_SESSION = array();
            session_destroy();
            session_start();
        }
    }

    public function preventHijacking()
    {
        if (!isset($_SESSION['IPaddress']) || !isset($_SESSION['userAgent'])) {
            return false;
        }

        if ($_SESSION['IPaddress'] != $_SERVER['REMOTE_ADDR']) {
            return false;
        }

        if ($_SESSION['userAgent'] != @$_SERVER['HTTP_USER_AGENT']) {
            return false;
        }

        return true;
    }

    public function regenerateSession()
    {
   
        if (isset($_SESSION['OBSOLETE']) && ($_SESSION['OBSOLETE'] == true)) {
            return;
        }

       
        $_SESSION['OBSOLETE'] = true;
        $_SESSION['EXPIRES'] = time() + 30;


        session_regenerate_id();
   
        $newSession = session_id();
        session_write_close();
     

        session_id($newSession);
        session_start();

  
        unset($_SESSION['OBSOLETE']);
        unset($_SESSION['EXPIRES']);
    }

    public function validateSession()
    {
        if (isset($_SESSION['OBSOLETE']) && !isset($_SESSION['EXPIRES'])) {
            return false;
        }

        if (isset($_SESSION['EXPIRES']) && $_SESSION['EXPIRES'] < time()) {
            return false;
        }

        return true;

    }
}

function output_html($text)
{
    global $before_box, $Ajax, $messages;

    if ($text && preg_match('/\bFatal error(<.*?>)?:(.*)/i', $text, $m)) {
        $Ajax->aCommands = array(); // Don't update page via ajax on errors
        $text = preg_replace('/\bFatal error(<.*?>)?:(.*)/i', '', $text);
        $messages[] = array(E_ERROR, $m[2], null, null);
    }
    $Ajax->run();
    return in_ajax() ? fmt_errors() : ($before_box . fmt_errors() . $text);
}


function kill_login()
{
    session_unset();
    session_destroy();
}


function login_fail()
{
    global $path_to_root;

    header('HTTP/1.1 401 Authorization Required');
    echo "<center><br><br><font size='5' color='red'><b>" . _('密码不正确') . '<b></font><br><br>';
    echo '<b>' . _('用户和密码组合对系统无效。') . '<b><br><br>';
    echo _('如果您不是授权用户，请与您的系统管理员联系，以获取一个帐户，使您能够使用该系统。');
    echo "<br><a href='" . $path_to_root . "/index.php'>" . _('重试') . '</a>';
    echo '</center>';
    kill_login();
    die();
}

function password_reset_fail()
{
    global $path_to_root;

    echo "<center><br><br><font size='5' color='red'><b>" . _('Email错误') . '<b></font><br><br>';
    echo '<b>' . _('该电子邮件地址在系统中不存在，或由多个用户使用。') . '<b><br><br>';

    echo _('请重试或联系系统管理员以获取新密码。');
    echo "<br><a href='" . $path_to_root . "/index.php?reset=1'>" . _('重试') . '</a>';
    echo '</center>';

    kill_login();
    die();
}

function password_reset_success()
{
    global $path_to_root;

    echo "<center><br><br><font size='5' color='green'><b>" . _('已发送新密码') . '<b></font><br><br>';
    echo '<b>' . _('新密码sent新密码已发送到您的邮箱。') . '<b><br><br>';

    echo "<br><a href='" . $path_to_root . "/index.php'>" . _('登录') . '</a>';
    echo '</center>';

    kill_login();
    die();
}

function check_faillog()
{
    global $SysPrefs, $login_faillog;

    $user = $_SESSION['wa_current_user']->user;

    $_SESSION['wa_current_user']->login_attempt++;
    if (@$SysPrefs->login_delay && (@$login_faillog[$user][$_SERVER['REMOTE_ADDR']] >= @$SysPrefs->login_max_attempts) && (time() < $login_faillog[$user]['last'] + $SysPrefs->login_delay)) {
        return true;
    }

    return false;
}


function cache_invalidate($filename)
{
    if (function_exists('opcache_invalidate')) {
        opcache_invalidate($filename);
    }

}


function write_login_filelog($login, $result)
{
    global $login_faillog, $SysPrefs, $path_to_root;

    $user = $_SESSION['wa_current_user']->user;

    $ip = $_SERVER['REMOTE_ADDR'];

    if (!isset($login_faillog[$user][$ip]) || $result) {
        $login_faillog[$user] = array($ip => 0, 'last' => '');
    }

    if (!$result) {
        if ($login_faillog[$user][$ip] < @$SysPrefs->login_max_attempts) {

            $login_faillog[$user][$ip]++;
        } else {
            $login_faillog[$user][$ip] = 0; // comment out to restart counter only after successfull login.
            error_log(sprintf(_("检测到对帐户“%s”的暴力攻击。暂时阻止未登录用户的访问。"), $login));
        }
        $login_faillog[$user]['last'] = time();
    }

    $msg = "<?php\n";
    $msg .= "/*\n";
    $msg .= "Login attempts info.\n";
    $msg .= "*/\n";
    $msg .= "\$login_faillog = " . var_export($login_faillog, true) . ";\n";

    $filename = VARLIB_PATH . '/faillog.php';

    if ((!file_exists($filename) && is_writable(VARLIB_PATH)) || is_writable($filename)) {
        file_put_contents($filename, $msg);
        cache_invalidate($filename);
    }
}

//----------------------------------------------------------------------------------------

function check_page_security($page_security)
{
    global $SysPrefs;

    $msg = '';

    if (!$_SESSION['wa_current_user']->check_user_access()) {
        $msg = $_SESSION['wa_current_user']->old_db ? _('尚未为您的用户帐户定义安全设置。') . '<br>' . _('请联系您的系统管理员。') : _("请从配置中删除\$security_组和\$security_标题文件！");
    } elseif (!$SysPrefs->db_ok && !$_SESSION['wa_current_user']->can_access('SA_SOFTWAREUPGRADE')) {
        $msg = _('在系统管理员完成数据库升级之前，对应用程序的访问已被阻止。');
    }

    if ($msg) {
        display_error($msg);
        end_page(@$_REQUEST['popup']);
        kill_login();
        exit;
    }

    if (!$_SESSION['wa_current_user']->can_access_page($page_security)) {

        echo '<center><br><br><br><b>';
        echo _('没有权限访问，请联系系统管理员');
        echo '</b>';
        echo '<br><br><br><br></center>';
        end_page(@$_REQUEST['popup']);
        exit;
    }
    if (!$SysPrefs->db_ok && !in_array($page_security, array('SA_SOFTWAREUPGRADE', 'SA_OPEN', 'SA_BACKUP'))) {
        display_error(_('系统在源代码升级后被阻止，直到系统/软件升级页面上的数据库更新'));
        end_page();
        exit;
    }

}


function set_page_security($value = null, $trans = array(), $gtrans = array())
{
    global $page_security;

    foreach ($gtrans as $key => $area) {
        if (isset($_GET[$key])) {
            $page_security = $area;
            return;
        }
    }

    if (isset($trans[$value])) {
        $page_security = $trans[$value];
        return;
    }
}


function strip_quotes($data)
{
    if (version_compare(phpversion(), '5.4', '<') && get_magic_quotes_gpc()) {
        if (is_array($data)) {
            foreach ($data as $k => $v) {
                $data[$k] = strip_quotes($data[$k]);
            }
        } else {
            return stripslashes($data);
        }

    }
    return $data;
}


function html_specials_encode($str)
{
    return htmlspecialchars($str, ENT_QUOTES, $_SESSION['language']->encoding == 'iso-8859-2' ? 'ISO-8859-1' : $_SESSION['language']->encoding);
}

function html_cleanup(&$parms)
{
    foreach ($parms as $name => $value) {
        if (is_array($value)) {
            html_cleanup($parms[$name]);
        } else {
            $parms[$name] = html_specials_encode($value);
        }

    }
    reset($parms); 
}

function login_timeout()
{

    if ($_SESSION['wa_current_user']->logged) {
        $tout = $_SESSION['wa_current_user']->timeout;
        if ($tout && (time() > $_SESSION['wa_current_user']->last_act + $tout)) {
            $_SESSION['wa_current_user']->logged = false;
        }

        $_SESSION['wa_current_user']->last_act = time();
    }
}

if (!isset($path_to_root)) {
    $path_to_root = '.';
}


if (isset($_GET['path_to_root']) || isset($_POST['path_to_root'])) {
    die('Restricted access');
}

include_once $path_to_root . '/includes/errors.inc';

set_error_handler('error_handler');
set_exception_handler('exception_handler');

include_once $path_to_root . '/includes/current_user.inc';
include_once $path_to_root . '/Apps.php';
include_once $path_to_root . '/admin/db/security_db.inc';
include_once $path_to_root . '/includes/lang/language.inc';
include_once $path_to_root . '/config_db.php';
include_once $path_to_root . '/includes/ajax.inc';
include_once $path_to_root . '/includes/ui/ui_msgs.inc';
include_once $path_to_root . '/includes/prefs/sysprefs.inc';

include_once $path_to_root . '/includes/hooks.inc';


foreach ($installed_extensions as $ext) {
    if (file_exists($path_to_root . '/' . $ext['path'] . '/hooks.php')) {
        include_once $path_to_root . '/' . $ext['path'] . '/hooks.php';
    }

}

ini_set('session.gc_maxlifetime', 36000); 

$Session_manager = new SessionManager();
$Session_manager->sessionStart('Notrinos' . md5(dirname(__FILE__)), 0, '/', null, SECURE_ONLY);

$_SESSION['SysPrefs'] = new sys_prefs();

$SysPrefs = &$_SESSION['SysPrefs'];

function Log_bug($msg)
{
    global $SysPrefs;
    return $SysPrefs->log()->debug($msg);
}

function log_b($msg)
{
    return Log_bug($msg);
}

function log_i($msg)
{
    global $SysPrefs;
    return $SysPrefs->log()->info($msg);
}

function log_w($msg)
{
    global $SysPrefs;
    return $SysPrefs->log()->warn($msg);
}

if ((!isset($SysPrefs->login_delay)) || ($SysPrefs->login_delay < 0)) {
    $SysPrefs->login_delay = 10;
}

if ((!isset($SysPrefs->login_max_attempts)) || ($SysPrefs->login_max_attempts < 0)) {
    $SysPrefs->login_max_attempts = 3;
}

if ($SysPrefs->go_debug > 0) {
    $cur_error_level = -1;
} else {
    $cur_error_level = E_USER_WARNING | E_USER_ERROR | E_USER_NOTICE;
}

error_reporting($cur_error_level);
ini_set('display_errors', 'On');

if ($SysPrefs->error_logfile != '') {
    ini_set('error_log', $SysPrefs->error_logfile);
    ini_set('ignore_repeated_errors', 'On');
    ini_set('log_errors', 'On');
}


hook_session_start(@$_POST['company_login_name']);

header('Cache-control: private');

get_text_init();

if ($SysPrefs->login_delay > 0 && file_exists(VARLIB_PATH . '/faillog.php')) {
    include_once VARLIB_PATH . '/faillog.php';
}


if (!isset($_SESSION['wa_current_user']) || !$_SESSION['wa_current_user']->logged_in() || !isset($_SESSION['language']) || !method_exists($_SESSION['language'], 'set_language')) {
    $l = array_search_value($dflt_lang, $installed_languages, 'code');
    $_SESSION['language'] = new language($l['name'], $l['code'], $l['encoding'], (isset($l['rtl']) && $l['rtl'] === true) ? 'rtl' : 'ltr');
}

$_SESSION['language']->set_language($_SESSION['language']->code);

include_once $path_to_root . '/includes/access_levels.inc';
include_once $path_to_root . '/version.php';
include_once $path_to_root . '/includes/main.inc';
include_once $path_to_root . '/includes/app_entries.inc';


$Ajax = new Ajax();


$Validate = array();

$Editors = array();

$Pagehelp = array();

$Refs = new references();


register_shutdown_function('end_flush');
ob_start('output_html', 0);

if (!isset($_SESSION['wa_current_user'])) {
    $_SESSION['wa_current_user'] = new current_user();
}

html_cleanup($_GET);
html_cleanup($_POST);
html_cleanup($_REQUEST);
html_cleanup($_SERVER);


if (!defined('FA_LOGOUT_PHP_FILE')) {

    login_timeout();

    if (!$_SESSION['wa_current_user']->old_db && file_exists($path_to_root . '/company/' . user_company() . '/installed_extensions.php')) {
        include $path_to_root . '/company/' . user_company() . '/installed_extensions.php';
    }

    install_hooks();

    if (!$_SESSION['wa_current_user']->logged_in()) {
        if (@$SysPrefs->allow_password_reset && !$SysPrefs->allow_demo_mode && (isset($_GET['reset']) || isset($_POST['email_entry_field']))) {
            if (!isset($_POST['email_entry_field'])) {
                include $path_to_root . '/access/password_reset.php';
                exit();
            } else {
                if (isset($_POST['company_login_nickname']) && !isset($_POST['company_login_name'])) {
                    for ($i = 0; $i < count($db_connections); $i++) {
                        if ($db_connections[$i]['name'] == $_POST['company_login_nickname']) {
                            $_POST['company_login_name'] = $i;
                            unset($_POST['company_login_nickname']);
                            break 1; // cannot pass variables to break from PHP v5.4 onwards
                        }
                    }
                }
                $_succeed = isset($db_connections[$_POST['company_login_name']]) &&
                $_SESSION['wa_current_user']->reset_password($_POST['company_login_name'],
                    $_POST['email_entry_field']);
                if ($_succeed) {
                    password_reset_success();
                }

                password_reset_fail();
            }
        }

        if (!isset($_POST['user_name_entry_field']) or $_POST['user_name_entry_field'] == '') {

            $_SESSION['timeout'] = array('uri' => preg_replace('/JsHttpRequest=(?:(\d+)-)?([^&]+)/s', '', html_specials_encode($_SERVER['REQUEST_URI'])), 'post' => $_POST);
            if (in_ajax()) {
                $Ajax->popup($path_to_root . '/access/timeout.php');
            } else {
                include $path_to_root . '/access/login.php';
            }

            exit;
        } else {
            if (isset($_POST['company_login_nickname']) && !isset($_POST['company_login_name'])) {
                for ($i = 0; $i < count($db_connections); $i++) {
                    if ($db_connections[$i]['name'] == $_POST['company_login_nickname']) {
                        $_POST['company_login_name'] = $i;
                        unset($_POST['company_login_nickname']);
                        break 1; // cannot pass variables to break from PHP v5.4 onwards
                    }
                }
            }
            $succeed = isset($db_connections[$_POST['company_login_name']]) && $_SESSION['wa_current_user']->login($_POST['company_login_name'], $_POST['user_name_entry_field'], $_POST['password']);

            $_SESSION['wa_current_user']->ui_mode = $_POST['ui_mode'];
            if (!$succeed) {
       
                if (isset($_SESSION['timeout'])) {
                    include $path_to_root . '/access/login.php';
                    exit;
                } else {
                    login_fail();
                }

            } elseif (isset($_SESSION['timeout']) && !$_SESSION['timeout']['post']) {

                header('HTTP/1.1 307 Temporary Redirect');
                header('Location: ' . $_SESSION['timeout']['uri']);
                exit();
            }
            $lang = &$_SESSION['language'];
            $lang->set_language($_SESSION['language']->code);
        }
    } else {
        set_global_connection();

        db_set_encoding($_SESSION['language']->encoding);

        $SysPrefs->refresh();
    }
    if (!isset($_SESSION['App'])) {
        $_SESSION['App'] = new Apps();
        $_SESSION['App']->init();
    }
}


$_POST = strip_quotes($_POST);
