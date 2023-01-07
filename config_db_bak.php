<?php

/*Connection Information for the database
$def_coy - the default company that is pre-selected on login

'host' - the computer ip address or name where the database is. The default is 'localhost' assuming that the web server is also the sql server.

'port' - the computer port where the database is. The default is '3306'. Set empty for default.

'dbuser' - the user name under which the company database should be accessed.
  NB it is not secure to use root as the dbuser with no password - a user with appropriate privileges must be set up.

'dbpassword' - the password required for the dbuser to authorise the above database user.

'dbname' - the name of the database as defined in the RDMS being used. Typically RDMS allow many databases to be maintained under the same server.
'collation' - the character set used for the database.
'tbpref' - prefix on table names, or '' if not used. Always use non-empty prefixes if multiply company use the same database.
*/


$def_coy = 0;

$mssql_db_connections = array(
  1 => array(
  'host' => '192.168.8.16',
  'port' => 1433,
  'user' => 'test',
  'password' => 'Test123',
  'name' => 'phprms',
  'charset' => 'utf8', 
  'driver_options' => array(
      3 => 0,
  )),
);

$tb_pref_counter = 2;

$db_connections = array (
  0 => 
  array (
    'name' => '商业系统返利折让管理系统',
    'host' => 'localhost',
    'port' => '3306',
    'dbname' => 'phprms',
    'collation' => NULL,
    'tbpref' => '0_',
    'dbuser' => 'root',
    'dbpassword' => 'azsOFT2022',
  ),
  1 => 
  array (
    'name' => '2022年度数据',
    'host' => 'localhost',
    'port' => '3306',
    'dbuser' => 'root',
    'dbpassword' => 'azsOFT2022',
    'dbname' => 'phprms',
    'collation' => NULL,
    'tbpref' => '1_',
  ),
);
