<?php

include_once($path_to_root.'/includes/ui/simple_crud_class.inc');
include_once($path_to_root.'/admin/db/formula_db.inc');

/*
	View/Edit class for contacts
*/
class Formulas extends simple_crud {
	var $id;
	var $entity;
	var $sub_class;
	var $class;
	
	function __construct($name, $id, $class, $subclass=null) {
		$fields = array('factorId','formulaId','factorName','factorStatus','isActive');

		parent::__construct($name, $fields);
		$this->class = $class;
		$this->subclass = $subclass;
		$this->entity = $id;

	}

	function list_view() {
		br();
        $factors = get_formula_factor_data($this->entity);
        
		start_table(TABLESTYLE, "width='40%'");

		$th = array(_('ID'), _('名称'), _('说明'), _('有效'),'', '');

		table_header($th);
		$k = 0; 

		$last = $myrow = mssqldb_fetch_assoc($factors);

		while ($last) {
					
			if (@$myrow['factorId'] != $last['factorId']) {
				alt_table_row_color($k);
				label_cell(implode('<br>', $factorId));
				label_cell($last['factorName']);
			    label_cell($last['factorStatus']);
                label_cell($last['isActive']);
			
				edit_button_cell("{$this->name}Edit[{$last['factorId']}]", _('编辑'));

				// delete button removes only category unless this is the last contact for this person
				delete_button_cell("{$this->name}Delete[{$last['factorId']}]", _('删除'));
				end_row();
				$factorId = array();
				$last = $myrow;
			}

			if ($myrow) {
				$factorId[] = $myrow['factorId'];
				$myrow = mssqldb_fetch_assoc($factors);
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

		text_row(_('名称:'), 'factorName', @$this->data['factorName'], 35, 40);
		text_row(_('说明:'), 'factorStatus', @$this->data['factorStatus'], 32, 30);
		check_row(_('有效:'), 'isActive');
		end_outer_table(1);
	}

	function db_insert() {
		global $Refs;
		$dat = $this->data;
		$factorId= $Refs->get_next(ST_FACTOR_ID,1);
		return add_formula_factor_data($factorId, $dat['factorName'], $dat['factorStatus'], $dat['isActive'],$this->entity);
	}

	function db_update() {
		return update_formula_factor_data($this->selected_id,$_POST['factorId'], $_POST['factorName'], $this->entity, 
		$_POST['factorStatus'], $_POST['IsActive'],$this->entity, $this->class);
	}

	function insert_check() {
	
		if (strlen($_POST['factorName']) == 0) {
			display_error(_('参数名称不能为空'));
			set_focus('FormulaName');
			return false;
		}
		
		return true; 
	}

	function db_read() {
		return get_factor_id($this->selected_id);
	}

	function delete_check() {
		return true;
	}
	//
	//	Delete all contacts for person in current class/entity
	//
	function db_delete() {
		return delete_factor_id($this->selected_id, $this->class, $this->entity);
	}
}