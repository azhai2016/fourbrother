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
//----------------------------------------------------------------------------------
//    CbSMS global system names. They have to be stored separate from
//    sys types defines to enable inclusion after every language change
//    (which happens for multilanguage bulk reports)
//
global $print_actions,$systypes_array, $bank_account_types, $bank_transfer_types, $payment_person_types, $wo_types_array, $wo_cost_types, $class_types, $quick_actions, $quick_entry_types, $stock_types, $tag_types, $bank_owner;

$systypes_array = array(
);

$type_shortcuts = array(
   // ST_ORDER_ID => _('OR'),
);

$print_actions = array(

);

//----------------------------------------------------------------------------------
//    Quick entry types
//
$quick_actions = array(
    '=' => _('Remainder'), // post current base amount to GL account
    'a' => _('Amount'), // post amount to GL account
    'a+' => _('Amount, increase base'), // post amount to GL account and increase base
    'a-' => _('Amount, reduce base'), // post amount to GL account and reduce base
    '%' => _('% amount of base'), // store acc*amount% to GL account
    '%+' => _('% amount of base, increase base'), // ditto & increase base amount
    '%-' => _('% amount of base, reduce base'), // ditto & reduce base amount
    'T' => _('Taxes added'), // post taxes calculated on base amount
    'T+' => _('Taxes added, increase base'), // ditto & increase base amount
    'T-' => _('Taxes added, reduce base'), // ditto & reduce base amount
    't' => _('Taxes included'), // post taxes calculated on base amount
    't+' => _('Taxes included, increase base'), // ditto & increase base amount
    't-' => _('Taxes included, reduce base'), // ditto & reduce base amount
);

// This month array is for use with the last 3 dateformats.
$tmonths = array('', _('一'), _('二'), _('三'), _('四'), _('五'), _('六'), _('七'), _('八'), _('九'), _('十'), _('十一'), _('十二'));


//----------------------------------------------------------------------------------
// Types of stock items
$item_types = array(

);


$supported_collations = array(
	'utf8' => _('Unicode(多语言)')
);
