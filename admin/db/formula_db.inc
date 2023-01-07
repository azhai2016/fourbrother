<?php

function check_mssql_empty_result($sql) {
	$result = mssql_db_query($sql, 'could not do check empty query');		
	$myrow = mssqldb_fetch_row($result);
	return $myrow[0] >= 0;
}

function db_has_formulas(){
    return check_mssql_empty_result("SELECT COUNT(*) FROM FormulaData");
 }
 

function get_formula_data($id){
    $sql = "select * from formulaData";
    if ($id) {
        $sql .= "(FormulaId =".$id." or FormulaName like '%".$id."%') ";
    }
	$result = mssql_db_query($sql, 'could not do check empty query');		
    
	return $result;
}

   

function get_formula_id_data($id) {

	$sql = "select * from formulaData where id =".$id;

	$result = mssql_db_query($sql, 'could not do check empty query');		

	$myrow = mssqldb_fetch_assoc($result);

	return $myrow;
}


function delete_formula_data($id){
    $sql = "delete from formulaData where formulaId='".$id."'";
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}





function add_formula_data($formulaId,$formulaName,$sourceId,$formulaType,$formulaStatus,$formulaContent,$description,$isActive)
{
    $sql = sprintf("insert into formulaData (FormulaId,FormulaName,SourceId,FormulaType,FormulaStatus,FormulaContent,Description,isActive) 
           values ('%s','%s','%s','%s','%s','%s','%s',%d)",
           $formulaId,$formulaName,$sourceId,$formulaType,$formulaStatus,$formulaContent,$description,$isActive
        );
    $result = mssql_db_query($sql, 1,'could not do check empty query');	
    
    db_query("update ".TB_PREF."refs set reference='".$formulaId."' where type=".ST_FORMULA_ID);

    return $result;
}

function update_formula_data($selected_id,$formulaId,$formulaName,$sourceId,$formulaType,$formulaStatus,$formulaContent,$description,$isActive)
{
    $sql = sprintf("update FormulaData set FormulaName='%s',SourceId='%s', FormulaType='%s', FormulaStatus='%s', FormulaContent='%s', Description='%s',isActive='%s'
           where id=%d",$formulaName,$sourceId,$formulaType,$formulaStatus,$formulaContent,$description,$isActive,$selected_id);
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}


function get_formula_factor_data($id){
    $sql = "select * from FactorData where formulaId =".$id;

	$result = mssql_db_query($sql, 'could not do check empty query');		

	return $result;
}


function add_formula_factor_data($factorId, $factorName,$factorStatus, $isActive,$entity) {
    
    $sql = sprintf("insert into FactorData (factorId,factorName,formulaId,factorStatus,isActive) 
                    values ('%s','%s','%s','%s','%s')",$factorId,$factorName,$entity,$factorStatus,$isActive);
    $result = mssql_db_query($sql, 1,'could not do check empty query');	

    db_query("update ".TB_PREF."refs set reference='".$factorId."' where type=".ST_FACTOR_ID);

    return $result;

}

function update_formula_target_data($id,$factorId, $factorName, $factorStatus,$isActive,$entity, $class) {
    $sql = sprintf("update FactorData set factorName='%s',factorId='%s',formulaId=%d, factorStatus='%s',isActive=%d
           where factorid=%d",$factorName,$factorId,$entity,$factorStatus,$isActive,$selected_id);
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}

function delete_factor_id($id,$cls,$sid){
    $sql = "delete from factorData where factorId='".$id."' and formulaId='".$sid."'";
    $result = mssql_db_query($sql, 1,'could not do check empty query');		
    return $result;
}




?>