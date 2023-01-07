<?php

include_once($path_to_root.'/includes/ui/simple_crud_class.inc');
include_once($path_to_root.'/admin/db/target_db.inc');

/*
	View/Edit class for contacts
*/
class sourceTargets extends simple_crud {
	var $id;
	var $entity;
	var $sub_class;
	var $class;
	
	function __construct($name, $id, $class, $subclass=null) {
		$fields = array('targetId','targetName','targetType','targetDataType', 'sqlScript', 'tablename','defaultValue');

		parent::__construct($name, $fields);
		$this->class = $class;
		$this->subclass = $subclass;
		$this->entity = $id;
	}

	function list_view() {
		br();
        $sourceTargets = get_target_data($this->entity);
        
		start_table(TABLESTYLE, "width='50%'");

		$th = array(_('参数ID'), _('参数名称'), _('参数类型'), _('数据类型'), _('函数'), _('表名'), _('关键字段组'),_('默认值'), '', '');

		table_header($th);
		$k = 0; 

		$last = $myrow = mssqldb_fetch_assoc($sourceTargets);

		while ($last) {
					
			if (@$myrow['targetId'] != $last['targetId']) {
				alt_table_row_color($k);
				label_cell(implode('<br>', $targetId));
				label_cell($last['targetName']);
				label_cell($last['targetType']);
				label_cell($last['targetDataType']);
				label_cell($last['sqlScript']);
                label_cell($last['tablename']);
				label_cell($last['fieldskey']);
                label_cell($last['defaultValue']);
			
				edit_button_cell("{$this->name}Edit[{$last['targetId']}]", _('编辑'));

				// delete button removes only category unless this is the last contact for this person
				delete_button_cell("{$this->name}Delete[{$last['targetId']}]", _('删除'));
				end_row();
				$targetId = array();
				$last = $myrow;
			}

			if ($myrow) {
				$targetId[] = $myrow['targetId'];
				$myrow = mssqldb_fetch_assoc($sourceTargets);
			}
		}

		//inactive_control_row($th);
		end_table(1);
	}
	
	function editor_view() {		
		br();

		start_outer_table(TABLESTYLE2);

		table_section(1);
		table_section_title(_('参数设置'));
		//text_row(_('参数ID'), 'targetId', @$this->data['targetId'], 35, 40);
		//text_row(_('Last Name:'), 'name2', @$this->data['name2'], 35, 40);
		text_row(_('参数名称:'), 'targetName', @$this->data['targetName'], 35, 40);

		target_type_list_row(_('参数类型:'),'targetType',@$this->data['targetType']);
		target_data_type_list_row(_('数据类型:'),'targetDataType',@$this->data['targetDataType']);
		
		text_row(_('函数:'), 'sqlScript', @$this->data['sqlScript'], 32, 30);
		text_row(_('表名称:'), 'tablename', @$this->data['tablename'], 32, 30);
		textarea_row(_('字段组:'), 'fieldskey', @$this->data['fieldskey'], 32, 8);
		text_row(_('默认值:'), 'defaultValue', @$this->data['defaultValue'], 32, 30);
		end_outer_table(1);
	}

	function db_insert() {
		global $Refs;
		$dat = $this->data;
		$targetId= $Refs->get_next(ST_TARGET_ID,1);
		return add_target_data($targetId, $dat['targetName'], $dat['targetType'], $dat['targetDataType'], $dat['sqlScript'], $dat['tablename'], $dat['defaultValue'],$this->entity,$dat['fieldskey']);
	}

	function db_update() {
		return update_target_data($this->selected_id, $_POST['targetName'], $_POST['targetType'], $_POST['targetDataType'], $_POST['sqlScript'], $_POST['tablename'], $_POST['defaultValue'],$this->entity,$_POST['fieldskey'],$this->class);
	}



	function insert_check() {
	
		if (strlen($_POST['targetName']) == 0) {
			display_error(_('参数名称不能为空'));
			set_focus('targetName');
			return false;
		}
        if (strlen(@$_POST['targetType']) == 0) {
			display_error(_('类型不能为空'));
			set_focus('sqlScript');
			return false;
		}
        if (strlen(@$_POST['targetDataType']) == 0) {
			display_error(_('数据类型不能为空'));
			set_focus('sqlScript');
			return false;
		}
        if (strlen(@$_POST['sqlScript']) == 0) {
			display_error(_('脚本不能为空'));
			set_focus('sqlScript');
			return false;
		}
		
		return true; 
	}

	function db_read() {
		return get_target_id($this->selected_id);
	}

	function delete_check() {
		return true;
	}
	//
	//	Delete all contacts for person in current class/entity
	//
	function db_delete() {
		return delete_target_id($this->selected_id, $this->class, $this->entity);
	}
}