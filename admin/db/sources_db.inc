<?php


function check_mssql_empty_result($sql) {
	$result = mssql_db_query($sql, 'could not do check empty query');		
	$myrow = mssqldb_fetch_row($result);

	return $myrow[0] > 0;
}

function db_has_sources(){
   return check_mssql_empty_result("SELECT COUNT(*) FROM sourceData WHERE isActive=1");
}


function get_source_id_data($id) {

	$sql = "select * from sourceData where id =".$id;

	$result = mssql_db_query($sql, 'could not do check empty query');		

	$myrow = mssqldb_fetch_assoc($result);

	return $myrow;
}

function delete_source_data($SourceId){
  	
  $sql = sprintf("select id from sourceData where SourceId='%s'",$SourceId);
  $rows = mssqldb_fetch_row(mssql_db_query($sql,0,''));
  $id= $rows[0];

  $sql = sprintf("delete from sourceData where id=%d",$id);
  $result = mssql_db_query($sql,1,'');
  
  $sql = sprintf("delete from targetData where sourceid=%d",$id);
  $result = mssql_db_query($sql,1,'');
  
  return $result;
}


function add_source_data($SourceId,$SourceName,$SourceTableName,$SourceTableMasterKey,
           $isActive,$isJudge,$isCalculation,$sqlScript)
{
	$sql = sprintf("insert into sourceData (SourceId,SourceName,SourceTableName,SourceTableMasterKey,
	isActive,isJudge,isCalculation,sqlScript) values ('%s','%s','%s','%s',%d,%d,%d,'%s')",
	$SourceId,$SourceName,$SourceTableName,$SourceTableMasterKey,
	$isActive,$isJudge,$isCalculation,$sqlScript);
	
	$result = mssql_db_query($sql,1,'could not do check empty query');	

	db_query("update ".TB_PREF."refs set reference='".$SourceId."' where type=".ST_SOURCE_ID);

    return $result;
}

function update_source_data($id,$SourceId,$SourceName,$SourceTableName,$SourceTableMasterKey,
           $isActive,$isJudge,$isCalculation,$sqlScript)
{
	$sql = sprintf("update sourceData set SourceId='%s',SourceName='%s',SourceTableName='%s',SourceTableMasterKey='%s',
	isActive=%d,isJudge=%d,isCalculation=%d,sqlScript='%s' where id=%d",
	$SourceId,$SourceName,$SourceTableName,$SourceTableMasterKey,
	$isActive,$isJudge,$isCalculation,$sqlScript,$id);
	$result = mssql_db_query($sql,1,'could not do check empty query');	
    return $result;
}




?>