<?php
// $Id: views_handler_field_pollfield_delta.inc,v 1.1.2.1 2010/02/06 01:12:40 marioprkos Exp $

/**
 * Field handler to translate a node type into its readable form.
 */

class views_handler_field_pollfield_results extends views_handler_field {

  function query(  ) {
  }

  function render($values) {

    // $number=$values->pollfield_votes_delta;
		// $nid=$values->pollfield_votes_nid;
		$nid=$values->nid;
    $type=$values->pollfield_field_table;
    $field_name=$values->pollfield_field_name;
    $field_name_delta=$values->pollfield_field_name_delta;

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
       
			 $poll_choices=db_result(db_query("SELECT %s FROM {%s} WHERE nid=%d",
															$column_choice,$table_content,$nid));
    }
		else{
			 $poll_choices=db_result(db_query("SELECT %s FROM {%s} WHERE nid=%d AND delta=%d",
															$column_choice,$table_content,$nid,$field_name_delta));
			
		}
		$choices=unserialize($poll_choices);

    $return_value = "";

    $votes=array();$choices_array=array();
    if (!empty($choices)) {
      foreach ($choices as $delta => $choice) {

        if (!empty($choice['choice'])) {
          $show_form = TRUE;
          $choices_array[] = check_plain($choice['choice']);

          $count_node = db_result(db_query("SELECT count(*) FROM {pollfield_votes} WHERE nid = %d AND delta=%d
                AND field_table='%s' AND field_name='%s' AND field_name_delta=%d",$nid, $delta, $type, $field_name, $field_name_delta));
          $count_translate=0;
          if (!empty($t_nodes_nid)){
            foreach($t_nodes_nid as $tnode){

              $count_temp = db_result(db_query("SELECT count(*) FROM {pollfield_votes} WHERE nid= %d AND delta=%d
                    AND field_table='%s' AND field_name='%s' AND field_name_delta=%d",
                    $tnode,$nid, $delta, $type, $field_name, $field_name_delta));
              $count_translate=$count_translate+$count_temp;
            }
          }
          $count=$count_node+$count_translate;
          $votes[] = $count+$choice['votes']+$count_translate_votes[$delta];
          $sum = $sum+$count+$choice['votes']+$count_translate_votes[$delta];//countig all votes
        }
      }
    }

    return implode(",", $votes);
  }
}

