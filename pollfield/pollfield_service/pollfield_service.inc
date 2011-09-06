<?php
// $Id$

/**
 *  @file
 *    Link general private message module functionalities to service module
 */

/**
 *  vote(select one of the choices) in issue poll
 *
 *  @param $poll_id
 *    Integer. which issue poll user like to vote
 *  
 *  @param $choice
 *    Integer. Selection of choice. in database, it refers to 'delta'.
 *  
 *  @param $user_id (optional)
 *    Integer. uid whose voted.
 *
 *  @return
 *    boolean: true - success, false - already voted.
 */
function pollfield_service_vote($nid,$choice){

  global $user;
  if (!user_is_logged_in()) {
    return services_error(t('This user is not logged in.'), 403);
  }

  /*
  if (is_numeric($uid) && ($uid != $user->uid)) {
    if (user_access("vote issue poll from remote")) {
      $account = user_load($uid);
    }
    else {
      return services_error(t('This user does not have permissions to vote issue poll.'), 403);
    }
  }
  else {
    $account = $user;
  }
  */

  $uid = $user->uid;
  $field_name = 'field_poll';
  $field_name_delta = '0';
  $field_table = 'issue_poll';
  $cancel = '0';
  $node = node_load($nid);
  $voted = FALSE;

  // choice number stars with 1.
  $choice = $choice-1;

  $fields = content_fields();
  $db_info = content_database_info($fields[$field_name]);
  $table_content = $db_info['table'];
  $column = 'delta';
  $is_delta_exist = db_column_exists($table_content, $column);

  if ($is_delta_exist) {
    $existing_field = 1;
    // get total votes
    $field_votes = $field_name .'_votes';
    $total_votes = db_result(db_query("SELECT %s FROM {%s} WHERE nid=%d AND delta=%d", $field_votes, $table_content, $nid, $field_name_delta));
  }
  else {
    // get total votes
    $field_votes = $field_name .'_votes';
    $total_votes = db_result(db_query("SELECT %s FROM {%s} WHERE nid = %d", $field_votes, $table_content, $nid));
  }
  // ************************

  $voted = pollfield_user_voted($node, $field_table, $field_name, $field_name_delta);
  if ($voted == 0) {
      if ($uid>0) {
        db_query("INSERT INTO {pollfield_votes} (nid, field_table, field_name, uid, delta, field_name_delta)
        VALUES (%d, '%s', '%s' , %d, %d, %d)", $nid, $field_table, $field_name, $uid, $choice, $field_name_delta);
        // increment total votes
        if ($existing_field == 1) {
          $total_votes++;
          db_query('UPDATE {%s} SET %s = %d WHERE nid = %d AND delta=%d',
              $table_content, $field_votes, $total_votes, $nid, $field_name_delta);
        }
        else {
          $total_votes++;
          db_query('UPDATE {%s} SET %s = %d WHERE nid = %d', $table_content, $field_votes, $total_votes, $nid);
        }
      }
      else {
        db_query("INSERT INTO {pollfield_votes} (nid, field_table, field_name, delta, hostname, field_name_delta, cookie)
          VALUES (%d, '%s', '%s', %d, '%s', %d, '%s')",
          $nid, $field_table, $field_name, $choice, $hostname, $field_name_delta, $cookie_db);
      }
    // Any time a vote is recorded, clear the CCK cache so the votes can be updated.
    pollfield_clear($node);
    drupal_set_message(t('Your vote was recorded.'));
  }
  else {
    drupal_set_message(t('You already voted or your are not allowed to vote'), 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 *  create issue poll
 *
 *  @param $question
 *    String. question of issue poll
 *  
 *  @param $choices
 *    Array. list of choice
 *  
 *  @return
 *    boolean: true - success, false - already voted.
 */
function pollfield_service_create($nid,$question,$choices){

  global $user;
  if (!user_is_logged_in()) {
    return services_error(t('This user is not logged in.'), 403);
  }

  $node = new stdClass();

  // Main Node Fields
  $node->uid = $user->uid;
  $node->name = $question;
  $node->title = $node->name;
  $node->body = "";
  $node->type = 'issue_poll';
  $node->created = time();
  $node->changed = $node->created;
  $node->promote = 0;
  $node->sticky = 0;
  $node->format = 2;
  $node->status = 1;
  $node->language = 'en';
  $node->comment = 2; // allow for read-write

  // CCK Node Reference Fields
  $node->field_tvshow[0]['nid'] = $nid;

  $poll_features = array();
  $poll_features['CancelVote_Allow'] = '1';
  $poll_features['ResultsAllow'] = '1';
  $poll_features['PollType'] = '0';

  $index = 0;
  $choices_array=array();
  foreach ($choices as $choice) {
    $choices_array[$index]['choice'] = $choice;
    $choices_array[$index++]['votes'] = "";
  }

  $node->field_poll[0]['choice'] = $choices_array;
  $node->field_poll[0]['question'] = $question;
  $node->field_poll[0]['active'] = '1';
  $node->field_poll[0]['runtime'] = '0';
  $node->field_poll[0]['votes'] = '0';
  $node->field_poll[0]['anonymous'] = 'anon_non';
  // $node->field_poll[0]['poll_features'] = serialize($poll_features);
  $node->field_poll[0]['poll_features'] = $poll_features;

  node_save($node);

  $is_result = db_query("UPDATE content_field_poll SET field_poll_choice = '%s' WHERE nid = %d", serialize($choices_array), $node->nid);

  if ($node->nid > 0) {
    return TRUE;
  } else {
    return FALSE;
  };

  return TRUE;
}

/**
 *  delete issue poll
 *
 *  @param $nid
 *    issue id(nid)
 *  
 *  @return
 *    boolean: true - success, false - already voted.
 */
function pollfield_service_delete($nid){

  /*
  if (comment_num_all($nid) > 3) {
    return FALSE;
  }
  */

  if (node_delete($nid)) {
    return TRUE;
  };

  return  FALSE;
}

