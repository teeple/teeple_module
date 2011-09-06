<?php
// $Id: views_handler_field_pollfield_delta.inc,v 1.1.2.1 2010/02/06 01:12:40 marioprkos Exp $ 

/**
 * Field handler to translate a node type into its readable form.
 */


class views_handler_field_userpoints_checkin_points extends views_handler_field {
  function query() {
  }

  function render($values){

    $nid = $values->userpoints_txn_entity_id;
    $uid = $values->userpoints_txn_uid;

    $_query = sprintf("SELECT sum(points) sum_points FROM userpoints_txn WHERE uid = %d AND operation = 'Flag Check In' GROUP BY entity_id HAVING entity_id = %d", $uid, $nid);

    $checkin_points = db_result(db_query($_query));

    return $checkin_points;
  }
}

