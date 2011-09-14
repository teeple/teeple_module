<?php
include 'XMPPHP/XMPP.php';

/*
 COMMAND ADD 
*/
/**
  print check status
*/
function tvchat_print_checkin_count( $nid, $prompt="오늘 체크인 회수 : ")
{
  $count = _count_today_checkin( $nid);
  $output = theme( 'tvchat_checkin_count', $nid, $prompt, $count);
//  drupal_set_message( t('print checkin_count'.$output));

  return $output;
}

/*
* count the number of checkins today
*/
function _count_today_checkin( $nid)
{
  static $cache = array();
  $timestamp = mktime( 0, 0, 0);		// timestamp of today  at 0:0:0

  if (($flag = flag_get_flag( 'checkin'))) {

	if ( ! isset( $cache[$nid][$interval])) {
	  $cache[$nid][$interval] = db_result( db_query(
	    'SELECT COUNT(*) FROM {flag_content} WHERE fid=%d AND content_id=%d AND timestamp > %d', 
		$flag->fid, $nid, $timestamp));
	}

	return $cache[$nid][$interval];
  }
}

/*
	theme functions
*/
function theme_tvchat_checkin_count( $nid, $prompt, $count)
{
  $output = sprintf( '<div class="tvchat_checkin_count" id="tvchat_checkin_count_%d">%s %d</div>', 
    $nid, t( $prompt), $count);

  return $output;
}


/* 
	AJAX handler
	@return 
	HTML
*/
function tvchat_checkin_count( )
{
  $nid = $_GET['nid'];
  drupal_set_message( t('tvchat_checkin_count called, nid='.$nid));

  drupal_set_header( 'Content-Type: text/plain; charset; utf-8');
  print tvchat_print_checkin_count( $nid);
}

/*
* create check-in link
*/
function tvchat_create_checkin_link( $nid, $start_time, $end_time)
{
  $now = date( 'Hi');
	if ( $now >= $start_time && $now <= $end_time) {
/* check time first */
		print flag_create_link( 'checkin', $nid);
	}
	else {
		print '방송 시간 종료';
	}
}

/*
*/
function tvchat_is_checkin_time( $nid, $start_time, $end_time)
{
  $now = date( 'Hi');
	if ( $now >= $start_time && $now <= $end_time) {
/* check time first */
    $flag = flag_get_flag('checkin');

		global $user;

		drupal_set_message( 'checkIn: nid='.$nid.'uid='.$user->uid."flagged=".$flag->is_flagged( $nid, $user->uid));
		if ( $flag && $flag->is_flagged( $nid, $user->uid))
			return FALSE;
    return TRUE;
	}
	return FALSE;
}


/* 
	category = ( checkin_count | favorite_count | comment_count )

	defaul interval = 7 days
*/
function tvchat_get_count( $content_id, $category, $interval=7)
{
	$from_day = date( 'Y-m-d', time() - 3600*24*$interval);

	/* get count */
	$sql = sprintf( 'SELECT sum(%s) FROM tvchat_rating WHERE content_id = %d AND date >= "%s" ',
	  $category, $content_id, $from_day);
	$count = db_result( db_query( $sql));

//	drupal_set_message( t('SQL : '.$sql));

	return $count;
}

//
// returns event for the program 
//
function tvchat_get_events($nid)
{
  $events = array();
  $event_id = array();
  $event_name = array();
  $sql = 'SELECT n.nid, title FROM node n, content_field_tvshow p WHERE n.nid=p.nid and p.field_tvshow_nid = %d and n.type="event"';
  $result = db_query($sql, $nid);
  while($row = db_fetch_object($result)) {
    array_push($event_id, $row->nid);
    array_push($event_name, $row->title);
  }
  array_push($events, $event_id, $event_name); 
//  dsm(t('SQL : '.$sql));
//  dsm(join($event_id, ','));
//  dsm(join($event_name, ','));
//  dsm($events);
  return $events;
}

//
// returns friend count
//
function tvchat_get_friend_count($uid)
{
	$sql = "SELECT COUNT(*) FROM user_relationships WHERE requester_id = %d AND approved = 1";
	$num_friends = db_result(db_query($sql, $uid));
	return $num_friends;	
}

//
// returns node id
//
function tvchat_search_node($title)
{
	$token = strtok($title, " ");
	$subquery = array();
	while($token != false) {
		array_push($subquery, sprintf("SELECT nid FROM node WHERE title LIKE '%%%s%%' and type = 'tvshow'", $token));
		$token = strtok(" ");
	}
	$sql = join(" UNION ", $subquery);

	$result = db_query($sql);
	$nodes = array();
  	while($row = db_result($result)) {
    	array_push($nodes, $row);
  	}
	return $nodes;
}	

//
// returns user flagged nodes 
//
function tvchat_get_flagged_node($flag_name, $uid) {
	$sql = "SELECT content_id FROM flag_content as c, flags as f WHERE f.fid = c.fid AND name = '%s' and uid = %d"; 
	$result = db_query($sql, $flag_name, $uid);
	$nodes = array();
  while($row = db_result($result)) {
    	array_push($nodes, $row);
  }
	return $nodes;
}	

function _calculate_tvshow($table, $array_operation, $timestamp_from, $timestamp_to, $target_date, $timestamp) {

  // count(entity_id) in userpoints_txn
  // 
  $_query = sprintf("select count(distinct(entity_id)) total_count FROM userpoints_txn WHERE operation in ('%s') AND points > 0 AND uid != 1", implode("','", $array_operation['tvshow']));
  $total_count = db_result(db_query($_query));

  // calculate for tvshow
  //
  $_query = sprintf("select entity_id, sum(points) points FROM userpoints_txn WHERE operation in ('%s') and time_stamp between %d and %d AND uid != 1 Group By entity_id HAVING points > 0 ORDER BY points DESC ", implode("','", $array_operation['tvshow']), $timestamp_from , $timestamp_to);
  $query = db_query($_query);

  $max_top_show = variable_get('max_top_show', 10);
  $count = 0;

  if( $table == 'tvchat_ranking_weekly') {
      $num_rows = db_affected_rows();

      while( ($num_rows < $max_top_show) && ($num_rows < $total_count) && ($count < $total_count ) ) {

          $count ++;

          $timestamp_from = $timestamp_from - 86400 * 7;
          $_query = sprintf("select entity_id, sum(points) points FROM userpoints_txn WHERE operation in ('%s') and time_stamp between %d and %d AND uid != 1 Group By entity_id HAVING points > 0 ORDER BY points DESC", implode("','", $array_operation['tvshow']), $timestamp_from , $timestamp_to);
          $query = db_query($_query);
          $num_rows = db_affected_rows();
      }
  }

  $count = 0;
  while($element = db_fetch_object($query)) {
    $node = node_load($element->entity_id);

    if (isset($node)) {
      if( ($table=='tvchat_ranking_weekly')&&((++$count)>($max_top_show))) {
        break;
      } 

      if (isset($node->field_genre[0]['value'])) {
          $genre = $node->field_genre[0]['value'];
      } else {
          $genre = 0;
      }

      $ranking_element = array(
        'entity_type'       => 'tvshow',
        'entity_nid'        =>  $element->entity_id,
        'entity_taxonomy'   => $genre,
        'points'            => $element->points,
        'target_date'       => $target_date,
        'timestamp'         => $timestamp
      );
      drupal_write_record($table, $ranking_element);
    }
  }
}

function _calculate_isse($table, $timestamp_from, $timestamp_to) {

  // calculate for issue
  //
  $query = db_query("select entity_id, sum(points) points FROM {userpoints_txn} WHERE operation = '%s' and time_stamp between %d and %d Group By entity_id", $array_operation['issue'][0], $timestamp_from , $timestamp_to);

  while($element = db_fetch_object($query)) {

    $node = node_load($element->entity_id);

    if (isset($node)) {
      $ranking_element = array(
        'entity_type'       => 'issue',
        'entity_nid'        => $element->entity_id,
        'entity_taxonomy'   => 0,
        'points'            => $element->points,
        'target_date'       => $target_date,
        'timestamp'         => $timestamp
      );
      drupal_write_record($table, $ranking_element);

    }
  }
}

function _calculate_user($table, $timestamp_from, $timestamp_to) {

  // calculate for user
  // 
  $query = db_query("select uid, sum(points) points FROM {userpoints_txn} WHERE time_stamp between %d and %d Group By uid", $timestamp_from , $timestamp_to);

  while($element = db_fetch_object($query)) {

    $user = user_load($element->uid);

    if (isset($user)) {
        $ranking_element = array(
            'entity_type'       => 'user',
            'entity_uid'        => $element->entity_uid,
            'entity_taxonomy'   => 0,
            'points'            => $element->points,
            'target_date'       => $target_date,
            'timestamp'         => $timestamp
        );
        drupal_write_record($table, $ranking_element);

    }
  }
}

function tvchat_calculate_ranking($table, $timestamp_from, $timestamp_to) {

  // get date from timestamp_from
  //
  $target_date = date('Ymd', $timestamp_to);
  $timestamp = time();

  $array_operation = array();
  $array_operation['tvshow'] = array('Comment: tvshow', 'Flag Check In', 'Flag FavoriteTVShow');
  $array_operation['issue'] = array('Comment: issue');

  _calculate_tvshow($table, $array_operation, $timestamp_from, $timestamp_to, $target_date, $timestamp);

  // _calculate_issue($table, $array_operation, $timestamp_from, $timestamp_to);

  // _calculate_user($table, $timestamp_from, $timestamp_to);

}

function tvchat_comment_master_change($node, $user, $prev_user) {

  $comment = new stdClass();

  $comment->uid = $node->uid;
  $comment->nid = $node->nid;
  $comment->subject = $node->title.'의 마스터가 '.$prev_user->name.'에서 '.$user->name.'(으)로 변경되었습니다.';
  $comment->comment = $comment->subject;
  $comment->timestamp = time();
  $comment->status = 0;
  $comment->format = 1;
  $comment->name = $user->name;

  comment_save((array)$comment);
}

function tvchat_assign_master($table, $timestamp_from, $timestamp_to) {  

  $timestamp = time();
  $array_operation = array();
  $array_operation['tvshow'] = array('Commnet: tvshow', 'Flag Check In', 'Flag FavoriteTVShow');
  $array_operation['issue'] = array('Comment: issue');

  // calculate for tvshow
  $_query = sprintf("SELECT entity_id, uid, sum(points) points FROM userpoints_txn WHERE operation in ('%s') and time_stamp between %d and %d GROUP BY entity_id, uid ORDER By entity_id DESC, sum(points) DESC", implode("','",$array_operation['tvshow']), $timestamp_from, $timestamp_to);

  // get date from timestamp_from
  $target_date = date('Ymd', $timestamp_to);

  $query = db_query($_query);

  $prev = 0;
  while($element = db_fetch_object($query)) {

    // skip same entity_id 
    if ($prev == $element->entity_id ) {
      continue;
    } 

    $node = node_load($element->entity_id);
    $user = user_load($element->uid);

    if (isset($node) && isset($user)) {

      // same if the master is same , go on next node 
      if ($node->field_master[0]['uid'] == $user->uid) {
          continue;
      }

      $ranking_element = array(
        'entity_type' => 'master',
        'entity_uid'    => $element->uid,
        'entity_taxonomy'   => 0,
        'points'            => $element->points,
        'target_date'       => $target_date,
        'entity_id'         => $element->entity_id,
        'timestamp'         => $timestamp
      );
      drupal_write_record($table, $ranking_element);

      $prev_user = user_load($node->field_master[0]['uid']);

      // change master 
      $node->field_master[0]['uid']= (string)$user->uid;

      // save changed node
      // node_save($node);
      content_update($node);

      tvchat_comment_master_change($node, $user, $prev_user);

      db_query("DELETE FROM {cache_content} WHERE cid = '%s'", 'content:' . $node->nid . ':' . $node->vid);

      // load the OLD cached version
      node_load(0, NULL, TRUE);

    } else {
      watchdog('cron_assign_master', 'entity_id: '.$element->id." doesn't exist");
    }
    $prev = $element->entity_id;

  }
}

function tvchat_issue_nids($showId) {

  /*
  module_load_include('inc', 'views_service');
  $result = views_service_get('list_issue_json', null, null, 0, 0, FALSE);
  dsm($result);
  if(isset($result) && (count($result) > 0))  {
      return array_values($result);
  }
  */

  // view: list_issue_json
  // 
  $_query = sprintf("SELECT node.nid AS nid FROM node node LEFT JOIN content_field_tvshow node_data_field_tvshow ON node.vid = node_data_field_tvshow.vid LEFT JOIN node node_node_data_field_tvshow ON node_data_field_tvshow.field_tvshow_nid = node_node_data_field_tvshow.nid WHERE (node.type in ('forum', 'issue_poll')) AND (node_node_data_field_tvshow.nid = %d) ", $showId);

  $uid_list = array();

  $results = db_query($_query);
  while($element = db_fetch_object($results)) {
      $uid_list[] = $element->nid;
  }

  if (isset($uid_list) && count($uid_list) > 0) {
    return array_values($uid_list);
  }

  return NULL;
}

// parameter format : $fromDate - 2011-05-20
// 
function tvchat_timetable_making($fromDate=NULL, $toDate=NULL) {
  global $user;

  module_load_include('inc', 'views_service');

  if (!isset($fromDate)) {
      $fromDate = date('Y-m-d', strtotime('today'));
  }

  if (!isset($toDate)) {
      $toDate = date('Y-m-d', strtotime('next Sunday'));
  }

  $args_date = sprintf("[\"%sT00:00:00--%sT23:59:59\"]", $fromDate, $toDate);
  $query_string = sprintf('sessid="%s"&method="%s"&view_name="%s"&args=%s&format_output=1', $user->sid, "views.get", "list_show_timetable_json", $args_date);

  $json_server = sprintf("http://%s/tvchat/services/json", $_SERVER['HTTP_HOST']);

  $result = drupal_http_request(
  url($json_server, array(absolute => TRUE)),
    array('Content-Type' => 'application/x-www-form-urlencoded'),
    'POST',
    $query_string
  );

  // $gzdata = gzencode($result->data,9);
  $gzdate = $result->data;
  $dir_timetable = variable_get('dir_timetable', '/var/www/data');

  // rename existing file
  // 
  $latest_filename = $dir_timetable."/timetable.txt";
  $prev_filename = $dir_timetable."/timetable.prev.txt";
  $backup_filename = sprintf("%s/timetable_%s_%s.txt", $dir_timetable, $fromDate, $toDate);

  if (file_exists($latest_filename)) {
    rename($latest_filename, $prev_filename);
  }

  // write file
  // 
  $file1 = fopen($latest_filename,'wb');
  $file2 = fopen($backup_filename, 'wb');
  fwrite($file1, $gzdata);
  fwrite($file2, $gzdata);
  fclose($file1);
  fclose($file2);
}

function tvchat_friends_uid_list(){
  module_load_include('inc', 'user_relationship_tvchat_service');

  try {
    global $user;

    $friends_list = user_relationship_tvchat_service_mine_with_type('1', 'Friend');

    $friends_uid_list = array();

    if (isset($friends_list) && count($friends_list) > 0 ) {
      foreach($friends_list as $friend) {
        $friends_uid_list[] = $friend->requester_id;
        $friends_uid_list[] = $friend->requestee_id;
      } 
      $friends_uid_list = array_unique($friends_uid_list);
      $friends_uid_list = array_diff($friends_uid_list, (array)$user->uid);
    }
    return array_values($friends_uid_list);

  } catch (Exception $ex) {
    return services_error(t('Error getting friend_list: $msg', array('@msg' => $ex->getMessage())));
  }
}

function tvchat_send_xmpp_message($authorId,$recipientUids,$body,$thread_id=0,$mid=0, $action) {
    global $user;
    global $base_root;
    global $base_url;

    $xmpp_server = substr($base_root, 7);

    $xmpp = XMPPHP_XMPP::getInstance($xmpp_server, 5222, '1', 'tvchat.123', 'teeple', $xmpp_server, false, XMPPHP_Log::LEVEL_INFO);

    if (!isset($xmpp)) {
        watchdog('xmpp', 'xmpp undefined');
    }

    $account = user_load($authorId);
    $sender_jid = $account->uid.'@'.$_SERVER['HTTP_HOST'];

    $ret_message = array();
    $ret_message['#error'] = 'false';

    foreach ($recipientUids as $uid) {

        $result = db_result(db_query("SELECT count(sid) FROM sessions WHERE uid = %d", $uid));

        // online user 
        //
        if ($result > 0) {

            $data = array();
            $account_receiver = user_load($uid);
            $receiver_jid = $account_receiver->uid.'@'.$_SERVER['HTTP_HOST'].'/teeple';

            $data['action'] = $action;
            $data['uid'] = $authorId;
            $data['nickName'] = empty($account->profile_nickname) ? null: $accont->profile_nickname;
            $data['userPicture'] = empty($account->picture) ? null: $base_url.'/'.$account->picture;
            $data['mid'] = $mid ? $mid : null;
            $data['thread_id'] = $thread_id ? $thread_id: null;
            $data['message'] = $body;

            $ret_message['#data'] = $data;

            try{
                $xmpp->message($receiver_jid, drupal_to_js($ret_message));
                watchdog( 'xmpp', 'send message to '.$receiver_jid);

            } catch(XMPPHP_Exception $e) {
                watchdog( 'xmpp', 'fail to send message. E:'.$e->getMessage());
            }
        }
    }
    return true;
}

//
//   refer to file_save_upload(in includes/file.inc )
//   tvchat_file_save_upload instead of file_save_upload (default) for base64 encoded file saving
//
function tvchat_file_save_upload($source, $validators = array(), $dest = FALSE, $replace = FILE_EXISTS_RENAME) {

  global $user;
  static $upload_cache;

  // Add in our check of the file name length
  $validators['file_validate_name_length'] = array();

  // Return cached objects without processing since the file will have
  // already been processed and the paths in _FILES will be invalid.
  if (isset($upload_cache[$source])) {
      return $upload_cache[$source];
  }

  // If a file was uploaded, process it.
  if (isset($_FILES['files']) && $_FILES['files']['name'][$source]) {

    // Check for file upload errors and return FALSE if a
    // lower level system error occurred.
    switch ($_FILES['files']['error'][$source]) {
        // @see http://php.net/manual/en/features.file-upload.errors.php
        case UPLOAD_ERR_OK:
            break;
        case UPLOAD_ERR_INI_SIZE:
        case UPLOAD_ERR_FORM_SIZE:
            drupal_set_message(t('The file %file could not be saved, because it exceeds %maxsize, the maximum allowed size for uploads.', array('%file' => $source, '%maxsize' => format_size(file_upload_max_size()))), 'error');
            return 0;
        case UPLOAD_ERR_PARTIAL:
        case UPLOAD_ERR_NO_FILE:
            drupal_set_message(t('The file %file could not be saved, because the upload did not complete.', array('%file' => $source)), 'error');
              return 0;

        // Unknown error
        default:
            drupal_set_message(t('The file %file could not be saved. An unknown error has occurred.', array('%file' => $source)), 'error');
            return 0;
    }

    $extensions = '';
    foreach ($user->roles as $rid => $name) {
      $extensions .= ' '.variable_get("upload_extensions_$rid", 
      variable_get('upload_extensions_default', 'jpg jpeg gif png txt html doc xls pdf ppt pps odt ods odp'));
    }
    
    // Begin building file object.
    $file = new stdClass();
    $file->filename = file_munge_filename(trim(basename($_FILES['files']['name'][$source]), '.'), $extensions);
    $file->filepath = $_FILES['files']['tmp_name'][$source];
    $file->filemime = file_get_mimetype($file->filename);

    // If the destination is not provided, or is not writable, then use the
    // temporary directory.
    if (empty($dest) || file_check_path($dest) === FALSE) {
      $dest = file_directory_temp();
    }

    $file->source = $source;
    $file->destination = file_destination(file_create_path($dest .'/'. $file->filename), $replace);
    $file->filesize = $_FILES['files']['size'][$source];

    // Call the validation functions.
    $errors = array();
    foreach ($validators as $function => $args) {
      array_unshift($args, $file);
      // Make sure $file is passed around by reference.
      $args[0] = &$file;

      $errors = array_merge($errors, call_user_func_array($function, $args));
    }

    // Rename potentially executable files, to help prevent exploits.
    if (preg_match('/\.(php|pl|py|cgi|asp|js)$/i', $file->filename) && (substr($file->filename, -4) != '.txt')) {
      $file->filemime = 'text/plain';
      $file->filepath .= '.txt';
      $file->filename .= '.txt';
      // As the file may be named example.php.txt, we need to munge again to
      // convert to example.php_.txt, then create the correct destination.
      $file->filename = file_munge_filename($file->filename, $extensions);
      $file->destination = file_destination(file_create_path($dest .'/'. $file->filename), $replace);
    }

    // Check for validation errors.
    if (!empty($errors)) {
      $message = t('The selected file %name could not be uploaded.', array('%name' => $file->filename));
      if (count($errors) > 1) {
        $message .= '<ul><li>'. implode('</li><li>', $errors) .'</li></ul>';
      }
      else {
        $message .= ' '. array_pop($errors);
      }
      // form_set_error($source, $message);
      // return 0;
      return -2;
    }

    // Move uploaded files from PHP's upload_tmp_dir to Drupal's temporary directory.
    // This overcomes open_basedir restrictions for future file operations.
    $file->filepath = $file->destination;

    $del_file_name = $_FILES['files']['tmp_name'][$source];
    if (!file_copy($_FILES['files']['tmp_name'][$source], $file->filepath, FILE_EXISTS_REPLACE)) {
      form_set_error($source, t('File upload error. Could not move uploaded file.'));
      watchdog('file', 'Upload error. Could not move uploaded file %file to destination %destination.', array('%file' => $file->filename, '%destination' => $file->filepath));
      return 0;
    }
    file_delete($del_file_name);

    // If we made it this far it's safe to record this file in the database.
    $file->uid = $user->uid;
    $file->status = FILE_STATUS_TEMPORARY;
    $file->timestamp = time();
    drupal_write_record('files', $file);

    // Add file to the cache.
    $upload_cache[$source] = $file;
    return $file;
  }
  return 0;
}

// file upload 
//
// : temp directory에 해당 파일을 저장한다. 이 때, 필요할 경우, 파일에 대해서 scale 조정한다.
// 
function tvchat_file_upload($fname, $attachment, $width = 0, $height = 0) {
  global $user;

  $directory_temp = file_directory_temp();
  $temp1 = tempnam($directory_temp, 'DRUPAL_');
  $del_file_name = $temp1;
  if (!$temp1) {
    drupal_set_message(t('The file could not be created.'), 'error');
    return -1;
  }

  $fp = fopen($temp1, 'w');
  fwrite($fp, base64_decode($attachment));
  fclose($fp);

  $temp_file = $directory_temp.'/'.$fname;
  $ret = null;
  if(!empty($width) && !empty($height)) {
    $ret = image_scale($temp1, $temp_file, $width, $height);
  } 
  if(!$ret) { 
    file_copy($temp1, $temp_file, FILE_EXISTS_REPLACE); 
  }
  // temp file delete
  //
  file_delete($del_file_name);

  $file = new stdClass();
  $file->filename = $fname;
  $file->description = $fname;
  $file->weight = 0;
  $file->remove = 0;
  $file->timestamp = time();
  $file->filemime = file_get_mimetype($file->filename);
  $file->filepath = $temp_file;
  $file->filesize = filesize($temp_file);

  if($file->filesize >0) { return $file; }
  return -1;
}

function views_views_pre_execute(&$view) {

 if($view->name=='query_tvchat_audience_list_1' 
     || $view->name=='query_tvchat_audience_list_2' ) {

     if(empty($view->args)) { 
         $view->build_info['query']=
             "SELECT comments.cid AS cid, comments.comment AS comment, node_comments.nid AS node_comments_nid, node_comments.language AS node_comments_language, node_comments.type AS node_comments_type, users_comments.uid AS users_comments_uid, users_comments_profile_values_profile_nickname.value AS users_comments_profile_values_profile_nickname_value, users_comments_profile_values_profile_nickname.uid AS users_comments_profile_values_profile_nickname_uid, users_comments.access AS users_comments_access, users_comments.picture AS users_comments_picture, users_comments.name AS users_comments_name, users_comments.mail AS users_comments_mail, comments.timestamp AS comments_timestamp FROM comments comments  LEFT JOIN node node_comments ON comments.nid = node_comments.nid LEFT JOIN users users_comments ON comments.uid = users_comments.uid LEFT JOIN profile_values users_comments_profile_values_profile_nickname ON users_comments.uid = users_comments_profile_values_profile_nickname.uid AND users_comments_profile_values_profile_nickname.fid = '5' WHERE comments.cid IN ( SELECT max(cid) cid FROM comments GROUP BY comments.uid ) AND users_commemnts.status = 1 ORDER BY comments_timestamp DESC";
        tvchat_log($view->build_info['query']);
     } else {

         $where[] = empty($view->args[0]) ? '': ' comments.nid IN ('.$view->args[0].')';
         if($view->name == 'query_tvchat_audience_list_2') { 
            $where[] = empty($view->args[1]) ? '':' comments.uid NOT IN ('.$view->args[1].')';
         } else {
            $where[] = empty($view->args[1]) ? '':' comments.uid IN ('.$view->args[1].')';
         }

         if(!empty($where)) {
             $where = 'WHERE '.join(' AND ', $where);
         }

         $view->build_info['query']=
             "SELECT comments.cid AS cid, comments.comment AS comment, node_comments.nid AS node_comments_nid, node_comments.language AS node_comments_language, node_comments.type AS node_comments_type, users_comments.uid AS users_comments_uid, users_comments_profile_values_profile_nickname.value AS users_comments_profile_values_profile_nickname_value, users_comments_profile_values_profile_nickname.uid AS users_comments_profile_values_profile_nickname_uid, users_comments.access AS users_comments_access, users_comments.picture AS users_comments_picture, users_comments.name AS users_comments_name, users_comments.mail AS users_comments_mail, comments.timestamp AS comments_timestamp FROM comments comments  LEFT JOIN node node_comments ON comments.nid = node_comments.nid LEFT JOIN users users_comments ON comments.uid = users_comments.uid LEFT JOIN profile_values users_comments_profile_values_profile_nickname ON users_comments.uid = users_comments_profile_values_profile_nickname.uid AND users_comments_profile_values_profile_nickname.fid = '5' WHERE comments.cid IN ( SELECT max(cid) cid FROM comments ".$where." GROUP BY comments.uid ) AND users_comments.status = 1 ORDER BY comments_timestamp DESC";
        tvchat_log($view->build_info['query']);
     }
   }
}

function get_login_status($uid) {
    $login_flag = db_result(db_query("SELECT count(sid) FROM {sessions} WHERE uid = %d", $element_uid));

    return empty($login_flag)? FALSE: TRUE;
}

function args_mapper($view_name, $view_results){
  global $base_url;
  $ret_array = array();

  switch($view_name) {
    case 'query_tvchat_audience_list_1':
    case 'query_tvchat_audience_list_2':
        foreach($view_results as $element) {
          $element_arr = (array)$element;
          $ret_array[] = array(
            // only for 1st phase 
            // 'comment' => $element_arr['comment'],
            // 'type' => $element_arr['node_comments_type'],
            // 'timestamp' => date('Y-m-d H:i',$element_arr['comments_timestamp']),
            'comment' => null,
            'type' => null,
            'timestamp' => null,
            'uid'   => $element_arr['users_comments_uid'],
            'userId' => $element_arr['users_comments_uid'],
            'userName' => $element_arr['users_comments_name'],
            'loginStatus' => get_login_status($element_arr['users_comments_uid']),
            'userNickName'=>  $element_arr['users_comments_profile_values_profile_nickname_value'],
            'access' => date('Y-m-d H:i', $element_arr['users_comments_access']),
            'userPicture' => empty($element_arr['users_comments_picture']) ? null: $base_url.'/'.$element_arr['users_comments_picture']
          );
        }
        break;

    case 'query_tvchat_ext_audience_list_1':
    case 'query_tvchat_ext_audience_list_2':
        foreach($view_results as $element) {
          $element_arr = (array)$element;
          $ret_array[] = array(
            'comment' => null,
            'type' => null, 
            'timestamp' => null,
            'uid'   => $element_arr['uid'],
            'userId' => $element_arr['uid'],
            'userName' => $element_arr['users_name'],
            'loginStatus' => get_login_status($element_arr['uid']),
            'userNickName'=>  $element_arr['profile_values_profile_nickname_value'],
            'access' => date('Y-m-d H:i', $element_arr['users_access']),
            'userPicture' => empty($element_arr['users_picture']) ? null: $base_url.'/'.$element_arr['users_picture']
          );
        }
        break;

    case 'query_ur_receive_json':
        foreach($view_results as $element) {
            $element_arr = (array)$element;
            $ret_array[] = array(
                'rid'           => $element_arr['rid'],
                'requesterUid'  => $element_arr['users_user_relationships_uid']
            );
        }
        break;

    case 'query_ur_request_json':
        foreach($view_results as $element) {
            $element_arr = (array)$element;
            $ret_array[] = array(
                'rid'           => $element_arr['rid'],
                'requesteeUid'  => $element_arr['users_user_relationships_uid']
            );
        }
        break;

    default:
      break;
  }
  return $ret_array;
}

function tvchat_user_save_profile(&$edit, &$user, $category, $register = FALSE) { 
    $result = _profile_get_fields($category, $register); 
    while ($field = db_fetch_object($result)) { 
        if(isset($edit[$field->name])){ // this prevent from deleting other stored fields 
            if (_profile_field_serialize($field->type)) { 
                $edit[$field->name] = serialize($edit[$field->name]); 
            } 
            db_query("DELETE FROM {profile_values} WHERE fid = %d AND uid = %d", $field->fid, $user->uid); 
            db_query("INSERT INTO {profile_values} (fid, uid, value) VALUES (%d, %d, '%s')", $field->fid, $user->uid, $edit[$field->name]); 
        } 
        // Mark field as handled (prevents saving to user->data). 
        $edit[$field->name] = NULL; 
    } 
} 

function tvchat_log($err_message){

    global $user;

    $usernames_for_tracking = explode(',',variable_get('usernames_for_tracking', 'youngmin@uangel.kr'));
    foreach($usernames_for_tracking as $username) {
        if($username == $user->name) {
            watchdog($username, $err_message);
        }
    }
}

function tvchat_views_embed_form() {
    return array(
    // Key in this array is the name of a form and also the name of a form function.
        'tvchat_views_form' => t('Form for tvchat module'),
    );
}


function tvchat_views_form(&$form_state, $row) {
    $form = array();
    $form['new_nid'] = array(
        '#type' => 'textfield',
        '#title' => t('changed nid'),
        '#size' => 10,
        '#maxlength' => 10,
        '#description' => t('changed nid'),
    );
    $form['nid'] = array(
        '#type' => 'hidden',
        '#value' => intval($row->nid),
    );
    $form['target_date'] = array(
        '#type' => 'hidden',
        '#value' => $row->tvchat_ranking_weekly_target_date,
    );
    $form['submit'] = array(
        '#type' => 'submit',
        '#name' => 'submit' . intval($row->nid),
        '#value' => 'Click me!',
    );
    return $form;
}

function tvchat_views_form_submit($form, &$form_state) {

    $_query = sprintf("UPDATE tvchat_ranking_weekly SET entity_nid = %d WHERE entity_nid = %d AND target_date ='%s'", $form_state['values']['new_nid'], $form_state['values']['nid'], $form_state['values']['target_date']);
    db_query($_query);
    drupal_set_message(t('You clicked a Views Embed Form Test for Node #@nid', array('@nid' => $form_state['values']['nid'])));
}

function tvchat_send_message($authorId=0, $recipientUids, $message, $badge=0) {

    $senderId = $authorId == 0 ? 1 : $authorId;

    $timestamp = time();

    foreach($recipientUids as $uid) {
        $account = user_load($uid);

        if(!empty($account->profile_APNS_token)) {
            $table_name = 'apns_messages';
            $device_token = $account->profile_APNS_token;

        } else if (!empty($account->profile_C2DM_token)) {
            $table_name = 'c2dm_messages';
            $device_token = $account->profile_C2DM_token;
        }

        $element = array(
            'sender' => $senderId,
            'receiver' => $uid,
            'device_token' => $device_token,
            'message' => $message,
            'badge' => $badge,
            'delivery' => $timestamp,
            'created' => $timestamp,
            'modified' => $timestamp
        );
        error_log('table_name: '.$table_name);
        error_log('device_token: '.$device_token);

        drupal_write_record($table_name, $element);
    }
    return true;
}

function tvchat_form() {

    return drupal_get_form('tvchat_my_form');
}

function tvchat_my_form($form_state) {

    $form['message'] = array(
        '#type' => 'textfield',
        '#title' => t('message'),
        '#required' => TRUE,
        '#default_value' => '',
        '#size' => 130,
        '#max_length' => 130
    );
    $form['submit'] = array(
        '#type' => 'submit',
        '#name' => 'submit_bulk_message',
        '#value' => '보내기',
    );
    return $form;
}

function tvchat_my_form_submit($form, $form_state) {
    module_load_include('inc', 'views_service');

    // $ret = views_service_get('admin_user_page', 'list_all_users_page', null, 0, 0, FALSE);

    rules_invoke_event('push_bulk_message', $form_state['values']['message'], $recipient_uids);
}
