<?php

function get_sql_base_info(){
   $rs = array(); 
   $sql = 'select count(1) as ct  from products';
   $result = mssql_db_query($sql,0);
   $rs['product'] = mssqldb_fetch_row($result);

   $sql = 'select count(1) as ct from customers';
   $result = mssql_db_query($sql,0);
   $rs['customer'] = mssqldb_fetch_row($result);

   $sql = 'select count(1) as ct from supplys';    
   $result = mssql_db_query($sql,0);

   $rs['supply'] = mssqldb_fetch_row($result);
   return $rs;
}

?>