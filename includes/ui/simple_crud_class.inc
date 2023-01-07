<?php

class simple_crud {
	var $name;
	var $Mode;
	var $selected_id;
	var $_none = ''; // selector value when no item is selected
	var $pre_handlers; // control buttons and related methods called before view display
	var $views;
	var $data = array();
	var $fields;
	var $tool_buttons;
	var $options;
	var $dec;
	var $change_formula;
	
	function __construct($name, $fields = null) {
		$this->name = $name;
		$this->pre_handlers = array(
			'Edit' => '_edit',
			'Delete' => '_delete',
			'NEW' => '_edit',
			'ADD' => '_add',
			'UPDATE' => '_update',
			'RESET' => '_cancel',
			'CLONE' => '_cloning',
			'CHANGE'=>'_change_formula',
			'SHOW'=>'_show'
			);
		$this->views = array(
			'' => 'list_view',			// default view
			'Edit' => 'editor_view',
			'Delete' => 'list_view',
			'NEW' => 'editor_view',
			'ADD' => 'editor_view',
			'UPDATE' => 'editor_view',
			'RESET' => 'list_view',
			'CLONE' => 'editor_view',
			'CHANGE'=>'editor_view',
			'SHOW'=>'show_view'
			);
 
		$this->tool_buttons['Edit'] = array(_('编辑'), _('Edit document line'), ICON_EDIT, '');
		$this->tool_buttons['Delete'] = array(_('删除'), _('Remove line from document'), ICON_DELETE, '');
		$this->tool_buttons['UPDATE'] = array(_('更新'), _('Confirm changes'), ICON_UPDATE, '');
		$this->tool_buttons['RESET'] = array(_('取消'), _('Cancel changes'), ICON_CANCEL, '');

		$this->fields = $fields;
		$this->selected_id = $this->_none;
	}
	
	//	Input/output formatters - convert values between php/user domains.
	function _format_input($value, $fmt) {
		switch($fmt) {
			case 'stock':
				$this->dec = get_qty_dec($value);
				return $value;
			case 'price':
			case 'qty':
			case 'number':
				return user_numeric($value);
			case 'percent':
				return user_numeric($value)/100;
			case 'text':
			case 'date':
			default:
				return $value;
		}
	}
	
	function _format_output($value, $fmt) {
		switch($fmt) {
			case 'price':
				return price_format($value);
			case 'qty':
				return number_format2($value, $this->dec);
			case 'number':
				return number_format2($value);
			case 'percent':
				return percent_format($value*100);
			case 'stock':
				$this->dec = get_qty_dec($value); // retrieve dec for use in later qty fields
			case 'text':
			case 'date':
			default:
				return $value;
		}
	}

	function _check_mode() {
		global $Ajax;
     	// list controls lookup

		foreach (array_keys($this->pre_handlers) as $m) {
		
			if (isset($_POST[$this->name.$m])) {
				unset($_POST['_focus']); // focus on first form entry
				$Ajax->activate($this->name.'_div');
				$val = is_array($_POST[$this->name.$m]) ? key($_POST[$this->name.$m]) : null;
			    $this->selected_id = $val!==null ? @quoted_printable_decode($val) : $this->_none;
      	        return $m;
			}

		}

 	    if (isset($_POST['CHANGE'])) {
			return 'CHANGE';
		} 



	    $mod = get_post($this->name.'Mode', '');


		if ($mod) {
			$val = @key($mod);
			$this->selected_id = $val!==null ? @quoted_printable_decode($val) : $this->_none;
    
			return $mod[$val];
		  }
	    
		return '';
	}

	//	Set record for edition
	function _edit($mode) {
		if ($this->Mode != $mode) {
			if ($this->selected_id != $this->_none)
				$this->data = $this->db_read();
			
			$this->set_posts($this->data);
		}
		$this->Mode = $mode;
	}

	//	Set record for edition
	function _show($mode) {
	
		if ($this->Mode != $mode) {
			if ($this->selected_id != $this->_none)
				$this->data = $this->db_read();
			
			$this->set_posts($this->data);
		}
		$this->Mode = $mode;
	}


	
	//	Update record after edition
	function _update($mode) {
		$this->get_posts();
		if ($this->update_check()) {
			if ($this->db_update()) {
				$this->selected_id = $this->_none;
				$this->Mode = '';
				return;
			}
		}
		$this->Mode = $mode;
	}
	
	//	Add new record
	function _add($mode) {
		$this->get_posts();
		if ($this->insert_check()) {
			$this->db_insert();
			$this->_cancel();
			return;
		}
		$this->Mode = $mode;
	}
	
	//	Delete selected  record
	function _delete() {
		if ($this->delete_check())
			$this->db_delete();
		$this->_cancel();
	}
	
	//	Return to listing view
	function _cancel() {
		$this->selected_id = $this->_none;
		$this->db_cancel();
		$this->Mode = '';
	}
	
	// Clone record for new edition
	function _cloning() {
		$this->Mode = '';
		$this->_edit('Edit');
		$this->selected_id = $this->_none;
	}

	function _change_formula($mode){

		$mod = get_post($this->name.'Mode', '');
 
		if ($mod) {
			$val = @key($mod);
			$this->selected_id = $val!==null ? @quoted_printable_decode($val) : $this->_none;
			if ($this->selected_id==0)$this->selected_id=$this->_none;
		}
	    
		$this->Mode = $mode;
	   
	}
	
	// Generate form controls
	function _bottom_controls() {
		$clone = $this->selected_id != $this->_none;

		$title=false;
		$async='both';
		$base=$this->name;

		$cancel = $async;

		if ($async === 'both') {
			$async = 'default';
			$cancel = 'cancel';
		}
		elseif ($async === 'default')
			$cancel = true;
		elseif ($async === 'cancel')
			$async = true;
		echo "<center>";
     
		if ($this->Mode == '' || $this->Mode == 'RESET') {
		  if($this->is_active) {
			echo "已审核状态，只能查看";
		  }
		  else {
			submit("{$base}NEW", _('添加'), true, $title, $async);
		  }
		}
		else {
	        if($this->is_active) {
			  echo "已审核状态，只能查看 \n";
			}
			else {
			if ($this->Mode == 'NEW' || $this->selected_id==$this->_none)
				submit("{$base}ADD", _('新增'), true, $title, $async);
			else {
			 
			 	   submit("{$base}UPDATE[{$this->selected_id}]", _('更新数据'), true, _('提交更新'), $async);
				   echo '<span style="padding:0 10px;"></span>';
			  	   submit("{$base}Delete[{$this->selected_id}]", _('删除'), true, _('提交删除'),$async,ICON_DELETE);
				   echo '<span style="padding:0 10px;"></span>';
				//if ($clone) 
				//	submit("{$base}CLONE[{$this->selected_id}]", _('复制'), true, _('复制当前数据'), $async);
			}
		   }
			submit("{$base}RESET", _("取消"), true, _('取消'), $cancel);
		  
		}
		echo "</center>";
	}

	//===========================================================================
	// Public functions
	//
	
	function tool_button($name, $selected_id=null, $params='') {
		$b = $this->tool_buttons[$name];

		return "<td align='center' $params>".button( "{$this->name}$name".($selected_id === null || $selected_id === $this->_none ? '' : "[$selected_id]"), $b[0], $b[1], $b[2], $b[3])."</td>";
	}
	
	function set_posts() {
		foreach($this->fields as $name => $fmt) {

			if (is_int($name)) {
				$name = $fmt;
				$fmt = array();
			}
			$post = isset($fmt['post']) ? $fmt['post'] : $name;
			$fld = isset($fmt['fld']) ? $fmt['fld'] : $name;

			$value = $this->selected_id == $this->_none ? (isset($fmt['dflt']) ? $fmt['dflt'] : null) : (is_array($this->data) ? $this->data[$fld]: $this->data->$fld);

			$_POST[$post] = $this->_format_output($value, isset($fmt['fmt']) ? $fmt['fmt'] : null);
		}
	}
	
	//	Get editor POST variables. 
	function get_posts() {
		foreach ($this->fields as $name => $fmt) {
		
			if (is_int($name)) {
				$name = $fmt;
				$fmt = array();
			}
			$post = isset($fmt['post']) ? $fmt['post'] : $name;
			$fld = isset($fmt['fld']) ? $fmt['fld'] : $name;

	
			$value = $this->_format_input(@$_POST[$post], @$fmt['fmt']);
			if (is_array($this->data))
				$this->data[$fld] = $value;
			else
				$this->data->$fld = $value;
		}
	}

	//	Main function - display current CRUD editor content
	function show() {
	
		if (!isset($_POST[$this->name.'Mode'])) {
			$this->set_posts();
		}

		$Mode = $this->_check_mode(true);
		

		div_start($this->name.'_div');

		if (array_key_exists($Mode, $this->pre_handlers)) {
			$fun = $this->pre_handlers[$Mode];
			$this->$fun($Mode);
		}

		if (isset($this->views[$this->Mode]))
			$this->{$this->views[$this->Mode]}();
		else
			$this->{$this->views['']}(); // default view

		$this->_bottom_controls();
		// this is needed only when we use temporary crud object together with ajax screen updates
		hidden($this->name.'Mode'.'['.$this->selected_id.']', $this->Mode);
		div_end();
	}
	
	//===========================================================================
	//	Database functions placeholders
	
	//	Read record from db for edition
	function db_read() {
		display_notification(__FUNCTION__.' 没有声明...');
		return array();
	}
	
	//	Update record in db after edition
	function db_update() {
		$this->db_insert();
	}
	
	//	Delete record
	function db_delete() {
		display_notification(__FUNCTION__.' 没有声明...');
	}
	
	//	Insert record
	function db_insert() {
		display_notification(__FUNCTION__.' 没有声明...');
	}
	
	//	Cancel edition
	//	Optional things like focus set.
	function db_cancel() {
	}


	function delete_check() {
		display_notification(__FUNCTION__.' 没有声明...');
		return true; 
	}

	function insert_check() {
		return true;
	}

	function update_check() {
		return $this->insert_check(); 
	}

	//	Show database content in pager/table
	function list_view() {
		display_notification(__FUNCTION__.' 没有声明...');
	}

	function show_view() {
		display_notification(__FUNCTION__.' 没有声明...');
	}

	//	Show record editor screen content
	function editor_view() {
		display_notification(__FUNCTION__.' 没有声明...');
	}
}
