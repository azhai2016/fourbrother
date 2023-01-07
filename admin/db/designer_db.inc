<?php

function add_print_content($data){

    $sql = vsprintf("REPLACE INTO ".TB_PREF."printers_json
      (`bind_id`,`name`,`description`,`json`,`sql_txt`,`sum_field`,`mod_id`,`inactive`) values (
        ".db_escape('%s').",".db_escape('%s').",".db_escape('%s').",'{}',".db_escape('%s').",".db_escape('%s').",".db_escape('%s').",".db_escape('%s')."
    )",$data);
    return db_query($sql,'The company prefferences could not be updated ');
    
}

function update_print_content($id,$json){

    $sql = "UPDATE ".TB_PREF."printers_json SET `json`=".db_escape($json)." where id=".$id;
    return db_query($sql,'The company prefferences could not be updated ');
}

function get_mod_id($id){
    $sql = "select id from ".TB_PREF."printers_json where `mod_id`=".$id;
    return db_fetch_assoc(db_query($sql,'The company prefferences could not be updated '));

} 

function get_print_template(){
    $sql = "select * from ".TB_PREF."printers_json ";
    return db_query($sql,'The company prefferences could not be updated ');
}

function get_print_template_id($id){
    $sql = "select * from ".TB_PREF."printers_json where `id`=".$id;
    return db_fetch_assoc(db_query($sql,'The company prefferences could not be updated '));
}

function get_print_template_json($id){

    $sql = "select json from ".TB_PREF."printers_json where `id`=".$id;
    $data = db_fetch_assoc(db_query($sql,'The company prefferences could not be updated '));
    $rs = implode($data);
    return $rs;
}

function get_print_template_sql($sql){
   $result = db_query($sql,'The company prefferences could not be updated ');
   return db_fetch_field($result);
}



function update_print_data($id,$data){

    $sql = vsprintf("UPDATE ".TB_PREF."printers_json 
        SET name=".db_escape('%s').
        ",description=".db_escape('%s').
        ",sql_txt=".db_escape('%s').
        ",sum_field=".db_escape('%s').
        ",mod_id=".db_escape('%s').
        ",inactive=%d WHERE id=".$id,$data);
    return db_query($sql,'The company prefferences could not be updated ');
 
}

function delete_print_data($id) {
    $sql = "DELETE FROM ".TB_PREF."printers_json WHERE id=".$id;
    return db_query($sql,'The company prefferences could not be updated ');
}



?>