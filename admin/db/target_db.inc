<?php


function get_target_data($id){

	$sql = "select * from targetData where sourceId =".$id;

	$result = mssql_db_query($sql, 'could not do check empty query');		

	return $result;
}


function get_target_id($id){
    $sql = "select * from targetData where targetId =".$id;

	$result = mssql_db_query($sql, 'could not do check empty query');		

	$myrow = mssqldb_fetch_assoc($result);

	return $myrow;   
}

function delete_target_id($id,$cls,$sid){
    $sql = "delete from targetData where targetid='".$id."' and sourceId='".$sid."'";
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}


function add_target_data($targetId, $targetName, $targetType, $targetDataType, $sqlScript, $tablename, $defaultValue,$sourceId,$fieldkey)
{
    $sql = sprintf("insert into targetData (targetId, targetName, targetType, targetDataType, sqlScript, tablename, defaultValue,sourceId,fieldkey) 
           values ('%s','%s','%s','%s','%s','%s','%s',%d,'%s')",
           $targetId, $targetName, $targetType, $targetDataType, $sqlScript, $tablename, $defaultValue,$sourceId,$fieldkey
        );
    $result = mssql_db_query($sql, 1,'could not do check empty query');	
    
    db_query("update ".TB_PREF."refs set reference='".$targetId."' where type=".ST_TARGET_ID);

    return $result;
}

function update_target_data($id,$targetName, $targetType, $targetDataType, $sqlScript, $tablename, $defaultValue,$sourceId,$fieldkey,$class)
{
    $sql = sprintf("update targetData set targetName='%s', targetType='%s', targetDataType='%s', sqlScript='%s', tablename='%s', defaultValue='%s',sourceId=%d,fieldskey='%s'
           where targetId='%s'",$targetName, $targetType, $targetDataType, $sqlScript, $tablename, $defaultValue,$sourceId,$fieldkey,$id);
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}



?>



