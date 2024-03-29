<?php
// $Id: views_handler_field_pollfield_question.inc,v 1.1.2.1 2010/02/06 01:12:40 marioprkos Exp $

/**
 * Field handler to translate a node type into its readable form.
 */

class views_handler_field_pollfield_question extends views_handler_field {
  function query() {
    
  }

  function render($values) {

    $number=$values->pollfield_delta;
    $nid=$values->pollfield_nid;
    $field_name_delta=$values->pollfield_field_name_delta;
    $field_name=$values->pollfield_field_name;

	//get table information
    $fields = content_fields();
    $db_info = content_database_info($fields[$field_name]);
    $table_content = $db_info['table'];
    $column_choice = $field_name ."_choice";
    $column_question = $field_name ."_question";
    $column = 'delta';
    $is_delta_exist = db_column_exists($table_content, $column);

    //get question and choices
    if (!$is_delta_exist) {
        $poll_question=db_result(db_query("SELECT %s FROM {%s} WHERE nid=%d",
	            $column_question,$table_content,$nid));
    } else{
	    $poll_question=db_result(db_query("SELECT %s FROM {%s} WHERE nid=%d AND delta=%d", $column_question,$table_content,$nid,$field_name_delta));
		}
	
    return $poll_question;
  }
}

