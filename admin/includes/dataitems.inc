<?php

function items_info($flag){
  switch ($flag) {
    default:
    case BASE_PUR: break;
    case BASE_PUR_INV: break;
    case BASE_PUR_AC: break;
    case BASE_SALE:  break;
    case BASE_SAL_INV: break;
    case BASE_SAL_AC: break;
    case BASE_PAL: break;
    case BASE_STOCK: break;
  }
  
}

function item_pur(){
  global $SysPrefs, $path_to_root, $page_nested,$Ajax;
  br();

  $where=''; 
  if (get_post('SearchOrders')) {
   
     $Ajax->activate('item_tbl_'.BASE_SUPPLY);
  }

  div_start('item_tbl_'.BASE_SUPPLY);
  start_table(TABLESTYLE_NOBORDER);
  start_row();
  text_cells_ex('供货商名称', 'find','',null);
  text_cells_ex('业务员', 'find_ma','',null);
  submit_cells('SearchOrders', _('查询'), '', _('查询'), 'default');
  end_row();
  end_table();
  $sql = '';

  $cols = array(
    _('供货商编号') => array('name' => 'supplyid', 'ord' => 'asc'),
    _('供货商名称')=>array('name'=>'displayname'),
    _('业务员'),
    _('有效'),
  );

  $showcol = 'supplyid,displayname,buyer,active,isvalid';
  $find = get_post('find');
  $find_ma = get_post('find_ma');

  if (!empty($find)||!empty($find_ma)) {
    $sqlwhere = " and (supplyid=''".$find."'' or supplyname like ''%".$find."%'')";
    $sqlwhere .= " and (buyer like ''%".$find_ma."%'')";
  }
  else 
  {
    $sqlwhere='';
  }
 
  $sql = sprintf("exec PagedResults @TableName='supplys',@Columns='%s',@where='%s'",$showcol,$sqlwhere); 

  $table = &new_mssql_db_pager('item_tbl_'.BASE_SUPPLY, $sql, $cols,null,'supplyid');

  $table->width = '70%';
  display_db_pager($table);
  div_end();
}