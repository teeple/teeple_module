<?php
// $Id$

/**
 * @file
 * Contains administrative pages for creating, editing, and deleting flags.
 */

/**
 * Flag administration page. Display a list of existing flags.
 */
function tvchat_admin_page() {
  $flags = array();
  $default_flags = array();
  return theme('tvchat_admin_page', $flags, $default_flags);
}

/**
 * Theme the output for the main flag administration page.
 */
function theme_tvchat_admin_page($flags, $default_flags) 
{
  $output = '<p>' . t('TV Chat 프로그램에 대한 각종 설정을 관리합니다.').'</p>';

  $output .= drupal_get_form( 'tvchat_config_form');
  return $output;
}

/*
	form menu for check-in service
*/
function tvchat_config_form()
{
  $config = _tvchat_query();

  /* check in */
  $form['checkin'] = array(
    '#type' => 'fieldset',
		'#title' => t('Check In'),
		'#description' => t('Check In 기능에 대한 설정'),
  );

  $form['checkin']['max_checkin_day'] = array(
    '#type' => 'textfield',
		'#title' => t('하루 최대 Check In 회수 '),
		'#weight' => 0,
		'#size' => 10,
		'#maxlength' => 3,
		'#default_value' => $config['max_checkin_day'],
		'#description' => t('하루에 최대로 Check In 할 수 있는 회수'),
  );

  $form['checkin']['reset_checkin'] = array(
    '#type' => 'checkbox',
		'#title' => t('어제까지의 체크인 내역 초기화'),
		'#weight' => 1,
		'#default_value' => 0,
		'#description' => t('어제까지의 모든 체크인 내역을 초기화한다.'),
  );

  $form['checkin']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
		'#weight' => 2,
  );

	/* TV Show */
  $form['tvshow'] = array(
    '#type' => 'fieldset',
		'#title' => t('TV 프로그램 등수 산정 가중치'),
		'#description' => t('각 TV 프로그램의 등수 산정을 위한 가중치'),
  );

  $form['tvshow']['checkin_count'] = array(
    '#type' => 'textfield',
		'#title' => t('Check In 회수'),
		'#size' => 10,
		'#maxlength' => 3,
		'#default_value' => $config['checkin_count'],
		'#description' => t('Check In 회수에 대한 가중치'),
  );

  $form['tvshow']['favorite_count'] = array(
    '#type' => 'textfield',
		'#title' => t('Follow 사용자 수'),
		'#size' => 10,
		'#maxlength' => 3,
		'#default_value' => $config['favorite_count'],
		'#description' => t('Follow 사용자수에 대한 가중치'),
  );

  $form['tvshow']['comment_count'] = array(
    '#type' => 'textfield',
		'#title' => t('Comment 사용자 수'),
		'#size' => 10,
		'#maxlength' => 3,
		'#default_value' => $config['comment_count'],
		'#description' => t('Comment 사용자수에 대한 가중치'),
  );

  $form['tvshow']['rate_now'] = array(
    '#type' => 'checkbox',
		'#title' => t('TV 프로그램 등수 계산'),
		'#default_value' => 0,
		'#description' => t('어제까지의 사용자 활동 내역을 바탕으로 TV 프로그램 등수를 계산한다.'),
  );


  $form['tvshow']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
  );

	/* TV Episode */
  $form['episode'] = array(
    '#type' => 'fieldset',
		'#title' => t('TV 프로그램 에피소드 관리'),
		'#description' => t('각 TV 프로그램의 에피소드에 대한 설정 관리'),
  );

  $form['episode']['create_episode'] = array(
    '#type' => 'checkbox',
		'#title' => t('오늘 방송될 에피소드 생성'),
		'#default_value' => 0,
		'#description' => t('오늘 방송될 에피소드를 생성한다.'),
  );

  $form['episode']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
  );

	/* TV Chat Configuration */
  $form['environ'] = array(
    '#type' => 'fieldset',
		'#title' => t('Teeple 환경 설정 관리'),
		'#description' => t('각 TV 프로그램의 에피소드에 대한 설정 관리'),
  );

  $form['environ']['dir_timetable'] = array(
    '#type' => 'textfield',
		'#title' => t('TimeTable Directory'),
		'#default_value' => '/var/www/data/timetable',
		'#description' => t('편성표가 저장되는 장소이다.'),
  );

  $form['environ']['max_top_show'] = array(
        '#type' => 'textfield',
		'#title' => t('인기 프로그램 수'),
		'#size' => 5,
		'#maxlength' => 3,
		'#default_value' => 5,
		'#description' => t('매일 변경되는 인기 프로그램의 갯수'),
  );

  $form['environ']['url_intro_shorten'] = array(
    '#type' => 'textfield',
		'#title' => t('Introduction Page URL (shorten)'),
		'#default_value' => 'http://goo.gl/VL6Ww',
		'#description' => t('teeple의 소개페이지 URL이다.'),
  );

  $form['environ']['apns_request_url'] = array(
    '#type' => 'textfield',
		'#title' => t('APNS REQUEST URL '),
		'#default_value' => 'http://newbiz.uangel.com/tvchat/ApnsPHP/sample_push.php',
		'#description' => t('teeple의 APNS request URL이다.'),
  );

  $form['environ']['xmpp_send_cmd'] = array(
    '#type' => 'textfield',
		'#title' => t('XMPP Send Command'),
		'#default_value' => 'echo newbiz.123 | sudo -S /usr/sbin/ejabberdctl send_message_chat %s %s \'%s\'',
		'#description' => t('teeple의 APNS request URL이다.'),
  );

  $form['environ']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
  );

  /* SMS Link Configuration */
  $form['smsgw'] = array(
    '#type' => 'fieldset',
		'#title' => t('Teeple SMS 링크 설정 관리'),
		'#description' => t('Teeple SMS 링크에 대한 설정 관리'),
  );

  $form['smsgw']['cmp_usr_id'] = array(
    '#type' => 'textfield',
		'#title' => t('CMP USER ID'),
		'#default_value' => '21105',
		'#description' => t('SMS링크 전달 Unique ID.'),
  );

  $form['smsgw']['snd_phn_id'] = array(
    '#type' => 'textfield',
		'#title' => t('SMS 전송자 폰 번호'),
		'#default_value' => '0317103953',
		'#description' => t('SMS 전송자 폰 번호임.'),
  );

  $form['smsgw']['callback'] = array(
    '#type' => 'textfield',
		'#title' => t('SMS 전송자 번호 - phone에 표시됨'),
		'#default_value' => '0317103953',
		'#description' => t('SMS 전송자 번호 - 받는 사람 폰에 표시되는 번호'),
  );

  $form['smsgw']['auth_msg'] = array(
    '#type' => 'textfield',
		'#title' => t('인증 번호 전송 메시지'),
		'#default_value' => '귀하의 인증 번호는 [%s]입니다.',
		'#description' => t('인증 번호 전송 메시지'),
  );

  $form['smsgw']['invite_msg'] = array(
    '#type' => 'textfield',
		'#title' => t('서비스 초대 메시지'),
		'#default_value' => '친구 %s(%s)분이 Teeple서비스(%s)에 초대하셨습니다.',
		'#description' => t('서비스 초대 메시지'),
  );

  $form['smsgw']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
  );

  /* Tracking Configuration */
  $form['tracking'] = array(
    '#type' => 'fieldset',
		'#title' => t('Teeple Tracking을 위한 설정 관리'),
		'#description' => t('Teeple Service Tracking을 위한 설정 관리'),
  );

  $form['tracking']['usernames_for_tracking'] = array(
    '#type' => 'textfield',
		'#title' => t('User names for tracking '),
		'#default_value' => 'youngmin.k@gmail.com, taiyen@nate.com, neoroman@uangel.com, neoroman@paran.com, casey_lee@uangel.com, kclee0112@yahoo.co.kr',
		'#description' => t('User names for tracking.'),
  );

  $form['tracking']['submit'] = array(
    '#type' => 'submit',
	  '#value' => t('Save'),
  );

  return $form;
}


/*
	handle request to save 'check-in' configuration
*/
function tvchat_config_form_submit( $form, &$form_state)
{
	$config = array(
		'max_checkin_day'   => $form_state['values']['max_checkin_day'],
		'reset_checkin'     => $form_state['values']['reset_checkin'],
		'checkin_count'     => $form_state['values']['checkin_count'],
		'favorite_count'    => $form_state['values']['favorite_count'],
		'comment_count'     => $form_state['values']['comment_count'],
		'rate_now'          => $form_state['values']['rate_now'],
		'create_episode'    => $form_state['values']['create_episode'],
		'dir_timetable'     => $form_state['values']['dir_timetable'],
		'apns_request_url'  => $form_state['values']['apns_request_url'],
		'xmpp_send_cmd'     => $form_state['values']['xmpp_send_cmd'],
		'url_intro_shorten' => $form_state['values']['url_intro_shorten'],
		'max_top_show'      => $form_state['values']['max_top_show'],
		'cmp_usr_id'        => $form_state['values']['cmp_usr_id'],
		'snd_phn_id'        => $form_state['values']['snd_phn_id'],
		'callback'          => $form_state['values']['callback'],
		'auth_msg'          => $form_state['values']['auth_msg'],
		'invite_msg'        => $form_state['values']['invite_msg'],
		'usernames_for_tracking' => $form_state['values']['usernames_for_tracking'],
	);

//	drupal_set_message( print_r( $form_state, TRUE).'\n'.print_r( $config, TRUE));

  $result = _tvchat_update( $config);
  if ( $result) {
    drupal_set_message( t('Configuration updated successfully.'));
  }
  else {
    drupal_set_message( t('Failed to update configuration.'));
  }
}


/*
	TODO
	1) validate form values
	2) reconsider schema
*/

/* 
	DB APIs
*/
function _tvchat_update( $config)
{
	foreach($config as $key => $value) {

        switch ($key) {
            case 'reset_checkin':
                if ( $value == '1')  _tvchat_reset_checkin();
                continue;

            case 'rate_now':
                if ( $value == '1') _tvchat_rate_now();
                continue;

            case 'create_episode':
                if ( $value == '1') _tvchat_create_episode();
                continue;

            case 'dir_timetable':
                if (isset($value)) { variable_set($key, $value); }
                break;

            case 'xmpp_send_cmd':
                if (isset($value)) { variable_set($key, $value); }
                break;

            case 'apns_request_url':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'url_intro_shorten':
                if (isset($value)) { variable_set($key, $value); }
                break;

            case 'max_top_show':
                if (isset($value)) { variable_set($key, $value); }
                break;

            case 'cmp_usr_id':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'snd_phn_id':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'callback':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'auth_msg':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'invite_msg':
                if (isset($value)) { variable_set($key, $value); }
                break;
            case 'usernames_for_tracking':
                if (isset($value)) { variable_set($key, $value); }
                break;
            default:
                break;
        }

		$result = db_query( "SELECT count(*) FROM tvchat_config WHERE keystr='%s'", $key);
		if ( db_result( $result) == 0) {
			db_query("INSERT INTO tvchat_config( keystr, valuestr) VALUES( '%s', '%s') ", $key, $value);
			if ( db_affected_rows() == 1)
				drupal_set_message( t('insert new record ('.$key.','.$value.')'));
		}
		else {
			db_query("UPDATE tvchat_config SET valuestr = '%s' WHERE keystr='%s'", $value, $key);
			if ( db_affected_rows() >= 1)
				drupal_set_message( t('update record ('.$key.','.$value.')'));
		}
	}

  return TRUE;
}

function _tvchat_query()
{
  $obj = array();
  $result = db_query( "SELECT keystr, valuestr FROM tvchat_config");
  while( $row = db_fetch_object( $result)) {
    $obj[$row->keystr] = $row->valuestr;
  }

  return $obj;
}

function _tvchat_reset_checkin()
{
	if (($flag = flag_get_flag( 'checkin'))) {
		$yesterday_timestamp = mktime( 0, 0, 0) -1;	// 23:59:59 of yesterday

		// remove all check history
		$sql = sprintf( 'DELETE FROM {flag_content} WHERE fid=%d AND timestamp <= %d',
			$flag->fid, $yesterday_timestamp);
		db_query( $sql);

	  drupal_set_message( t( sprintf('SQL "%s", affected_rows=%d', $sql, db_affected_rows())));
	}
}

/*
	rate tvshow by check-in, favorite, follower count
*/
function _tvchat_rate_now()
{
	$yesterday_timestamp = mktime( 0, 0, 0) -1;	// 23:59:59 of yesterday
	$date_str = date( 'Y-m-d', $yesterday_timestamp);
	$config = _tvchat_query();

	/* check-in count */
	if (($flag = flag_get_flag( 'checkin'))) {
		// count the number of check-in of yesterday
		$result = db_query( 'SELECT content_id, count(content_id) as "count"'
			.' FROM {flag_content} '
			.' WHERE fid=%d AND timestamp <= %d AND timestamp > %d'
			.' GROUP BY content_id', $flag->fid, $yesterday_timestamp, $yesterday_timestamp - 3600*24);

		$sql = "";

		while( $row = db_fetch_array( $result)) {

			if ( $row['count'] == 0) continue;

			if ( db_result( db_query( "SELECT count(*) FROM tvchat_rating WHERE date='%s' AND content_id=%d",
				$date_str, $row['content_id'])) == 0) {
				// record not found. insert new one
				$sql = sprintf( "INSERT INTO tvchat_rating( date, content_id, checkin_count) VALUES( '%s', '%s', '%s')", 
					$date_str, $row['content_id'], $row['count']);
			}
			else {
				// update records
				$sql = sprintf( "UPDATE tvchat_rating SET checkin_count = %d WHERE date='%s' AND content_id=%d",
					$row['count'], $date_str, $row['content_id']);
			
			}

			db_query( $sql);
			drupal_set_message( t( sprintf('SQL "%s", affected_rows=%d', $sql, db_affected_rows())));
		}
  }


	/* favorite count */
	if (($flag = flag_get_flag( 'favorite'))) {

		// count the number of check-in of yesterday
		$result = db_query( 'SELECT content_id, count(content_id) as "count"'
			.' FROM {flag_content} '
			.' WHERE fid=%d AND timestamp <= %d AND timestamp > %d'
			.' GROUP BY content_id', $flag->fid, $yesterday_timestamp, $yesterday_timestamp - 3600*24);

		$sql = "";

		while( $row = db_fetch_array( $result)) {

			if ( $row['count'] == 0) continue;

			if ( db_result( db_query( "SELECT count(*) FROM tvchat_rating WHERE date='%s' AND content_id=%d",
				$date_str, $row['content_id'])) == 0) {
				// record not found. insert new one
				$sql = sprintf( "INSERT INTO tvchat_rating( date, content_id, favorite_count) VALUES( '%s', '%s', '%s')", 
					$date_str, $row['content_id'], $row['count']);
			}
			else {
				// update records
				$sql = sprintf( "UPDATE tvchat_rating SET favorite_count = %d WHERE date='%s' AND content_id=%d",
					$row['count'], $date_str, $row['content_id']);
			
			}

			db_query( $sql);
			drupal_set_message( t( sprintf('SQL "%s", affected_rows=%d', $sql, db_affected_rows())));
		}
  }

  /* weekly rating */
	$week_ago = date( 'Y-m-d', $yesterday_timestamp - 3600*24*7);

  $sql = sprintf( 'SELECT content_id, sum( checkin_count) AS "checkin", sum( favorite_count) AS "favorite", sum( comment_count) AS "comment" '
	  .' FROM tvchat_rating WHERE date > "%s" GROUP BY content_id ', $week_ago);
  $result = db_query( $sql);

	while( $row = db_fetch_array( $result)) {
		$rate = _tvchat_calc_rate( $config, $row);
		$node = node_load( $row['content_id'], NULL, TRUE);
		$node->field_weekly_score[0]['value'] = $rate;
		node_save( $node);
		drupal_set_message( print_r( $node, TRUE));
		/*
	  $sql = sprintf( 'UPDATE tvchat_ranking SET weekly_rating = %d WHERE content_id = %d',
		  $rate, $row['content_id']);
    db_query( $sql);

		if ( db_affected_rows() == 0) {
		  // insert record
			$sql = sprintf( 'INSERT INTO tvchat_ranking( content_id, weekly_rating) VALUES ( %d, %d)', $row['content_id'], $rate);
			db_query( $sql);
		}

		drupal_set_message( t( sprintf( 'SQL : %s. affected_rows=%d', $sql, db_affected_rows())));
		*/
	}

	$month_ago = date( 'Y-m-d', $yesterday_timestamp - 3600*24*30);

  $sql = sprintf( 'SELECT content_id, sum( checkin_count) AS "checkin", sum( favorite_count) AS "favorite", sum( comment_count) AS "comment" '
	  .' FROM tvchat_rating WHERE date > "%s" GROUP BY content_id ', $month_ago);
  $result = db_query( $sql);

	while( $row = db_fetch_array( $result)) {
		$rate = _tvchat_calc_rate( $config, $row);
		$node = node_load( $row['content_id'], NULL, TRUE);
		$node->field_monthly_score[0]['value'] = $rate;
		node_save( $node);
		//drupal_set_message( print_r( $node, TRUE));
		/*
	  $sql = sprintf( 'UPDATE tvchat_ranking SET monthly_rating = %d WHERE content_id = %d',
		  $rate, $row['content_id']);
    db_query( $sql);

		if ( db_affected_rows() == 0) {
		  // insert record
			$sql = sprintf( 'INSERT INTO tvchat_ranking( content_id, monthly_rating) VALUES ( %d, %d)', $row['content_id'], $rate);
			db_query( $sql);
		}

		drupal_set_message( t( sprintf( 'SQL : %s. affected_rows=%d', $sql, db_affected_rows())));
		*/
	}
}


function _tvchat_calc_rate( $config, $row)
{
  return $row['checkin'] * $config['checkin_count'] + $row['favorite'] * $config['favorite_count'] + $row['comment'] * $config['comment_count'];
}

function _tvchat_create_episode()
{
	$today = date( 'Y-m-d');
	$today_strlen = strlen( $today);

	$result = db_query( 'SELECT nid FROM {node} WHERE type = "tvshow"');
	while( $row = db_fetch_object( $result)) {
	  $node = node_load( $row->nid);
//		drupal_set_message( print_r( $node, TRUE));

		/* check date to start and end play */
		if ( isset( $node->field_start_date[0]['value']) && $node->field_start_date[0]['value'] > $today)
		  continue;
			
		if ( isset( $node->field_end_date[0]['value']) && $node->field_end_date[0]['value'] < $today)
		  continue;

		/* check tv episode created */
		$episode = db_fetch_object( db_query( sprintf( 'SELECT nid, field_episode_num_value FROM content_type_tv_episode '
		  .' WHERE field_tvshow_nid = %d ORDER BY field_episode_num_value DESC LIMIT 0, 1', $row->nid)));
		if ( isset( $episode->nid)) {
		  $episode_node = node_load( $episode->nid);
			if ( substr( $episode_node->field_start_date[0]['value'], 0, $today_strlen) < $today) {
			  // create new episode
				$new_episode = _tvchat_create_new_episode( $node, $episode_node->field_episode_num[0]['value'] +1, $today);

				drupal_set_message( 'create '.$new_episode->title.$episode->nid);
				drupal_set_message( 'episode '.print_r( $episode_node, TRUE));
			}
		}
		else {
			// create the first episode
			$new_episode = _tvchat_create_new_episode( $node, 1, $today);

			drupal_set_message( 'insert '.$new_episode->title);
		}
	}
}


function _tvchat_create_new_episode( $tvshow, $episode_num, $date_str)
{
	$newnode = new stdClass();
	$newnode->title = $tvshow->title." 제 ".$episode_num." 회";
	$newnode->body = $newnode->title;

	$newnode->uid = $tvshow->uid;
	$newnode->type = 'tv_episode';
	$newnode->status = 1;
	$newnode->promote = 0;
	$newnode->comment = 2;	// enable comment

	$newnode->field_start_date = $tvshow->field_start_date;
	$newnode->field_start_date[0]['value'] = $date_str.substr( $tvshow->field_start_date[0]['value'], strlen( $date_str));
	$newnode->field_start_time = $tvshow->field_start_time;
	$newnode->field_end_time = $tvshow->field_end_time;
	$newnode->field_episode_num = array( array( 'value' => $episode_num));
	$newnode->field_tvshow = array( array( 'nid' => $tvshow->nid));

	node_save( node_submit($newnode));

	return $newnode;
}
