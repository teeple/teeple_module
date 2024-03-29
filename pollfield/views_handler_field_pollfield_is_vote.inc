<?php
// $Id: views_handler_field_pollfield_delta.inc,v 1.1.2.1 2010/02/06 01:12:40 marioprkos Exp $

/**
 * Field handler to translate a node type into its readable form.
 */

class views_handler_field_pollfield_is_vote extends views_handler_field {

  function query(  ) {
  }

  function render($values) {

    global $user;

		$nid=$values->nid;
    $field_table=$values->pollfield_field_table;
    $field_name=$values->pollfield_field_name;
    $field_name_delta=$values->pollfield_field_name_delta;

    $node=node_load($nid);

    $is_vote = pollfield_user_voted($node, $field_table, $field_name, $field_name_delta);

    $return = 'NO';

    if ($is_vote > 0) {
       $return = 'YES';
    };

    return $return;
  }
}
