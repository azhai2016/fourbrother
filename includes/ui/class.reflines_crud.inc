<?php
/**********************************************************************
	Copyright (C) FrontAccounting, LLC.
	Released under the terms of the GNU General Public License, GPL, 
	as published by the Free Software Foundation, either version 3 
	of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
	See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
***********************************************************************/
include_once $path_to_root.'/includes/db/class.reflines_db.inc';
include_once $path_to_root.'/includes/ui/class.crud_view.inc';

class fa_reflines extends simple_crud_view {

	function __construct() {
		parent::__construct('refs', new reflines_db(), array('clone' => false));

		$this->fields = array(
			'prefix',
			'description',
			'trans_type',
			'pattern',
			'default' => 'check',
		);
	}

	function list_view($Mode) {
		global $systypes_array;

		start_table(TABLESTYLE);

		$th = array(_('序号类型'), _('前缀'),
			_('样式'), _('默认'), _('说明'),  '', '');

		inactive_control_column($th);

		table_header($th);

		$k = 0;
		$data = $this->data_set->get_all(check_value('show_inactive') ? null : '!inactive', array('trans_type', 'prefix'));
     
		if (!$data) return false;  

		while ($rec = db_fetch($data)) {
			alt_table_row_color($k);

			label_cell($systypes_array[$rec['trans_type']]);
			label_cell($rec['prefix']);
			label_cell($rec['pattern']);
			label_cell($rec['default'] ? _('是') : _('否'));
			label_cell($rec['description']);

			if (check_value('show_inactive') && $rec['default'])
				label_cell('');
			else
				inactive_control_cell($rec['id'], $rec['inactive'], 'reflines', 'id');
			if ($this->options['update'])
				echo $this->tool_button('Edit', $rec['id']);
			if ($this->options['delete'])
				echo $this->tool_button('Delete', $rec['id']);
			
			end_row();
		}
		//inactive_control_row($th);
		end_table(1);
		$this->_record_controls(true);

		return true;
	}

	function editor_view($Mode) {
		global $systypes_array;

		$selected_id = $this->selected_id;

		// new or never used
		$fresh = $selected_id == $this->_none || !$this->data_set->is_used(get_post($this->name.'prefix'), get_post($this->name.'trans_type'));

		start_table(TABLESTYLE2);

		if ($fresh) {
			systypes_list_row(_('序号类型:'), $this->name.'trans_type');
			$prefix = text_input($this->name.'prefix', null, 5, 30);
		}
		else {
			label_row(_('序号类型:'), $systypes_array[get_post($this->name.'trans_type')]);
			hidden($this->name.'trans_type');
			$prefix = get_post($this->name.'prefix') . hidden($this->name.'prefix');
		}

		label_row(_('序号样式:'), $prefix . text_input($this->name.'pattern', null, 30, 60));

		if (get_post($this->name.'default'))
			{ label_row(_('默认值:'), _('是')); hidden($this->name.'default', 1); }
		else
			check_row(_('设置默认类型:'), $this->name.'default');

		text_row_ex(_('说明:'), $this->name.'description', 30, 60);

		end_table(1);
		hidden($this->name.'selected_id', $selected_id);
		$this->_record_controls();
	}

	function show($Mode=null) {

		parent::show($Mode);
	}
}