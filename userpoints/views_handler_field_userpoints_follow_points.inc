<?php
// $Id: views_handler_field_pollfield_delta.inc,v 1.1.2.1 2010/02/06 01:12:40 marioprkos Exp $ 

/**
 * Field handler to translate a node type into its readable form.
 */


class views_handler_field_userpoints_follow_points extends views_handler_field {
  function query() {
  }

  function render($values){

    $nid = $values->userpoints_txn_entity_id;
    $uid = $values->userpoints_txn_uid;

    $_query = sprintf("SELECT sum(points) sum_points FROM userpoints_txn WHERE entity_id = %d AND uid = %d AND operation = 'Flag FavoriteTVShow' GROUP BY entity_id, uid, operation ", $nid, $uid, $nid);

    $follow_points = db_result(db_query($_query));

    return $follow_points;
  }
}

