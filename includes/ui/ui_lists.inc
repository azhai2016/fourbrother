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

include_once($path_to_root.'/includes/types.inc');
include_once($path_to_root.'/includes/current_user.inc');

define('SEARCH_BUTTON', "<input %s type='submit' class='combo_submit' style='border:0;background:url($path_to_root/themes/"
	."%s/images/locate.png) no-repeat;%s' aspect='fallback' name='%s' value=' ' title='"._('Set filter')."'> ");

define('SELECT_BUTTON', "<input %s type='submit' class='combo_select' style='border:0;background:url($path_to_root/themes/"
	."%s/images/button_ok.png) no-repeat;%s' aspect='fallback' name='%s' value=' ' title='"._('Select')."'> ");

//----------------------------------------------------------------------------
//	Universal sql combo generator
//	$sql must return selector values and selector texts in columns 0 & 1
//	Options are merged with defaults.



function combo_input($name, $selected_id, $sql, $valfield, $namefield, $options=null, $type=null) {
	global $Ajax, $path_to_root, $SysPrefs, $search_types;

	$opts = array(		// default options
		'where'=> array(),		// additional constraints
		'order' => $namefield,	// list sort order
		// special option parameters
		'spec_option'=>false, 	// option text or false
		'spec_id' => 0,		// option id
		// submit on select parameters
		'default' => '', // default value when $_POST is not set
		'multi' => false,	// multiple select
		'select_submit' => false, //submit on select: true/false
		'async' => true,	// select update via ajax (true) vs _page_body reload
		// search box parameters
		'sel_hint' => null,
		'search_box' => false, 	// name or true/false
		'type' => 0,	// type of extended selector:
		// 0 - with (optional) visible search box, search by fragment inside id
		// 1 - with hidden search box, search by option text
		// 2 - with (optional) visible search box, search by fragment at the start of id
		// 3 - TODO reverse: box with hidden selector available via enter; this
		// would be convenient for optional ad hoc adding of new item
		'search_submit' => true, //search submit button: true/false
		'size' => 8,	// size and max of box tag
		'max' => 50,
		'height' => false,	// number of lines in select box
		'cells' => false,	// combo displayed as 2 <td></td> cells
		'search' => array(), // sql field names to search
		'format' => null, 	 // format functions for regular options
		'disabled' => false,
		'box_hint' => null, // box/selectors hints; null = std see below
		'category' => false, // category column name or false
		'show_inactive' => false, // show inactive records. 
		'editable' => false, // false, or length of editable entry field
		'editlink' => false // link to entity entry/edit page (optional)
	);
	// ------ merge options with defaults ----------
	if($options != null)
		$opts = array_merge($opts, $options);
	if (!is_array($opts['where']))  $opts['where'] = array($opts['where']);

	$search_box = $opts['search_box']===true ? '_'.$name.'_edit' : $opts['search_box'];
	// select content filtered by search field:
	$search_submit = $opts['search_submit']===true ? '_'.$name.'_button' : $opts['search_submit'];
	// select set by select content field
	$search_button = $opts['editable'] ? '_'.$name.'_button' : ($search_box ? $search_submit : false);
	// datalist id
	$datalist_id = $opts['search_box'] === true ? $name.'_datalist' : $opts['search_box'];

	$select_submit = $opts['select_submit'];
	$spec_id = $opts['spec_id'];
	$spec_option = $opts['spec_option'];
	if ($opts['type'] == 0) {
		$by_id = true;
		$class = 'combo';
	}
	elseif($opts['type'] == 1) {
		$by_id = false;
		$class = 'combo2';
	}
	else {
		$by_id = true;
		$class = 'combo3';
	}

	$disabled = $opts['disabled'] ? 'disabled' : '';
	$multi = $opts['multi'];
	
	if(!count($opts['search']))
		$opts['search'] = array($by_id ? $valfield : $namefield);
	
	if ($opts['sel_hint'] === null) 
		$opts['sel_hint'] = $by_id || $search_box == false ? '' : _('Press Space tab for search pattern entry');

	if ($opts['box_hint'] === null)
		$opts['box_hint'] = $search_box && $search_submit != false ? ($by_id ? _('Enter code fragment to search or * for all') : _('Enter description fragment to search or * for all')) :'';

	if ($selected_id == null)
		$selected_id = get_post($name, (string)$opts['default']);

	if(!is_array($selected_id))
		$selected_id = array((string)$selected_id); // code is generalized for multiple selection support

	$txt = get_post($search_box);
	$rel = '';
	$limit = '';

	if (isset($_POST['_'.$name.'_update'])) { // select list or search box change
		if ($by_id)
			$txt = $_POST[$name];
		if (!$opts['async'])
			$Ajax->activate('_page_body');
		else
			$Ajax->activate($name);
	}
	if (isset($_POST[$search_button])) {
		if (!$opts['async'])
			$Ajax->activate('_page_body');
		else
			$Ajax->activate($name);
	}
	if ($search_box) {
		// search related sql modifications

		$rel = "rel='$search_box'"; // set relation to list
		if ($opts['search_submit']) {
			if (isset($_POST[$search_button])) {
				$selected_id = array(); // ignore selected_id while search
				if (!$opts['async'])
					$Ajax->activate('_page_body');
				else
					$Ajax->activate($name);
			}
		}
	}

	// sql completion
	if (count($opts['where'])) {
		$where = strpos($sql, 'WHERE')==false ? ' WHERE ':' AND ';
		$where .= '('. implode(' AND ', $opts['where']) . ')';
		$group_pos = strpos($sql, 'GROUP BY');
		if ($group_pos) {
			$group = substr($sql, $group_pos);
			$sql = substr($sql, 0, $group_pos) . $where.' '.$group;
		}
		else
			$sql .= $where;
	}
	if ($opts['order'] != false) {
		if (!is_array($opts['order']))
			$opts['order'] = array($opts['order']);
		$sql .= ' ORDER BY '.implode(',',$opts['order']);
	}

	$sql .= $limit;
	// ------ make selector ----------
	$selector = '';
	$first_opt = '';
	$data_options = '';
	$first_id = false;
	$found = false;
	$lastcat = null;
	$edit = false;
	$pname = false;
	if (($type === 'customer' || $type === 'supplier') && !empty($SysPrefs->prefs['shortname_name_in_list']))
		$pname = true;

	if($result = db_query($sql)) {
		while ($contact_row = db_fetch($result)) {
			$value = $contact_row[0];
			$descr = $opts['format']==null ?  $contact_row[1] : call_user_func($opts['format'], $contact_row, $pname);
			$sel = '';
			if (get_post($search_button) && ($txt == $value))
				$selected_id[] = $value;

			if (in_array((string)$value, $selected_id, true)) {
				$sel = 'selected';
				$found = $value;
				// get non-formatted description
				$edit = $opts['editable'] && $contact_row['editable'] && (@$_POST[$search_box] == $value) ? $contact_row[1] : false;
			}

			if(!$edit) { // abandon list construction if selected field is editable.
				// show selected option even if inactive 
				if (!$opts['show_inactive'] && @$contact_row['inactive'] && $sel==='')
					continue;
				else 
					$optclass = @$contact_row['inactive'] ? "class='inactive'" : '';

				if ($first_id === false) {
					$first_id = $value;
					$first_opt = $descr;
				}
				$cat = $contact_row[$opts['category']];
				if ($opts['category'] !== false && $cat != $lastcat) {
					if ($lastcat!==null)
						$selector .= '</optgroup>';
					$selector .= "<optgroup label='".$cat."'>\n";
					$lastcat = $cat;
				}
				$selector .= "<option ".$sel.' '.$optclass." value='".$value."'>".$descr."</option>\n";
			}

			$data_options .= "<option value='".$value."'>".$descr."</option>\n";
		}
		if ($lastcat!==null)
			$selector .= '</optgroup>';
		db_free_result($result);
	}

	// Prepend special option.
	if ($spec_option !== false) { // if special option used - add it
		$first_id = $spec_id;
		$first_opt = $spec_option;
		$sel = $found===false ? 'selected' : '';
		$optclass = @$contact_row['inactive'] ? "class='inactive'" : '';
		$selector = "<option ".$sel." value='".$first_id."'>".$first_opt."</option>\n".$selector;
	}

	if ($found===false)
		$selected_id = array($first_id);
	
	$_POST[$name] = $multi ? $selected_id : $selected_id[0];

	if ($SysPrefs->use_popup_search)
		$selector = "<select id='$name' autocomplete='off' ".($multi ? 'multiple' : '')
		. ($opts['height']!==false ? ' size="'.$opts['height'].'"' : '')
		. "$disabled name='$name".($multi ? '[]':'')."' class='$class' title='"
		. $opts['sel_hint']."' $rel>".$selector."</select>\n";
	else
		$selector = "<select autocomplete='off' ".($multi ? "multiple" : '')
		. ($opts['height']!==false ? ' size="'.$opts['height'].'"' : '')
		. "$disabled name='$name".($multi ? '[]':'')."' class='$class' title='"
		. $opts['sel_hint']."' $rel>".$selector."</select>\n";
	if ($by_id && ($search_box != false || $opts['editable']) ) {
		// on first display show selector list
		if (isset($_POST[$search_box]) && $opts['editable'] && $edit) {
			$selector = "<input type='hidden' name='$name' value='".$_POST[$name]."'>"
			."<input type='text' $disabled name='{$name}_text' id='{$name}_text' size='"
			.$opts['editable']."' maxlength='".$opts['max']."' $rel value='$edit'>\n";

			set_focus($name.'_text'); // prevent lost focus
		}
		else if (get_post($search_submit ? $search_submit : "_{$name}_button"))
			set_focus($name); // prevent lost focus
		if (!$opts['editable'])
			$txt = $found;
		$datalist = "<datalist id='$datalist_id'>".$data_options."</datalist>";
		$Ajax->addUpdate($name, $search_box, $txt ? $txt : '');
	}

	$Ajax->addUpdate($name, "_{$name}_sel", $selector);

	// span for select list/input field update
	$selector = "<span id='_{$name}_sel'>".$selector."</span>\n";

	// if selectable or editable list is used - add select button
	if ($select_submit != false || $search_button) {
	// button class selects form reload/ajax selector update
		$selector .= sprintf(SELECT_BUTTON, $disabled, user_theme(),
			(fallback_mode() ? '' : 'display:none;'),
			 '_'.$name.'_update')."\n";
	}

	// ------ make combo ----------
	$edit_entry = '';
	if ($search_box != false) {
		$edit_entry = "<input $disabled type='text' list='$datalist_id' name='$search_box' id='$search_box' size='".
			$opts['size']."' maxlength='".$opts['max'].
			"' value='$txt' class='$class' rel='$name' autocomplete='off' title='"
			.$opts['box_hint']."'"
			.(!fallback_mode() && !$by_id ? " style=display:none;":'')
			.">\n";
		if ($search_submit != false || $opts['editable']) {
			$edit_entry .= sprintf(SEARCH_BUTTON, $disabled, user_theme(),
				(fallback_mode() ? '' : 'display:none;'),
				$search_submit ? $search_submit : "_{$name}_button")."\n";
		}
		if(!empty($datalist))
			$edit_entry .= $datalist;
	}
	default_focus(($search_box && $by_id) ? $search_box : $name);

	$search_btn = '';
	if ($SysPrefs->use_popup_search && (!isset($opts['fixed_asset']) || !$opts['fixed_asset'])) {
		$btn_title = '';
		$link = '';
		$id = $name;
		if ($SysPrefs->use_popup_windows && !empty($type)) {
			$type = strtolower($type);
			if($type === 'component') {
				$parent = $opts['parent'];
				$link = $path_to_root.$search_types[$type][0].$parent.$search_types[$type][1].$id;
				$btn_title = $search_types[$type][2];
			}
			elseif($type === 'branch') {
				$link = $path_to_root.$search_types[$type][0].$id.$search_types[$type][1];
				$btn_title = $search_types[$type][2];
			}
			else {
				$link = $path_to_root.$search_types[$type][0].$id;
				$btn_title = $search_types[$type][1];
			}
		}

		if ($link !== '')
			$search_btn = '<div hidden title="'.$btn_title.'" class="search_btn_container" id="_'.$name.'_search" onclick="javascript:lookupWindow(&quot;'.$link.'&quot;, &quot;&quot;);"><span class="search_btn_txt">'._('高级搜索')."</span><i class='fas fa-search'></i></div>";
	}

	if ($opts['editlink'])
		$selector .= ' '.$opts['editlink'];
	if ($search_box && $opts['cells'])
		$str = ($edit_entry != '' ? '<td>'.$edit_entry.'</td>' : '')."<td nowrap><div class='list_container'>".$selector.$search_btn."</div></td>";
	else
		$str = $edit_entry."<div class='list_container'>".$selector.$search_btn."</div>";

	return $str;
}

function combo_mssql_input($name, $selected_id, $sql, $valfield, $namefield, $options=null, $type=null) {
	global $Ajax, $path_to_root, $SysPrefs, $search_types;

	$opts = array(		// default options
		'where'=> array(),		// additional constraints
		'order' => $namefield,	// list sort order
		// special option parameters
		'custom_row_value'=>0,
		'spec_option'=>false, 	// option text or false
		'spec_id' => 0,		// option id
		// submit on select parameters
		'default' => '', // default value when $_POST is not set
		'multi' => false,	// multiple select
		'select_submit' => false, //submit on select: true/false
		'async' => true,	// select update via ajax (true) vs _page_body reload
		// search box parameters
		'sel_hint' => null,
		'search_box' => false, 	// name or true/false
		'type' => 0,	// type of extended selector:
		// 0 - with (optional) visible search box, search by fragment inside id
		// 1 - with hidden search box, search by option text
		// 2 - with (optional) visible search box, search by fragment at the start of id
		// 3 - TODO reverse: box with hidden selector available via enter; this
		// would be convenient for optional ad hoc adding of new item
		'search_submit' => true, //search submit button: true/false
		'size' => 8,	// size and max of box tag
		'max' => 50,
		'height' => false,	// number of lines in select box
		'cells' => false,	// combo displayed as 2 <td></td> cells
		'search' => array(), // sql field names to search
		'format' => null, 	 // format functions for regular options
		'disabled' => false,
		'box_hint' => null, // box/selectors hints; null = std see below
		'category' => false, // category column name or false
		'show_inactive' => false, // show inactive records. 
		'editable' => false, // false, or length of editable entry field
		'editlink' => false // link to entity entry/edit page (optional)
	);
	// ------ merge options with defaults ----------
	if($options != null)
		$opts = array_merge($opts, $options);
	if (!is_array($opts['where']))  $opts['where'] = array($opts['where']);

	$search_box = $opts['search_box']===true ? '_'.$name.'_edit' : $opts['search_box'];
	// select content filtered by search field:
	$search_submit = $opts['search_submit']===true ? '_'.$name.'_button' : $opts['search_submit'];
	// select set by select content field
	$search_button = $opts['editable'] ? '_'.$name.'_button' : ($search_box ? $search_submit : false);
	// datalist id
	$datalist_id = $opts['search_box'] === true ? $name.'_datalist' : $opts['search_box'];

	$select_submit = $opts['select_submit'];
	$spec_id = $opts['spec_id'];
	$spec_option = $opts['spec_option'];
	if ($opts['type'] == 0) {
		$by_id = true;
		$class = 'combo';
	}
	elseif($opts['type'] == 1) {
		$by_id = false;
		$class = 'combo2';
	}
	else {
		$by_id = true;
		$class = 'combo3';
	}

	$disabled = $opts['disabled'] ? 'disabled' : '';
	$multi = $opts['multi'];
	
	if(!count($opts['search']))
		$opts['search'] = array($by_id ? $valfield : $namefield);
	
	if ($opts['sel_hint'] === null) 
		$opts['sel_hint'] = $by_id || $search_box == false ? '' : _('Press Space tab for search pattern entry');

	if ($opts['box_hint'] === null)
		$opts['box_hint'] = $search_box && $search_submit != false ? ($by_id ? _('Enter code fragment to search or * for all') : _('Enter description fragment to search or * for all')) :'';

	if ($selected_id == null)
		$selected_id = get_post($name, (string)$opts['default']);

	if(!is_array($selected_id))
		$selected_id = array((string)$selected_id); // code is generalized for multiple selection support

	$txt = get_post($search_box);

	$rel = '';
	$limit = '';

	if (isset($_POST['_'.$name.'_update'])) { // select list or search box change
		if ($by_id)
			$txt = $_POST[$name];
		if (!$opts['async'])
			$Ajax->activate('_page_body');
		else
			$Ajax->activate($name);
	}
	if (isset($_POST[$search_button])) {
		if (!$opts['async'])
			$Ajax->activate('_page_body');
		else
			$Ajax->activate($name);
	}
	if ($search_box) {
		// search related sql modifications

		$rel = "rel='$search_box'"; // set relation to list
		if ($opts['search_submit']) {
			if (isset($_POST[$search_button])) {
				$selected_id = array(); // ignore selected_id while search
				if (!$opts['async'])
					$Ajax->activate('_page_body');
				else
					$Ajax->activate($name);
			}
		}
	}

	// sql completion
	if (count($opts['where'])) {
		$where = strpos($sql, 'WHERE')==false ? ' WHERE ':' AND ';
		$where .= '('. implode(' AND ', $opts['where']) . ')';
		$group_pos = strpos($sql, 'GROUP BY');
		if ($group_pos) {
			$group = substr($sql, $group_pos);
			$sql = substr($sql, 0, $group_pos) . $where.' '.$group;
		}
		else
			$sql .= $where;
	}
	if ($opts['order'] != false) {
		if (!is_array($opts['order']))
			$opts['order'] = array($opts['order']);
		$sql .= ' ORDER BY '.implode(',',$opts['order']);
	}

	$sql .= $limit;
	// ------ make selector ----------
	$selector = '';
	$first_opt = '';
	$data_options = '';
	$first_id = false;
	$found = false;
	$lastcat = null;
	$edit = false;
	$pname = false;
	if (($type === 'SourceId' || $type === 'supplier') && !empty($SysPrefs->prefs['shortname_name_in_list']))
		$pname = true;

   

	if($result = mssql_db_query($sql)) {

		while ($contact_row = mssqldb_fetch_row($result)) {

			$value =$opts['custom_row_value']>0 ? $contact_row[0].'_'.trim($contact_row[1]):$contact_row[0] ;
		
			$descr = $opts['format']==null ?  $contact_row[1] : call_user_func($opts['format'], $contact_row, $pname);
			$sel = '';
		
			if (get_post($search_button) && ($txt == $value)) {
				$selected_id[] = $value;
			}
			

			if (in_array((string)$value, $selected_id, true)) {
				$sel = 'selected';
				$found = $value;
				// get non-formatted description
				$edit = $opts['editable'] && $contact_row['editable'] && (@$_POST[$search_box] == $value) ? $contact_row[1] : false;
			}

			if(!$edit) { // abandon list construction if selected field is editable.
				// show selected option even if inactive 
				if (!$opts['show_inactive'] && @$contact_row['inactive'] && $sel==='')
					continue;
				else 
					$optclass = @$contact_row['inactive'] ? "class='inactive'" : '';

				if ($first_id === false) {
					$first_id = $value;
					$first_opt = $descr;
				}
				$cat = $contact_row[$opts['category']];
				if ($opts['category'] !== false && $cat != $lastcat) {
					if ($lastcat!==null)
						$selector .= '</optgroup>';
					$selector .= "<optgroup label='".$cat."'>\n";
					$lastcat = $cat;
				}
				$selector .= "<option ".$sel.' '.$optclass." value='".$value."'>".$descr."</option>\n";
			}

			$data_options .= "<option value='".$value."'>".$descr."</option>\n";
		}
		if ($lastcat!==null)
			$selector .= '</optgroup>';
			db_msqql_close($result);
	}

	// Prepend special option.
	if ($spec_option !== false) { // if special option used - add it
		$first_id = $spec_id;
		$first_opt = $spec_option;
		$sel = $found===false ? 'selected' : '';
		$optclass = @$contact_row['inactive'] ? "class='inactive'" : '';
		$selector = "<option ".$sel." value='".$first_id."'>".$first_opt."</option>\n".$selector;
	}

	if ($found===false)
		$selected_id = array($first_id);
	
	$_POST[$name] = $multi ? $selected_id : $selected_id[0];

	if ($SysPrefs->use_popup_search)
		$selector = "<select id='$name' autocomplete='off' ".($multi ? 'multiple' : '')
		. ($opts['height']!==false ? ' size="'.$opts['height'].'"' : '')
		. "$disabled name='$name".($multi ? '[]':'')."' class='$class' title='"
		. $opts['sel_hint']."' $rel>".$selector."</select>\n";
	else
		$selector = "<select autocomplete='off' ".($multi ? "multiple" : '')
		. ($opts['height']!==false ? ' size="'.$opts['height'].'"' : '')
		. "$disabled name='$name".($multi ? '[]':'')."' class='$class' title='"
		. $opts['sel_hint']."' $rel>".$selector."</select>\n";
	if ($by_id && ($search_box != false || $opts['editable']) ) {
		// on first display show selector list
		if (isset($_POST[$search_box]) && $opts['editable'] && $edit) {
			$selector = "<input type='hidden' name='$name' value='".$_POST[$name]."'>"
			."<input type='text' $disabled name='{$name}_text' id='{$name}_text' size='"
			.$opts['editable']."' maxlength='".$opts['max']."' $rel value='$edit'>\n";

			set_focus($name.'_text'); // prevent lost focus
		}
		else if (get_post($search_submit ? $search_submit : "_{$name}_button"))
			set_focus($name); // prevent lost focus
		if (!$opts['editable'])
			$txt = $found;
		$datalist = "<datalist id='$datalist_id'>".$data_options."</datalist>";
		$Ajax->addUpdate($name, $search_box, $txt ? $txt : '');
	}

	$Ajax->addUpdate($name, "_{$name}_sel", $selector);

	// span for select list/input field update
	$selector = "<span id='_{$name}_sel'>".$selector."</span>\n";

	// if selectable or editable list is used - add select button
	if ($select_submit != false || $search_button) {
	// button class selects form reload/ajax selector update
		$selector .= sprintf(SELECT_BUTTON, $disabled, user_theme(),
			(fallback_mode() ? '' : 'display:none;'),
			 '_'.$name.'_update')."\n";	 
	}

	// ------ make combo ----------
	$edit_entry = '';
	if ($search_box != false) {
		$edit_entry = "<input $disabled type='text' list='$datalist_id' name='$search_box' id='$search_box' size='".
			$opts['size']."' maxlength='".$opts['max'].
			"' value='$txt' class='$class' rel='$name' autocomplete='off' title='"
			.$opts['box_hint']."'"
			.(!fallback_mode() && !$by_id ? " style=display:none;":'')
			.">\n";
		if ($search_submit != false || $opts['editable']) {
			$edit_entry .= sprintf(SEARCH_BUTTON, $disabled, user_theme(),
				(fallback_mode() ? '' : 'display:none;'),
				$search_submit ? $search_submit : "_{$name}_button")."\n";
		}
		if(!empty($datalist))
			$edit_entry .= $datalist;
	}
	default_focus(($search_box && $by_id) ? $search_box : $name);

	$search_btn = '';
	if ($SysPrefs->use_popup_search && (!isset($opts['fixed_asset']) || !$opts['fixed_asset'])) {
		$btn_title = '';
		$link = '';
		$id = $name;
		if ($SysPrefs->use_popup_windows && !empty($type)) {
			$type = strtolower($type);
			if($type === 'component') {
				$parent = $opts['parent'];
				$link = $path_to_root.$search_types[$type][0].$parent.$search_types[$type][1].$id;
				$btn_title = $search_types[$type][2];
			}
			elseif($type === 'branch') {
				$link = $path_to_root.$search_types[$type][0].$id.$search_types[$type][1];
				$btn_title = $search_types[$type][2];
			}
			else {
				$link = $path_to_root.$search_types[$type][0].$id;
				$btn_title = $search_types[$type][1];
			}
		}

		if ($link !== '')
			$search_btn = '<div hidden title="'.$btn_title.'" class="search_btn_container" id="_'.$name.'_search" onclick="javascript:lookupWindow(&quot;'.$link.'&quot;, &quot;&quot;);"><span class="search_btn_txt">'._('高级搜索')."</span><i class='fas fa-search'></i></div>";
	}

	if ($opts['editlink'])
		$selector .= ' '.$opts['editlink'];
	if ($search_box && $opts['cells'])
		$str = ($edit_entry != '' ? '<td>'.$edit_entry.'</td>' : '')."<td nowrap><div class='list_container'>".$selector.$search_btn."</div></td>";
	else
		$str = $edit_entry."<div class='list_container'>".$selector.$search_btn."</div>";

	return $str;
}

function item_type_list($name, $selected_id = null, $submit_on_change = false)
{
	$result = db_query("select `category_id`,`description` from ".TB_PREF."stock_category");
    $items = array(0=>'所有');

    while ($rows = db_fetch($result))
	{
		$items[$rows['category_id']] = $rows['description'];
	}
	
    return array_selector($name, $selected_id, $items,
        array(
            'select_submit' => $submit_on_change,
            'class' => array('nosearch'),
            'async' => false)); // FIX?
}

function item_type_list_row($label, $name, $selected_id = null, $submit_on_change = false)
{
    echo "<tr><td>" . $label . "</td>\n";
    echo item_type_list_cells(null,$name, $selected_id, $submit_on_change);
    echo "</tr>\n";
}

function item_type_list_cells($label, $name, $selected_id = null, $submit_on_change = false)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo item_type_list($name, $selected_id, $submit_on_change);
    echo "</td>\n";
}

function item_units_list($name, $selected_id = null, $submit_on_change = false)
{
	$result = db_query("select `abbr`,`name` from ".TB_PREF."item_units");
    $items = array();
    while ($rows = db_fetch($result))
	{
		$items[$rows['abbr']] = $rows['name'];
	}
	
    return array_selector($name, $selected_id, $items,
        array(
            'select_submit' => $submit_on_change,
            'class' => array('nosearch'),
            'async' => false)); // FIX?
}


function item_units_list_row($label, $name, $selected_id = null, $submit_on_change = false)
{
	echo "<tr><td class='label'>" . $label . "</td>";
    echo item_units_list_cells(null,$name, $selected_id, $submit_on_change);
	echo "</tr>\n";
}



function item_units_list_cells($label, $name, $selected_id = null, $submit_on_change = false)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo item_units_list($name, $selected_id, $submit_on_change);
    echo "</td>\n";
}


function stock_items_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $opts = array(), $editkey = false, $type = 'item_list')
{
    $sql = "SELECT id,item_name as description,
	        c.description, s.inactive,s.item_spec
			FROM " . TB_PREF . "item_codes s," . TB_PREF . "stock_category c
			WHERE s.item_category=c.category_id ";

    if ($editkey) {
        set_editor('item', $name, $editkey);
    }

    $ret = combo_input($name, $selected_id, $sql, 's.id', 'item_name',
        array_merge(
            array(
                'format' => '_format_stock_items',
                'spec_option' => $all_option === true ? _('全部商品') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => true,
                'search' => array("s.stock_id", "item_name"),
                'search_submit' => false,
                'size' => 10,
                'select_submit' => $submit_on_change,
                'category' => 2,
                'order' => array('item_category', 'id'),
                'editlink' => $editkey ? add_edit_combo('item', $name) : false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type);

    return $ret;
}





function yesno_list($name, $selected_id = null, $name_yes = '', $name_no = '', $submit_on_change = false)
{
    $items = array();
    $items['0'] = strlen($name_no) ? $name_no : _('否');
    $items['1'] = strlen($name_yes) ? $name_yes : _('是');

    return array_selector($name, $selected_id, $items,
        array(
            'select_submit' => $submit_on_change,
            'class' => array('nosearch'),
            'async' => false)); // FIX?
}

function yesno_list_cells($label, $name, $selected_id = null, $name_yes = '', $name_no = '', $submit_on_change = false)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo yesno_list($name, $selected_id, $name_yes, $name_no, $submit_on_change);
    echo "</td>\n";
}

function yesno_list_row($label, $name, $selected_id = null, $name_yes = '', $name_no = '', $submit_on_change = false)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    yesno_list_cells(null, $name, $selected_id, $name_yes, $name_no, $submit_on_change);
    echo "</tr>\n";
}



/*
	Helper function.
	Returns true if selector $name is subject to update.
*/
function list_updated($name) {
	return isset($_POST['_'.$name.'_update']) || isset($_POST['_'.$name.'_button']);
}

//----------------------------------------------------------------------------------------------
//	Universal array combo generator
//	$items is array of options 'value' => 'description'
//	Options is reduced set of combo_selector options and is merged with defaults.

function array_selector($name, $selected_id, $items, $options=null) {
	global $Ajax;

	$opts = array(		// default options
		'spec_option'=>false, 	// option text or false
		'spec_id' => 0,		// option id
		'select_submit' => false, //submit on select: true/false
		'async' => true,	// select update via ajax (true) vs _page_body reload
		'default' => '', // default value when $_POST is not set
		'multi'=>false,	// multiple select
		// search box parameters
		'height' => false,	// number of lines in select box
		'sel_hint' => null,
		'disabled' => false,
		'class' => array()
	);
	// ------ merge options with defaults ----------
	if($options != null)
		$opts = array_merge($opts, $options);
	$select_submit =  $opts['select_submit'];
	$spec_id = $opts['spec_id'];
	$spec_option = $opts['spec_option'];
	$disabled = $opts['disabled'] ? "disabled" : '';
	$multi = $opts['multi'];

	if ($selected_id == null)
		$selected_id = get_post($name, $opts['default']);
	
	if(!is_array($selected_id))
		$selected_id = array((string)$selected_id); // code is generalized for multiple selection support

	if (isset($_POST[ '_'.$name.'_update'])) {
		if (!$opts['async'])
			$Ajax->activate('_page_body');
		else
			$Ajax->activate($name);
	}

	// ------ make selector ----------
	$selector = $first_opt = '';
	$first_id = false;
	$found = false;
	foreach($items as $value=>$descr) {
		$sel = '';
		if (in_array((string)$value, $selected_id, true)) {
			$sel = 'selected';
			$found = $value;
		}
		if ($first_id === false) {
			$first_id = $value;
			$first_opt = $descr;
		}
		$selector .= "<option $sel value='$value'>$descr</option>\n";
	}

	if ($first_id!==false)
		$sel = ($found===$first_id) || ($found===false && ($spec_option===false)) ? "selected='selected'" : '';
	
	// Prepend special option.
	if ($spec_option !== false) { // if special option used - add it
		$first_id = $spec_id;
		$first_opt = $spec_option;
		$sel = $found===false ? 'selected' : '';
		$selector = "<option $sel value='$spec_id'>$spec_option</option>\n".$selector;
	}

	if ($found===false)
		$selected_id = array($first_id);
	
	$_POST[$name] = $multi ? $selected_id : $selected_id[0];

	$classes = "class='combo ".implode(' ', $opts['class'])."'";

	$selector = "<select autocomplete='off' ".($multi  ? "multiple" : '')
		. ($opts['height']!==false ? ' size="'.$opts['height'].'"' : '')
		. "$disabled name='$name".($multi ? '[]' : '')."' ".$classes." title='"
		. $opts['sel_hint']."'>".$selector."</select>\n";

	$Ajax->addUpdate($name, "_{$name}_sel", $selector);

	$selector = "<span id='_{$name}_sel'>".$selector."</span>\n";

	if ($select_submit != false) { // if submit on change is used - add select button
		$selector .= sprintf(SELECT_BUTTON, $disabled, user_theme(),
			(fallback_mode() ? '' : 'display:none;'), '_'.$name.'_update')."\n";
	}
	default_focus($name);

	return "<div class='list_container'>".$selector."</div>";
}

//----------------------------------------------------------------------------------------------

function array_selector_row($label, $name, $selected_id, $items, $options=null) {
	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector($name, $selected_id, $items, $options);
	echo "</td></tr>\n";
}



//----------------------------------------------------------------------------------------------

function _format_add_curr($row, $pname=false) {
	static $company_currency;

	if ($company_currency == null)
		$company_currency = get_company_currency();
	
	if (!$pname)
		return $row[1].($row[2] == $company_currency ? '' : ("&nbsp;-&nbsp;".$row[2]));
	else
		return $row[1]."&nbsp;-&nbsp;".$row[2] . ($row[3] == $company_currency ? '' : ("&nbsp;-&nbsp;".$row[3]));
}

function add_edit_combo($type, $name=false) {
	global $path_to_root, $popup_editors, $SysPrefs;

	// Derive theme path
	$theme_path = $path_to_root.'/themes/'.user_theme();
	$key = $popup_editors[$type][1];
	$onclick = "onclick=\"javascript:callEditor($key); return false;\"";
	$img = "<i class='fas fa-edit'></i>";

	if (!isset($SysPrefs->use_icon_for_editkey) || $SysPrefs->use_icon_for_editkey == 0)
		return '<div hidden title="'._('新增/编辑').'" class="search_btn_container" id="_'.$name.'_add" '.$onclick.'><span class="search_btn_txt">'._('新增或编辑').'</span>'.$img.'</div>';

	return "<a target = '_blank' href='#' ".$onclick." tabindex='-1'>".$img."</a>";
}

function _format_print_data($rows){
	return $rows['name'];
}

function print_template_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
    $sql = "select id,name,json,description from ". TB_PREF . "printers_json";

    $ret = combo_input($name, $selected_id, $sql, 'id', 'name',
	array(
		'format' => '_format_print_data',
		'order' => array('id'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array('id','name'),
		'spec_option' => $spec_option === true ? _('所有数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => $submit_on_change,
		'async' => false,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
		'show_inactive' => $show_inactive,
		'editlink' => false,
	), 'print_data');
    return $ret;
}

function print_template_list_cells($label, $name, $selected_id = null)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }
    echo "<td>";
    echo print_template_list($name, $selected_id);
    echo "</td>\n";
}

function print_template_list_row($label, $name, $selected_id = null)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    print_template_list_cells(null, $name, $selected_id);
    echo "</tr>\n";
}





function _format_customer_datas($row,$pname = false)
{
   return $row[0] ."_" . $row[1]; 
}


function _format_select_datas($row,$pname = false)
{
   return $row[0] ."_" . trim($row[1]); 
}

function source_data_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
    $sql = "select ID,SourceId,SourceName,SourceTableName from sourceData";

    if ($editkey) {
        set_editor('item', $name, $editkey);
    } 

    $ret = combo_mssql_input($name, $selected_id, $sql, 'id', 'SourceName',
	array(
		'format' => '_format_source_datas',
		'order' => array('SourceId'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array('SourceId','SourceName', 'ID'),
		'spec_option' => $spec_option === true ? _('所有源数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => $submit_on_change,
		'async' => false,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
		'show_inactive' => $show_inactive,
		'editlink' => $editkey ? add_edit_combo('ID', $name) : false,
	), 'source_data');
    return $ret;
}


function source_data_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array())
{

    $editor_item = 'item';

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

	echo '<td nowrap>';
    echo source_data_list($name, $selected_id, $all_option, $submit_on_change, $all, $editkey);
	echo "</td>\n";
}



function source_terms_list($name, $selected_id = null)
{
    $sql = "SELECT SourceId,SourceName,id FROM sourceData";
    return combo_mssql_input($name, $selected_id, $sql, 'id', 'SourceName', array(
		'type' => 0,
	));
}

function source_terms_list_cells($label, $name, $selected_id = null)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo source_terms_list($name, $selected_id);

    echo "</td>\n";

}

function source_data_list_row($label, $name, $selected_id = null)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    source_terms_list_cells(null, $name, $selected_id);
    echo "</tr>\n";
}


function source_target_list($name, $selected_id = null)
{
    $sql = "SELECT TargetId,TargetName FROM targetData where SourceId='".$selected_id."'";

    return combo_mssql_input($name, $selected_id, $sql, 'TargetId', 'TargetName', array());
}

function source_targer_list_cells($label, $name, $selected_id = null)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo source_target_list($name, $selected_id);
	echo "<button type='button' style='margin-left:10px;margin-top:2pxl;width:60px;' value='".$selected_id."' id='Add_content'>添加</button>";
    echo "</td>\n";
}

function source_data_target_list_row($label, $name, $selected_id = null)
{
    echo "<tr><td class='label' >" . $label . "</td>";
    source_targer_list_cells(null, $name, $selected_id);

	echo "</tr>\n";
}

function _format_stock_items($row){
  return $row[2].'_'.$row[3];
}

function order_items_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $type = '', $opts = array(), $editkey = false)
{
    // all sales codes
    $sql = "SELECT i.id as item_id,i.item_code, CONCAT(i.item_name,'_',i.item_spec) as descr, i.description, count(*)>1 as kit,
			 i.inactive, if(count(*)>1, '0','1') as editable,i.item_spec,item_units,def_price,s.description as d
			FROM " . TB_PREF . "item_codes i, " . TB_PREF . "stock_category s 
			where i.item_category=s.category_id ";
			$sql .= " GROUP BY i.item_code";

    return combo_input($name, $selected_id, $sql, 'item_id', 'item_name',
        array_merge(
            array(
                'format' => '_format_stock_items',
                'spec_option' => $all_option === true ? _('全部商品') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => true,
                'search' => array("item_id", "item_name"),
                'search_submit' => false,
                'size' => 10,
                'select_submit' => $submit_on_change,
                'category' => 10,
                'order' => array('item_name', 'item_code'),
                'editlink' => false,// $editkey ? add_edit_combo('item', $name) : false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type == 'kits' ? $type : "products_list");
}

function order_items_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $editkey = false)
{
    if ($editkey) {
        set_editor('item', $name, $editkey);
    }

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo order_items_list($name, $selected_id, $all_option, $submit_on_change,
        'order', array('cells' => true, 'max' => 50), $editkey); // maximum is set to 50 and not default 255.

}

function select_items_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $editkey = false)
{
    if ($editkey) {
        set_editor('item', $name, $editkey);
    }

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo order_items_list($name, $selected_id, $all_option, $submit_on_change,
        'order', array('cells' => true, 'max' => 50), $editkey);

}

function stock_manufactured_items_list($name, $selected_id=null, $all_option=false, $submit_on_change=false) {
	return order_items_list($name, $selected_id, $all_option, $submit_on_change,'order');
}

function stock_manufactured_items_list_cells($label, $name, $selected_id=null, $all_option=false, $submit_on_change=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo stock_manufactured_items_list($name, $selected_id, $all_option, $submit_on_change);
	echo "</td>\n";
}

function stock_manufactured_items_list_row($label, $name, $selected_id=null, $all_option=false, $submit_on_change=false) {
	echo "<tr><td class='label'>".$label."</td>";
	stock_manufactured_items_list_cells(null, $name, $selected_id, $all_option, $submit_on_change);
	echo "</tr>\n";
}




//------------------------------------------------------------------------------------------------

function wo_types_list($name, $selected_id=null) {
	global $wo_types_array;
	
	return array_selector($name, $selected_id, $wo_types_array, 
		array( 'select_submit'=> true, 'async' => true ) );
}

function wo_types_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td><td>\n";
	echo wo_types_list($name, $selected_id);
	echo "</td></tr>\n";
}


function _format_items_categoty($rows){
	return $rows[0].'_'.$rows[1];
}

function items_category_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $type = '', $opts = array(), $editkey = false)
{

    $sql = "SELECT i.item_code,i.item_name,i.item_spec,item_units,c.description
	        FROM " . TB_PREF . "item_codes i left join 
			 ". TB_PREF . "stock_category c on i.item_category=c.category_id ";
			
    return combo_input($name, $selected_id, $sql, 'i.item_code', 'item_name',
        array_merge(
            array(
                'format' => '_format_items_categoty',
                'spec_option' => $all_option === true ? _('全部商品') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => false,
                'search' => array("item_code", "item_name"),
                'search_submit' => false,
                'size' => 15,
                'select_submit' => $submit_on_change,
                'category' => 'description',
                'order' => array('item_code'),
                'editlink' => false,// $editkey ? add_edit_combo('item', $name) : false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type == 'kits' ? $type : "products_list");
}

function select_items_list($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $editkey = false)
{
    echo items_category_list($name, $selected_id, $all_option, $submit_on_change,
        'order', array('cells' => true, 'max' => 50), $editkey);
}




function item_tax_types_list($name, $selected_id = null, $show_inactive = false)
{
    $sql = "SELECT id, name FROM " . TB_PREF . "item_tax_types";
    if (!$show_inactive) {
        $sql .= " WHERE !inactive";
    }

    return combo_input($name, $selected_id, $sql, 'id', 'name', array('order' => 'id'));
}

function item_tax_types_list_cells($label, $name, $selected_id = null, $show_inactive = false)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo item_tax_types_list($name, $selected_id, $show_inactive);
    echo "</td>\n";
}

function item_tax_types_list_row($label, $name, $selected_id = null, $show_inactive = false)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    item_tax_types_list_cells(null, $name, $selected_id, $show_inactive);
    echo "</tr>\n";
}


function locations_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $fixed_asset = false)
{

    $sql = "SELECT loc_code, location_name, inactive FROM " . TB_PREF . "locations WHERE fixed_asset = " . (int) $fixed_asset;

    return combo_input($name, $selected_id, $sql, 'loc_code', 'location_name',
        array(
            'spec_option' => $all_option === true ? _('所有库区') : $all_option,
            'spec_id' => ALL_TEXT,
            'select_submit' => $submit_on_change,
        ));
}

function locations_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $fixed_asset = false)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo locations_list($name, $selected_id, $all_option, $submit_on_change, $fixed_asset);
    echo "</td>\n";
}

function locations_list_row($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $fixed_asset = false)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    locations_list_cells(null, $name, $selected_id, $all_option, $submit_on_change, $fixed_asset);
    echo "</tr>\n";
}

//-----------------------------------------------------------------------------------------------

function workcenter_list($name, $selected_id = null, $all_option = false)
{

    $sql = "SELECT id, name, inactive FROM " . TB_PREF . "workcentres";

    return combo_input($name, $selected_id, $sql, 'id', 'name',
        array(
            'spec_option' => $all_option === true ? _('All Suppliers') : $all_option,
            'spec_id' => ALL_TEXT,
        ));
}

function workcenter_list_cells($label, $name, $selected_id = null, $all_option = false)
{
    default_focus($name);
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo workcenter_list($name, $selected_id, $all_option);
    echo "</td>\n";
}

function workcenter_list_row($label, $name, $selected_id = null, $all_option = false)
{
    echo "<tr><td class='label'>" . $label . "</td>";
    workcenter_list_cells(null, $name, $selected_id, $all_option);
    echo "</tr>\n";
}

function stock_item_types_list_row($label, $name, $selected_id = null, $enabled = true)
{
    global $stock_types;

    echo "<tr>";
    if ($label != null) {
        echo "<td class='label'>" . $label . "</td>\n";
    }

    echo "<td>";

    echo array_selector($name, $selected_id, $stock_types,
        array(
            'select_submit' => true,
            'disabled' => !$enabled));
    echo "</td></tr>\n";
}

function stock_units_list_row($label, $name, $value = null, $enabled = true)
{
    $result = get_all_item_units();
    echo "<tr>";
    if ($label != null) {
        echo "<td class='label'>" . $label . "</td>\n";
    }

    echo "<td>";

    while ($unit = db_fetch($result)) {
        $units[$unit['abbr']] = $unit['name'];
    }

    echo array_selector($name, $value, $units, array('disabled' => !$enabled));

    echo "</td></tr>\n";
}


//---------------------------------------------


function select_data_list($name,$tbname,$fields,$morelink,$selected_id = null,$flag=0)
{
	  $_field_str = $fields;
	  $fields = explode(',',$fields);
	  $sql = "select top 10 $_field_str from $tbname"; 
      
  	  return combo_mssql_input($name, $selected_id, $sql, $fields[0], $fields[1], array(
        'format' => '_format_select_datas',
		'custom_row_value'=>1,
		'type' => 1,
		'size' => 20,
		'search' => array(''),
		'search_box' => true,
		'spec_id' => ALL_TEXT,

		'multi'=>false,
		'select_submit' => true,
	  
	  ),$morelink);
	
}

function select_data_list_cells($label, $name,$tbname,$fields,$morelink,$selected_id = null,$tgname=null,$id=null,$flag=0)
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

    echo "<td>";
    echo select_data_list($name,$tbname,$fields,$morelink,$selected_id);
	
	echo '<input type="hidden" name="'.$name.'_tgname" value="'.$tgname.'" />';
	echo '<input type="hidden" name="tbname[]" value="'.$name.'" />';
	echo '<input type="hidden" name="'.$name.'_conditions_id" value="'.$id.'" />';
	echo '<span style="padding-left:10px;">'.button($name.'_btn', $tbname, '选择', ICON_ANGLE_DOUBLE_RIGHT, '').'</span>';

	//echo '<span style="padding-left:10px;">'.submit($name.'_btn','添加',$selected_id,"", '选择', true,false).'</span>';
	echo "</td>\n";
	
}

function select_data_list_row($label,$name,$tbname,$fields,$morelink,$selected_id = null,$tgname=null,$id=null,$flag=0)
{
    echo "<tr><td class='label' >" . $label . "</td>";
    select_data_list_cells(null,$name,$tbname,$fields,$morelink,$selected_id,$tgname,$id,$flag);
	echo "</tr>\n";
}


function select_data_list_row2($label,$name,$tbname,$fields,$morelink,$selected_id = null,$tgname=null,$id=null,$flag=0)
{
    echo "<tr><td class='label' >" . $label . "</td>";
    echo "<td>".select_data_list($name,$tbname,$fields,$morelink,$selected_id)."</td>";
	echo "</tr>\n";
}







//---------------------------------------------

function formula_data_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
    $sql = "select Id,FormulaId,FormulaName,FormulaType from FormulaData";

    if ($editkey) {
        set_editor('formula_all_data', $name, $editkey);
    } 

    $ret = combo_mssql_input($name, $selected_id, $sql, 'Id', 'FormulsName',
	array(
		'format' => '_format_source_datas',
		'order' => array('FormulaId'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array(''),
		'spec_option' => $spec_option === true ? _('所有源数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => $submit_on_change,
		'async' => false,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
		'show_inactive' => $show_inactive,
		'editlink' => $editkey ? add_edit_combo('Id', $name) : false,
	), 'formula_all_data');
    return $ret;
}


function formula_data_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array())
{

    $editor_item = 'item';

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

	echo '<td nowrap>';
    echo formula_data_list($name, $selected_id, $all_option, $submit_on_change, $all, $editkey);
	echo "</td>\n";
}

//------------------------------------------------------------------------------------

function _format_template_items($row) {
	return ($row[0].'&nbsp;- &nbsp;'._('Amount').'&nbsp;'.$row[1]);
}

function templates_list($name, $selected_id=null, $special_option=false) {
	$sql = "SELECT sorder.order_no,	Sum(line.unit_price*line.quantity*(1-line.discount_percent)) AS OrderValue
		FROM ".TB_PREF."sales_orders as sorder, ".TB_PREF."sales_order_details as line
		WHERE sorder.order_no = line.order_no AND sorder.type = 1 GROUP BY line.order_no";
	return combo_input($name, $selected_id, $sql, 'order_no', 'OrderValue', array(
		'format' => '_format_template_items',
		'spec_option' => $special_option===true ? ' ' : $special_option,
		'order' => 'order_no', 'spec_id' => 0,
	));
}

function templates_list_cells($label, $name, $selected_id=null, $special_option=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo templates_list($name, $selected_id, $special_option);
	echo "</td>\n";
}

function templates_list_row($label, $name, $selected_id=null, $special_option=false) {
	echo "<tr><td class='label'>".$label."</td>";
	templates_list_cells(null, $name, $selected_id, $special_option);
	echo "</tr>\n";
}

//------------------------------------------------------------------------------------

function workorders_list($name, $selected_id=null) {
	$sql = "SELECT id, wo_ref FROM ".TB_PREF."workorders WHERE closed=0";
	return combo_input($name, $selected_id, $sql, 'id', 'wo_ref', array());
}

function workorders_list_cells($label, $name, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo workorders_list($name, $selected_id);
	echo "</td>\n";
}

function workorders_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td>";
	workorders_list_cells(null, $name, $selected_id);
	echo "</tr>\n";
}

//------------------------------------------------------------------------------------
function _format_target_datas($rows){
	return $rows[0];
}

function target_type_list($name, $selected_id=null) {
	$sql = "SELECT value,id FROM Settings where flag=".ST_TARGETTYPE;
	return combo_mssql_input($name, $selected_id, $sql, 'id', 'value', array(
		'format' => '_format_target_datas',
	));
}

function target_type_list_cells($label, $name, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo target_type_list($name, $selected_id);
	echo "</td>\n";
}

function target_type_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td>";
	target_type_list_cells(null, $name, $selected_id);
	echo "</tr>\n";
}


//----------------------- 客商选择  -----------------------
function customer_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false) {
	$sql = "SELECT id,name FROM  ".TB_PREF."customers ";

    if ($editkey) {
        set_editor('item', $name, $editkey);
    } 

    $ret = combo_input($name, $selected_id, $sql, 'id', 'name',
	array(
		'format' => '_format_customer_datas',
		'order' => array('Id'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array('id','name'),
		'spec_option' => $spec_option === true ? _('所有数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => $submit_on_change,
		'async' => false,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
		'show_inactive' => $show_inactive,
		'editlink' =>  false,
	), 'customer_list');
    return $ret;
}

function customer_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array()) {
	$editor_item = 'item';

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

	echo '<td nowrap>';
    echo customer_list($name, $selected_id, $all_option, $submit_on_change, $all, $editkey);
	echo "</td>\n";
}




function customer_list_row($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
    echo "<tr><td class='label'>" . $label . "</td><td nowrap>";
    echo customer_list($name, $selected_id, $all_option, $submit_on_change, $show_inactive, $editkey);
    echo "</td>\n</tr>\n";
}

function customer_type_row($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
    echo "<tr><td class='label'>" . $label . "</td><td nowrap>";
    echo customer_list($name, $selected_id, $all_option, $submit_on_change, $show_inactive, $editkey);
    echo "</td>\n</tr>\n";
}


function customer_data($id)
{
	if (isset($id)) {
	  $sql = "SELECT id,name,address FROM ".TB_PREF."customers WHERE id=$id";
	  $result = db_fetch(db_query($sql,''));
      return $result;
	}
}

function _format_items($row)
{
    return (user_show_codes() ? ($row[0] . "&nbsp;-&nbsp;") : '') . $row[1];
}


function items_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $opts = array(), $editkey = false, $type = 'products_list')
{
    $sql = "SELECT id, concat(item_code,'_',item_name,'_',item_spec,'_',item_factory) as description
	        FROM  " . TB_PREF . "item_codes
			WHERE 1=1 ";

    $ret = combo_input($name, $selected_id, $sql, 'id', 'description',
        array_merge(
            array(
                'format' => '_format_items',
                'spec_option' => $all_option === true ? _('全部商品') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => false,
                'search' => array("id", "description"),
                'search_submit' => false,
                'size' => 10,
                'select_submit' => $submit_on_change,
                'order' => array('description', 'id'),
                'editlink' => $editkey ? add_edit_combo('item', $name) : false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type);

    return $ret;
}

function items_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array())
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }
    echo '<td>'.items_list($name, $selected_id, $all_option, $submit_on_change,$opts, $editkey).'</td>';
}

function leads_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $opts = array(), $editkey = false, $type = 'leads_list')
{
    $sql = "SELECT *  FROM  " . TB_PREF . "leads
			WHERE 1=1 and pid=0 ";

    $ret = combo_input($name, $selected_id, $sql, 'id', 'name',
        array_merge(
            array(
                'format' => '_format_leads',
                'spec_option' => $all_option === true ? _('全部联系人') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => false,
                'search' => array("id", "name"),
                'search_submit' => false,
                'size' => 10,
                'select_submit' => $submit_on_change,
                'order' => array('name', 'id'),
                'editlink' => false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type);

    return $ret;
}


function leads_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array())
{
    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }
    echo '<td>'.leads_list($name, $selected_id, $all_option, $submit_on_change,$opts, $editkey).'</td>';
}


function leads_work_list($name, $selected_id = null, $all_option = false, $submit_on_change = false, $opts = array(), $editkey = false, $type = 'leads_list')
{
    $sql = "SELECT *  FROM  " . TB_PREF . "contacts
			WHERE 1=1 ";

    $ret = combo_input($name, $selected_id, $sql, 'id', 'name',
        array_merge(
            array(
                'format' => '_format_work_leads',
                'spec_option' => $all_option === true ? _('全部联系人') : $all_option,
                'spec_id' => ALL_TEXT,
                'search_box' => false,
                'search' => array("id", "name"),
                'search_submit' => false,
                'size' => 10,
                'select_submit' => $submit_on_change,
                'order' => array('name', 'id'),
                'editlink' => false,
                'editable' => false,
                'max' => 255,
            ), $opts), $type);
    return $ret;
}


function leads_work_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array()){
	if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }
    echo '<td>'.leads_work_list($name, $selected_id, $all_option, $submit_on_change,$opts, $editkey).'</td>';

}

//
function supply_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false) {
	$sql = "SELECT top 10 supplyid,supplyname,buyer FROM  supplys ";
    $ret = combo_mssql_input($name, $selected_id, $sql, 'supplyid', 'supplyname',
	array(
		'format' => '_format_select_datas',
		'order' => array('supplyid'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array('supplyid','supplyname'),
		'spec_option' => $spec_option === true ? _('所有数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => true,
		'async' => true,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
	), 'source_data');
    return $ret;
}

function supply_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array()) {
	$editor_item = 'item';

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

	echo '<td nowrap>';
    echo supply_list($name, $selected_id, $all_option, $submit_on_change, $all, $editkey);
	echo "</td>\n";
}


//---------------  计算类型  ------------------------------
function formula_type_list($name, $selected_id=null) {
	$sql = "SELECT value,id FROM Settings where flag=".ST_FORMULATYPE;
	return combo_mssql_input($name, $selected_id, $sql, 'id', 'value', array(
		'format' => '_format_target_datas',
	));
}


function formula_type_list_cells($label, $name, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td style='padding:5px 0 5px'>";
	echo formula_type_list($name, $selected_id);
	echo "</td>\n";
}

function formula_type_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td>";
	formula_type_list_cells(null, $name, $selected_id);
	echo "</tr>\n";
}


//--------------  采购规则 ----------------------------
function purchase_rule_list($name, $selected_id = null, $spec_option = false, $submit_on_change = false, $show_inactive = false, $editkey = false)
{
	$sql = "SELECT id,ruleid,Name FROM  purchaseRules ";

    if ($editkey) {
        set_editor('item', $name, $editkey);
    } 

    $ret = combo_mssql_input($name, $selected_id, $sql, 'id', 'Name',
	array(
		'format' => '_format_source_datas',
		'order' => array('Id'),
		'search_box' => true,
		'type' => 1,
		'size' => 20,
		'search' => array('ruleid','Name', 'ID'),
		'spec_option' => $spec_option === true ? _('所有数据') : $spec_option,
		'spec_id' => ALL_TEXT,
		'select_submit' => $submit_on_change,
		'async' => false,
		'sel_hint' =>('按空格键按名称片段过滤；F2-进入新客商'),
		'show_inactive' => $show_inactive,
		'editlink' => $editkey ? add_edit_combo('ID', $name) : false,
	), 'source_data');
    return $ret;
}




function purchase_rule_list_cells($label, $name, $selected_id = null, $all_option = false, $submit_on_change = false, $all = false, $editkey = false, $opts = array())
{

    $editor_item = 'item';

    if ($label != null) {
        echo "<td>" . $label . "</td>\n";
    }

	echo '<td nowrap>';
    echo purchase_rule_list($name, $selected_id, $all_option, $submit_on_change, $all, $editkey);
	echo "</td>\n";
}


//----------------------------------------------------------------------------
function target_data_type_list($name, $selected_id=null) {
	$sql = "SELECT value,id FROM Settings where flag=".ST_TARGETDATATYPE;
	return combo_mssql_input($name, $selected_id, $sql, 'value', 'id', array(
		'format' => '_format_target_datas',
	));
}

function target_data_type_list_cells($label, $name, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo target_data_type_list($name, $selected_id);
	echo "</td>\n";
}

function target_data_type_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td>";
	target_data_type_list_cells(null, $name, $selected_id);
	echo "</tr>\n";
}


//------------------------------------------------------------------------------------

function credit_status_list($name, $selected_id=null) {
	$sql ="SELECT id, reason_description, inactive FROM ".TB_PREF."credit_status";
	return combo_input($name, $selected_id, $sql, 'id', 'reason_description', array());
}

function credit_status_list_cells($label, $name, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo credit_status_list($name, $selected_id);
	echo "</td>\n";
}

function credit_status_list_row($label, $name, $selected_id=null) {
	echo "<tr><td class='label'>".$label."</td>";
	credit_status_list_cells(null, $name, $selected_id);
	echo "</tr>\n";
}



//-----------------------------------------------------------------------------------------------

function class_list($name, $selected_id=null, $submit_on_change=false) {
	$sql = "SELECT cid, class_name FROM ".TB_PREF."chart_class";

	return combo_input($name, $selected_id, $sql, 'cid', 'class_name',
		array(
			'select_submit'=> $submit_on_change,
			'async' => false
		) );

}

function class_list_cells($label, $name, $selected_id=null, $submit_on_change=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo class_list($name, $selected_id, $submit_on_change);
	echo "</td>\n";
}

function class_list_row($label, $name, $selected_id=null, $submit_on_change=false) {
	echo "<tr><td class='label'>".$label."</td>";
	class_list_cells(null, $name, $selected_id, $submit_on_change);
	echo "</tr>\n";
}



function area_list($name, $selected_id=null, $all_option=false) {
	$sql = "SELECT area_code, description FROM ".TB_PREF."areas";	
		$result = db_query($sql, 'cannot get all areas');
		$items = array();
		while($myrow=db_fetch($result)) {
			$items[$myrow['area_code'].'_'.$myrow['description']] = $myrow['description'];
		}

	if($all_option)
		$items[''] = $all_option;
	return array_selector($name, $selected_id, $items);
}



function area_list_cells($label, $name, $selected_id=null, $all_option=false) {
	if($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo area_list($name, $selected_id, $all_option);
	echo "</td>\n";
}

function area_list_row($label, $name, $selected_id=null, $all_option=false) {
	echo "<tr><td class='label'>".$label."</td>";
	area_list_cells(null, $name, $selected_id, $all_option);
	echo "</tr>\n";
}

//------------------------------------------------------------------------------------------------
function sale_type_list($name, $selected_id=null, $all_option=false) {
	$sql = "SELECT id,sales_type FROM ".TB_PREF."sales_types";	
		$result = db_query($sql, 'cannot get all areas');
		$items = array();
		while($myrow=db_fetch($result)) {
			$items[$myrow['id'].'_'.$myrow['sales_type']] = $myrow['sales_type'];
		}

	if($all_option)
		$items[' '] = '' ;

	return array_selector($name, $selected_id, $items);
}

function sale_type_list_cells($label, $name, $selected_id=null, $all_option=false) {
	if($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo sale_type_list($name, $selected_id, $all_option);
	echo "</td>\n";
}

function sale_type_list_row($label, $name, $selected_id=null, $all_option=false) {
	echo "<tr><td class='label'>".$label."</td>";
	sale_type_list_cells(null, $name, $selected_id, $all_option);
	echo "</tr>\n";
}


//------------------------------------------------------------------------------------------------

function languages_list($name, $selected_id=null, $all_option=false) {
	global $installed_languages;

	$items = array();
	if($all_option)
		$items[''] = $all_option;
	foreach($installed_languages as $lang)
		$items[$lang['code']] = $lang['name'];

	return array_selector($name, $selected_id, $items);
}

function languages_list_cells($label, $name, $selected_id=null, $all_option=false) {
	if($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo languages_list($name, $selected_id, $all_option);
	echo "</td>\n";
}

function languages_list_row($label, $name, $selected_id=null, $all_option=false) {
	echo "<tr><td class='label'>".$label."</td>";
	languages_list_cells(null, $name, $selected_id, $all_option);
	echo "</tr>\n";
}

//------------------------------------------------------------------------------------------------

function dateformats_list_row($label, $name, $value=null) {
	global $SysPrefs;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector($name, $value, $SysPrefs->dateformats);
	echo "</td></tr>\n";
}

function dateseps_list_row($label, $name, $value=null) {
	global $SysPrefs;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector($name, $value, $SysPrefs->dateseps);
	echo "</td></tr>\n";
}

function thoseps_list_row($label, $name, $value=null) {
	global $SysPrefs;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $SysPrefs->thoseps );
	echo "</td></tr>\n";
}

function decseps_list_row($label, $name, $value=null) {
	global $SysPrefs;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $SysPrefs->decseps );
	echo "</td></tr>\n";
}

function print_mod_list($label, $name, $value=null) {
	global $print_actions;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $print_actions);
	echo "</td></tr>\n";
}


function themes_list_row($label, $name, $value=null) {
	global $path_to_root;

	$path = $path_to_root.'/themes/';
	$themes = array();
	$themedir = opendir($path);
	while(false !== ($fname = readdir($themedir))) {
		if($fname!='.' && $fname!='..' && $fname!='CVS' && is_dir($path.$fname))
			$themes[$fname] =  $fname;
	}
	ksort($themes);

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $themes );
	echo "</td></tr>\n";
}

function pagesizes_list_row($label, $name, $value=null) {
	global $SysPrefs;

	$items = array();
	foreach ($SysPrefs->pagesizes as $pz)
		$items[$pz] = $pz;

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $items );
	echo "</td></tr>\n";
}

function systypes_list($name, $value=null, $spec_opt=false, $submit_on_change=false, $exclude=array()) {
	global $systypes_array;

	// emove non-voidable transactions if needed
	$systypes = array_diff_key($systypes_array, array_flip($exclude));
	return array_selector($name, $value, $systypes, 
		array( 
			'spec_option'=> $spec_opt,
			'spec_id' => ALL_NUMERIC,
			'select_submit'=> $submit_on_change,
			'async' => false,
			)
	);
}

function systypes_list_cells($label, $name, $value=null, $submit_on_change=false, $exclude=array()) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo systypes_list($name, $value, false, $submit_on_change, $exclude);
	echo "</td>\n";
}

function systypes_list_row($label, $name, $value=null, $submit_on_change=false, $exclude=array()) {
	echo "<tr><td class='label'>".$label."</td>";
	systypes_list_cells(null, $name, $value, $submit_on_change, $exclude);
	echo "</tr>\n";
}

function journal_types_list_cells($label, $name, $value=null, $submit_on_change=false) {
	global $systypes_array;

	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";

	$items = $systypes_array;

	// exclude quotes, orders and dimensions
	foreach (array(ST_PURCHORDER, ST_SALESORDER, ST_DIMENSION, ST_SALESQUOTE, ST_LOCTRANSFER) as $excl)
		unset($items[$excl]);
	
	echo array_selector($name, $value, $items, 
		array( 
			'spec_option'=> _('All'),
			'spec_id' => -1,
			'select_submit'=> $submit_on_change,
			'async' => false
			)
	);
	echo "</td>\n";
}

function cust_allocations_list_cells($label, $name, $selected=null, $submit_on_change=false) {

	if ($label != null)
		label_cell($label);
	echo "<td>\n";
	$allocs = array(
		ALL_TEXT=>_('All Types'),
		'1'=> _('Sales Invoices'),
		'2'=> _('Unsettled transactions'),
		'3' => _('Payments'),
		'4' => _('Credit Notes'),
		'5' => _('Delivery Notes')
	);
	echo array_selector($name, $selected, $allocs, array('select_submit'=> $submit_on_change));
	echo "</td>\n";
}


function number_list($name, $selected, $from, $to, $no_option=false, $submit_on_change=false, $adclass=array()) {
	$items = array();
	for ($i = $from; $i <= $to; $i++)
		$items[$i] = $i;

	if(!is_array($adclass))
		$adclass = array($adclass);

	return array_selector($name, $selected, $items,
		array(	'spec_option' => $no_option,
				'select_submit' => $submit_on_change,
				'spec_id' => ALL_NUMERIC,
				'class' => $adclass)
		);
}

function number_list_cells($label, $name, $selected, $from, $to, $no_option=false, $submit_on_change=false, $adclass=array()) {
	if ($label != null)
		label_cell($label);
	echo "<td>\n";
	echo number_list($name, $selected, $from, $to, $no_option, $submit_on_change, $adclass);
	echo "</td>\n";
}

function number_list_row($label, $name, $selected, $from, $to, $no_option=false, $submit_on_change=false, $adclass=array()) {
	echo "<tr><td class='label'>".$label."</td>";
	echo number_list_cells(null, $name, $selected, $from, $to, $no_option, $submit_on_change, $adclass);
	echo "</tr>\n";
}

function print_profiles_list_row($label, $name, $selected_id=null, $spec_opt=false, $submit_on_change=true) {
	$sql = "SELECT profile FROM ".TB_PREF."print_profiles GROUP BY profile";
	$result = db_query($sql, 'cannot get all profile names');
	$profiles = array();
	while($myrow=db_fetch($result)) {
		$profiles[$myrow['profile']] = $myrow['profile'];
	}

	echo "<tr>";
	if ($label != null)
		echo "<td class='label'>".$label."</td>\n";
	echo "<td>";

	echo array_selector($name, $selected_id, $profiles, 
		array( 'select_submit'=> $submit_on_change,
			'spec_option'=>$spec_opt,
			'spec_id' => ''
		 ));

	echo "</td></tr>\n";
}

function printers_list($name, $selected_id=null, $spec_opt=false, $submit_on_change=false) {
	static $printers; // query only once for page display

	if (!$printers) {
		$sql = "SELECT id, name, description FROM ".TB_PREF."printers";	
		$result = db_query($sql, 'cannot get all printers');
		$printers = array();
		while($myrow=db_fetch($result)) {
			$printers[$myrow['id']] = $myrow['name'].'&nbsp;-&nbsp;'.$myrow['description'];
		}
	}
	return array_selector($name, $selected_id, $printers, 
		array( 'select_submit'=> $submit_on_change,
			'spec_option'=>$spec_opt,
			'spec_id' => ''
		 ));
}

//------------------------------------------------------------------------------------------------

function quick_entries_list($name, $selected_id=null, $type=null, $submit_on_change=false) {
	$where = false;
	$sql = "SELECT id, description FROM ".TB_PREF."quick_entries";
	if ($type != null)
		$sql .= " WHERE type=$type";

	return combo_input($name, $selected_id, $sql, 'id', 'description',
		array(
			'spec_id' => '',
			'order' => 'description',
			'select_submit'=> $submit_on_change,
			'async' => false
		) );

}

function quick_entries_list_cells($label, $name, $selected_id, $type, $submit_on_change=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";	
	echo quick_entries_list($name, $selected_id, $type, $submit_on_change);
	echo "</td>";
}

function quick_entries_list_row($label, $name, $selected_id, $type, $submit_on_change=false) {
	echo "<tr><td class='label'>".$label."</td>";
	quick_entries_list_cells(null, $name, $selected_id, $type, $submit_on_change);
	echo "</tr>\n";
}


function quick_actions_list_row($label, $name, $selected_id=null, $submit_on_change=false) {
	global $quick_actions;
	
	echo "<tr><td class='label'>".$label."</td><td>";
	echo array_selector($name, $selected_id, $quick_actions, 
		array( 
			'select_submit'=> $submit_on_change
		) );
	echo "</td></tr>\n";
}

function quick_entry_types_list_row($label, $name, $selected_id=null, $submit_on_change=false) {
	global $quick_entry_types;
		
	echo "<tr><td class='label'>".$label."</td><td>";
	echo array_selector($name, $selected_id, $quick_entry_types, 
		array( 
			'select_submit'=> $submit_on_change
			) );
	echo "</td></tr>\n";
}

function record_status_list_row($label, $name) {
	return yesno_list_row($label, $name, null, 	_('Inactive'), _('Active'));
}

function class_types_list_row($label, $name, $selected_id=null, $submit_on_change=false) {
	global $class_types;
	
	echo "<tr><td class='label'>".$label."</td><td>";
	echo array_selector($name, $selected_id, $class_types, array('select_submit'=> $submit_on_change));
	echo "</td></tr>\n";
}

//------------------------------------------------------------------------------------------------

function security_roles_list($name, $selected_id=null, $new_item=false, $submit_on_change=false, $show_inactive=false) {

	$sql = "SELECT id, role, inactive FROM ".TB_PREF."security_roles";

	return combo_input($name, $selected_id, $sql, 'id', 'description',
		array(
			'spec_option'=>$new_item ? _('新角色') : false,
			'spec_id' => '',
			'select_submit'=> $submit_on_change,
			'show_inactive' => $show_inactive
		) );
}

function security_roles_list_cells($label, $name, $selected_id=null, $new_item=false, $submit_on_change=false, $show_inactive=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo security_roles_list($name, $selected_id, $new_item, $submit_on_change, $show_inactive);
	echo "</td>\n";
}

function security_roles_list_row($label, $name, $selected_id=null, $new_item=false, $submit_on_change=false, $show_inactive=false) {
	echo "<tr><td class='label'>".$label."</td>";
	security_roles_list_cells(null, $name, $selected_id, $new_item, $submit_on_change, $show_inactive);
	echo "</tr>\n";
}

function tab_list_row($label, $name, $selected_id=null) {
	global $installed_extensions;
	
	$tabs = array();
	foreach ($_SESSION['App']->applications as $app) {
		$tabs[$app->id] = access_string($app->name, true);
	}
	echo "<tr>\n";
	echo "<td class='label'>".$label."</td><td>\n";
	echo array_selector($name, $selected_id, $tabs);
	echo "</td></tr>\n";
}


//-----------------------------------------------------------------------------------------------

function tag_list($name, $height, $type, $multi=false, $all=false, $spec_opt=false) {
	// Get tags
	global $path_to_root;
	include_once($path_to_root.'/admin/db/tags_db.inc');
	$results = get_tags($type, $all);

	while ($tag = db_fetch($results))
		$tags[$tag['id']] = $tag['name'];
	
	if (!isset($tags)) {
		$tags[''] = $all ? _('No tags defined.') : _('No active tags defined.');
		$spec_opt = false;
	}
	return array_selector($name, null, $tags,
		array(
			'multi' => $multi,
			'height' => $height,
			'spec_option'=> $spec_opt,
			'spec_id' => -1,
		) );
}

function tag_list_cells($label, $name, $height, $type, $mult=false, $all=false, $spec_opt=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>\n";
	echo tag_list($name, $height, $type, $mult, $all, $spec_opt);
	echo "</td>\n";
	
}

function tag_list_row($label, $name, $height, $type, $mult=false, $all=false, $spec_opt=false) {
	echo "<tr><td class='label'>".$label."</td>";
	tag_list_cells(null, $name, $height, $type, $mult, $all, $spec_opt);
	echo "</tr>\n";	
}

//---------------------------------------------------------------------------------------------
//	List of sets of active extensions 
//
function extset_list($name, $value=null, $submit_on_change=false) {
	global $db_connections;

	$items = array();
	foreach ($db_connections as $comp)
		$items[] = sprintf(_("Activated for '%s'"), $comp['name']);
	return array_selector( $name, $value, $items,
		array(
			'spec_option'=> _('Available and/or installed'),
			'spec_id' => -1,
			'select_submit'=> $submit_on_change,
			'async' => true
		));
}

function crm_category_types_list($name, $selected_id=null, $filter=array(), $submit_on_change=true) {

	$sql = "SELECT id, name, type, inactive FROM ".TB_PREF."crm_categories";

	$multi = false;
	$groups = false;
	$where = array();
	if (@$filter['class'])
		$where[] = 'type='.db_escape($filter['class']);
	else
		$groups = 'type';
	if (@$filter['subclass'])
		$where[] = 'action='.db_escape($filter['subclass']);
	if (@$filter['entity'])
		$where[] = 'entity_id='.db_escape($filter['entity']);
	if (@$filter['multi']) // contact category selector for person
		$multi = true;

	return combo_input($name, $selected_id, $sql, 'id', 'name',
		array(
			'multi' => $multi,
			'height' => $multi ? 5:1,
			'category' => $groups,
			'select_submit'=> $submit_on_change,
			'async' => true,
			'where' => $where
		));
}

function crm_category_types_list_row($label, $name, $selected_id=null, $filter=array(), $submit_on_change=true) {
	echo "<tr><td class='label'>".$label."</td><td>";
	echo crm_category_types_list($name, $selected_id, $filter, $submit_on_change);
	echo "</td></tr>\n";
}

function payment_type_list_row($label, $name, $selected_id=null, $submit_on_change=false) {
	global $pterm_types;
	
	echo "<tr><td class='label'>".$label."</td><td>";
	echo array_selector($name, $selected_id, $pterm_types, 
		array( 
			'select_submit'=> $submit_on_change
		) );
	echo "</td></tr>\n";
}

function coa_list_row($label, $name, $value=null) {
	global $path_to_root, $installed_extensions;

	$path = $path_to_root.'/sql/';
	$coas = array();
	$sqldir = opendir($path);
	while (false !== ($fname = readdir($sqldir))) {
		if (is_file($path.$fname) && substr($fname,-4)=='.sql' && @($fname[2] == '_')) {
			$ext = array_search_value($fname, $installed_extensions, 'sql');
			if ($ext!=null)
				$descr = $ext['name'];
			elseif ($fname == 'en_US-new.sql') // two standard COAs
				$descr = _('正式版本');
			elseif ($fname == 'en_US-demo.sql')
				$descr = _('Demo版本');
			else
				$descr = $fname;

			$coas[$fname] =  $descr;
		}
	}
	ksort($coas);

	echo "<tr><td class='label'>".$label."</td>\n<td>";
	echo array_selector( $name, $value, $coas );
	echo "</td></tr>\n";
}

function payment_services($name) {
	global $payment_services;

	$services = (object)array_combine(array_keys($payment_services), array_keys($payment_services));

	return array_selector($name, null, $services, array(
			'spec_option'=> _('No payment Link'),
			'spec_id' => '',
		));
}

function tax_algorithm_list($name, $value=null, $submit_on_change=false) {
	global $tax_algorithms;
	
	return array_selector($name, $value, $tax_algorithms, 
		array( 
			'select_submit'=> $submit_on_change,
			'async' => true,
			)
	);
}

function tax_algorithm_list_cells($label, $name, $value=null, $submit_on_change=false) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";
	echo tax_algorithm_list($name, $value, $submit_on_change);
	echo "</td>\n";
}

function tax_algorithm_list_row($label, $name, $value=null, $submit_on_change=false) {
	echo "<tr><td class='label'>".$label."</td>";
	tax_algorithm_list_cells(null, $name, $value, $submit_on_change);
	echo "</tr>\n";
}

function refline_list($name, $type, $value=null, $spec_option=false) {
	$sql = "SELECT id, prefix, inactive FROM ".TB_PREF."reflines";

	$where = array();

	if (isset($type))
		$where = array('`trans_type`='.db_escape($type));

	return combo_input($name, $value, $sql, 'id', 'prefix',
		array(
			'order'=>array('prefix'),
			'spec_option' => $spec_option,
			'spec_id' => '',
			'type' => 2,
			'where' => $where,
			'select_submit' => true,
			)
		);
}

function refline_list_row($label, $name, $type, $selected_id=null, $spec_option=false) {
	echo "<tr>";
	if ($label != null)
		echo "<td class='label'>".$label."</td>\n";
	echo "<td>";

	echo refline_list($name, $type, $selected_id, $spec_option);
	echo "</td></tr>\n";
}


//----------------------------------------------------------------------------------------------

function subledger_list($name, $account, $selected_id=null) {

	$type = is_subledger_account($account);
	if (!$type)
		return '';

	if($type > 0)
		$sql = "SELECT DISTINCT d.debtor_no as id, debtor_ref as name 
		FROM "
		.TB_PREF."debtors_master d,"
		.TB_PREF."cust_branch c
		WHERE d.debtor_no=c.debtor_no AND NOT d.inactive AND c.receivables_account=".db_escape($account);
	else
		$sql = "SELECT supplier_id as id, supp_ref as name 
		FROM "
		.TB_PREF."suppliers s
		WHERE NOT s.inactive AND s.payable_account=".db_escape($account);

	$mode = get_company_pref('no_customer_list');

	return combo_input($name, $selected_id, $sql, 'id', 'name',
	array(
		'type' => 1,
		'size' => 20,
		'async' => false,
	) );
}

function subledger_list_cells($label, $name, $account, $selected_id=null) {
	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td nowrap>";
	echo subledger_list($name, $account, $selected_id);
	echo "</td>\n";
}

function subledger_list_row($label, $name, $selected_id=null, $all_option=false, $submit_on_change=false, $show_inactive=false, $editkey=false) {
	echo "<tr><td class='label'>".$label."</td><td nowrap>";
	echo subledger_list($name, $account, $selected_id);
	echo "</td>\n</tr>\n";
}

function accounts_type_list_row($label, $name, $selected_id=null) {
	echo "<tr>";
	if ($label != null)
		echo "<td class='label'>".$label."</td>\n";
	echo "<td>";
	$sel = array(_('Numeric'), _('Alpha Numeric'), _('ALPHA NUMERIC'));
	echo array_selector($name, $selected_id, $sel); 
	echo "</td></tr>\n";
}

function users_list_cells($label, $name, $selected_id=null, $submit_on_change=false, $spec_opt=true) {
	$where = false;
	$sql = " SELECT user_id, IF(real_name = '', user_id, real_name) FROM ".TB_PREF."users";

	if ($label != null)
		echo "<td>".$label."</td>\n";
	echo "<td>";

	echo combo_input($name, $selected_id, $sql, 'user_id', 'real_name',
		array(
			'spec_option' => $spec_opt===true ?_('All users') : $spec_opt,
			'spec_id' => '',
			'order' => 'real_name',
			'select_submit'=> $submit_on_change,
			'async' => false
		) );
	echo "</td>";

}

function collations_list_row($label, $name, $selected_id=null) {
	global $supported_collations;

	echo "<tr>";
	if ($label != null)
		echo "<td class='label'>".$label."</td>\n";
	echo "<td>";

	echo array_selector($name, $selected_id, $supported_collations, 
		array('select_submit'=> false) );
	echo "</td></tr>\n";
}
