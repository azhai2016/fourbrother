<?php


function set_mssql_connection($count = -1)
{
    if ($count<1) return;

    global $path_to_root,$mssqldb;

    include $path_to_root . '/config_db.php';
    $connection = $mssql_db_connections[$count];

    $server = $connection['host'];
    $port = $connection['port'];
    $name = $connection['name'];
    $option = $connection['driver_options'];

    $dsn = sprintf('sqlsrv:Server=%s,%d;Database=%s;ENCRYPT=yes;trustServerCertificate=true',
        $server,
        $port,
        $name);

    $defaultOptions = array(
        PDO::ATTR_TIMEOUT => 30,
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
    );

    $driverOptions = $option + $defaultOptions; // 注意：这里只能使用相加，不能使用array_merge()，因为下标是数值
  // try {
       $mssqldb = new PDO($dsn, $connection['user'], $connection['password'], $driverOptions);
      
    //}
   // catch(PDOException $e) {
   //     return ' MSSQL 数据库错误 : ' . $e->getMessage();;
  //  }

    return $mssqldb;
}

//DB wrapper functions to change only once for whole application

function mssql_db_query($sql, $exceute=0, $err_msg = null)
{
    global $mssqldb, $SysPrefs, $sql_queries, $Ajax;

    if ($SysPrefs->show_sql) {
        $Ajax->activate('footer_debug');
        $sql_queries .= "<pre style='white-space:pre-wrap;word-wrap:break-word;'>$sql</pre>\n<hr>";
    }
 
    $retry = MAX_DEADLOCK_RETRY;

    do {
        try {
            if ($exceute == 1) {
                $result = $mssqldb->exec($sql);
            } else {
                $result = $mssqldb->query($sql);
            }

            $retry = 0;

        } catch (PDOException $e) {
  
            sleep(1);
            $mssqldb->mssql_rollback();
            $retry--;
        }
    } while ($retry);

    return $result;
}


function mssqldb_num_rows($result)
{
    return $result->fetchColumn();
}

function mssqldb_num_fields($result)
{
    return $result->fetchColumn();
}

function mssqldb_field_name($result, $n)
{
    $fieldinfo = $result->fetchAll(\PDO::FETCH_COLUMN,$n);

    return $fieldinfo->name;
}

function mssqldb_fetch_row($result)
{

    $ret = $result->fetch(\PDO::FETCH_NUM);
    return ($ret === null ? array() : $ret);
}

function mssqldb_fetch_assoc($result)
{
    $ret = $result->fetch(\PDO::FETCH_ASSOC);
    return ($ret === null ? array(): $ret);
} 


function mssql_transaction()
{
    global $mssqldb;
    $mssqldb->beginTransaction();
}

function mssql_commit()
{
    global $mssqldb;
    $mssqldb->commit();
}

function mssql_rollback()
{
    global $mssqldb;
    $mssqldb->rollback();
}

function do_fetch($result)
{
    $ret = $result->fetch(\PDO::FETCH_ASSOC);
    return ($ret === null ? false : $ret);
}

function do_fetch_all($result)
{

    $ret = $result->fetchAll(\PDO::FETCH_ASSOC);

    return ($ret === null ? false : $ret);
}

function db_mssql_error_no()
{
    global $mssqldb;

    $error = $mssqldb->errorInfo();

    return $error;
}


function ms_escape_string($data) {
        if ( !isset($data) or empty($data) ) return '';
        if ( is_numeric($data) ) return $data;

        $non_displayables = array(
            '/%0[0-8bcef]/',            // url encoded 00-08, 11, 12, 14, 15
            '/%1[0-9a-f]/',             // url encoded 16-31
            '/[\x00-\x08]/',            // 00-08
            '/\x0b/',                   // 11
            '/\x0c/',                   // 12
            '/[\x0e-\x1f]/'             // 14-31
        );
        foreach ( $non_displayables as $regex )
            $data = preg_replace( $regex, '', array($data) );
        $data = str_replace("'", "''", $data );
        return $data;
    }

function mssql_db_escape($value = '', $nullify = false)
{
    global $db;

    $value = @html_entity_decode($value, ENT_QUOTES, $_SESSION['language']->encoding == 'iso-8859-2' ? 'ISO-8859-1' : $_SESSION['language']->encoding);
    $value = html_specials_encode($value);

    //reset default if second parameter is skipped
    $nullify = ($nullify === null) ? (false) : ($nullify);

    //check for null/unset/empty strings
    if ((!isset($value)) || (is_null($value)) || ($value === '')) {
        $value = ($nullify) ? ('NULL') : ("''");
    } else {
        if (is_string($value)) {
            $value = "'" .$value. "'";
            //value is a string and should be quoted;
        } else if (!is_numeric($value)) {
            //value is not a string nor numeric
            display_error("ERROR: incorrect data type send to sql query");
            echo '<br><br>';
            exit();
        }
    }
    return $value;
}


function db_msqql_close($dbase = null)
{
    global $mssqldb;
    if (!$mssqldb) {
        $mssqldb = null;
    }

    return $mssqldb;
}
