<?php

function items_info($flag){

  switch ($flag) {
    default:
    case BASE_INFO: item_count();break;
    case BASE_PRODUCT: item_product();break;
    case BASE_CUSTOMER: item_customer();break;
    case BASE_SUPPLY: item_supply();break;
  }
  
}

function item_count(){
  $data = get_sql_base_info();
  br();
  start_outer_table(TABLESTYLE2,'style="width:200px;"',2,12);
  table_section(1,'100%');
  table_section_title(_('备份'));

   label_row(_('商品数量:'), $data['product'][0]);
   label_row(_('供货商数量:'), $data['supply'][0]);
   label_row(_('客商数量:'), $data['customer'][0]);

  end_outer_table();

}

function item_product(){
  global $SysPrefs, $path_to_root, $page_nested,$Ajax;
  br();

  $where=''; 
if (get_post('SearchOrders')) {
 
   $Ajax->activate('item_tbl_'.BASE_PRODUCT);
}


  div_start('item_div_'.BASE_PRODUCT);
  start_table(TABLESTYLE_NOBORDER);
  start_row();
  text_cells_ex('商品名称', 'find','',null);
  text_cells_ex('生产企业', 'find_ma','',null);
  submit_cells('SearchOrders', _('查询'), '', _('查询'), '',ICON_SUBMIT);
  end_row();
  end_table();
 
  $cols = array(
    _('商品编号') => array('name' => 'goodsid', 'ord' => 'asc'),
    _('商品名称_规格_生产企业')=>array('name'=>'displayname','width'=>500),
    _('通用名称')=>array('name'=>'productname'),
    _('规格')=>array('name'=>'spec'),
    _('批准文号')=>array('name'=>'approvalnumber'),
    _('单位')=>array('name'=>'unit'),
    _('产地')=>array('name'=>'manufacturer'),
    _('有效')=>array('name'=>'active')
  );

  $showcol = 'goodsid,goodsname,productname,spec,displayname,manufacturer,plantingland,approvalnumber,unit,active,isvalid';
  $find = get_post('find');
  $find_ma = get_post('find_ma');

  if (!empty($find)||!empty($find_ma)) {
    $sqlwhere = " and (goodsid=''".$find."'' or goodsname like ''%".$find."%'')";
    $sqlwhere .= " and (manufacturer like ''%".$find_ma."%'')";
  }
  else 
  {
    $sqlwhere='';
  }
 
  $sql = sprintf("exec PagedResults @TableName='products',@Columns='%s',@where='%s'",$showcol,$sqlwhere); 

  $table = &new_mssql_db_pager('item_tbl_'.BASE_PRODUCT, $sql, $cols,null,'goodsid');

  $table->width = '80%';
  display_db_pager($table);
  div_end();

}

function item_customer(){
    global $SysPrefs, $path_to_root, $page_nested,$Ajax;
    br();

    $where=''; 
    if (get_post('SearchOrders')) {
     
       $Ajax->activate('item_tbl_'.BASE_CUSTOMER);
    }


    div_start('item_tbl_'.BASE_CUSTOMER);
    start_table(TABLESTYLE_NOBORDER);
    start_row();
    text_cells_ex('客户名称', 'find','',null);
    text_cells_ex('业务员', 'find_ma','',null);
    submit_cells('SearchOrders', _('查询'), '', _('查询'), '',ICON_SUBMIT);
    end_row();
    end_table();
    $sql = '';
    $cols = array(
      _('客户编号') => array('name' => 'customerid', 'ord' => 'asc'),
      _('客户名称') => array('name'=>'displayname'),
      _('业务员'),
      _('部门'),
      _('有效'),
      _('活跃')
    );

    $showcol = 'customerid,customername,displayname,salesman,department,active,isvalid';
    $find = get_post('find');
    $find_ma = get_post('find_ma');

    if (!empty($find)||!empty($find_ma)) {
      $sqlwhere = " and (customerid=''".$find."'' or customername like ''%".$find."%'')";
      $sqlwhere .= " and (salesman like ''%".$find_ma."%'')";
    }
    else 
    {
      $sqlwhere='';
    }
   
    $sql = sprintf("exec PagedResults @TableName='customers',@Columns='%s',@where='%s'",$showcol,$sqlwhere); 

    $table = &new_mssql_db_pager('item_tbl_'.BASE_CUSTOMER, $sql, $cols,null,'customerid');

    $table->width = '70%';
    display_db_pager($table);
    div_end();
}


function item_supply(){
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