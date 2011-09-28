<?php
// $Id: user_service.inc,v 1.1.2.8.2.12 2010/10/26 23:54:21 ocyrus Exp $

/**
 * 
 *
 * This can create user depending on the
 * information passed to the function. This function uses drupal_execute()
 * and as such exepects all input to match the submitting form in question.
 *
 * @param $nick.
 *    string
 *    user name and nick name
 * @param $pass.
 *    string
 *    user password. md5 encoded.
 * @param $email
 *    string
 *    user email address
 * @param $phone_number
 *    string
 *    user phone number
 * @param $fname
 *    string
 *    user picture filename
 * @param $picture
 *    strinuser->uid
 *    user picture - base64 encoded
 * @param $token_APNS
 *    string
 *    APNS_token number
 *
 * @return
 *   The UID of the user whose information was updated.
 */
function anonymous_tvchat_service_signup($nick, $email, $phone_number, $fname=null, $picture=null, $token_APNS=null, $status=1, $token_C2DM=null) {

  module_load_include('inc', 'user_tvchat_service');

  global $user;
  $status = isset($status)? $status: 1;

  if(!empty($picture) || !empty($_POST['picture'])) {
      $picture = str_replace('"', '', $_POST['picture']);
  }

  if(!valid_email_address($email)) {
      return services_error(t('The e-mail address %mail is not valid.', array('%mail' => $email)), 501);
  }

  $ret_array = array();

  $uid_by_email = db_result(db_query("SELECT uid FROM {users} WHERE LOWER(mail) = LOWER('%s')", $email));
  $uid_by_phone = db_result(db_query("SELECT uid FROM {profile_values} WHERE fid=14 AND value='%s'", $phone_number));
  $uid_by_nick = db_result(db_query("SELECT uid FROM {profile_values} WHERE fid=5 AND value='%s'", $nick));

  // 이메일이 기존에 있을 경우 
  //
  if (!empty($uid_by_email)) {

    $account = user_load($uid_by_email);
    $account = _account_reset($uid_by_email, $email);

    $password = null;

  // nickname이 기존에 있을 경우, 
  // 
  } else if (!empty($uid_by_nick)) {
      return services_error(t('Nickname is already taken.'), 10002);


  } else {

    if(!empty($uid_by_phone)) {
        $account = user_load($uid_by_phone);

        // 이미 있으나, status = 0 (블록되어 있었다면)
        if($account->status != 0) { $account = null; }

    } else {
        $account = null;
    }

    $password = user_password();

    $edit = array(
        'status' => $status,
        'name'   => $email,
        'mail'   => $email,
        'pass'   => $password
    );

    // if $account == null -> user create
    // else user update
    // 
    $new_user = user_save($account, $edit, null);

    if(!empty($fname) && !empty($picture)) {
        $ret = _user_save_picture($new_user->uid, $fname, $picture);

        /* 이미지 업로드 실패시 삭제 처리는 당분간 하지 않도록 함.
        if($ret < 0) {
            user_delete($user->uid);
            return services_error(t("User Picture Can't be save"), 502);
        }
        */
    }

    $account = user_load($new_user->uid);
    profile_load_profile(&$account);
    $edit = array(
        'profile_nickname' => $nick,
        'profile_APNS_token' => $token_APNS,
        'profile_phone_no' => $phone_number,
        'profile_message_alarm' => 1,
        'profile_tvshow_alarm' => 1,
        'profile_C2DM_token' => $token_C2DM,
    );
    tvchat_user_save_profile($edit, $account, 'Personal information');
  }

  $ret_array = array(
    'uid' => $account->uid,
    'name' => $account->name,
    'pass' => $password,
    'jid'  => $account->uid
  );

  return $ret_array;
}

function anonymous_tvchat_service_login($name, $password, $phone_number=null) {
  // error_log('anonymous_tvchat_service_login');
  global $user;

  // watchdog('login try', 'LOGGED IN: '.$name.' Password: '.$password);
  // watchdog('login try', 'USER ID  : '.(string)$user->uid);

  // $sessid = db_result(db_query('SELECT sid FROM sessions WHERE uid = %d', $user->uid));
  // watchdog('login try', 'SESSION ID: '.$sessid);

  /*
  if ($user->uid) { return services_error(t('Already logged in as !user.', array('!user' => $user->name)), 406);
 }
  */

 $user = user_authenticate(array('name' => $name, 'pass' => $password));

 if ($user->uid) {

    // one session id for one user
    // youngmin, modified 
    // 
    sess_destroy_uid($user->uid);

    // Regenerate the session ID to prevent against session fixation attacks.
    sess_regenerate();
    $array = array();
    user_module_invoke('login', $array, $user);

    $return = new stdClass();
    $return->sessid = session_id();
    $return->user = $user;

    // Profile phone_number update
    // 
    if (isset($phone_number)) {
      $edit = array();
      if ($phone_number) { $edit['profile_phone_no'] = $phone_number; }
        tvchat_user_save_profile($edit, $user, 'Personal information');
    }

    return $return;
 }

 if(isset($user->sid)) {
    session_destroy();
 }

 // return services_error(t('Wrong username or password. '), 401);
  return services_error(t('Wrong username or password. '), 401);
}

// 친구 또는 일반인들 해당 프로그램 관련자 + others 를 리턴한다.
// 
function anonymous_tvchat_service_audience_list_org($showId,$friend_flag=0,$offset=0,$limit=20) {

  module_load_include('inc', 'views_service');
  module_load_include('inc', 'tvchat');
  module_load_include('inc', 'basic_tvchat_service');

  try{
    $limit = isset($limit) ? $limit: 20;

    global $user;

    error_log('anonymous_tvchat_service_audience_list'.$friend_flag.' uid: '.$user->uid);

    if($user->uid ==0) {
        return services_error(t('Access denied'), 401);
    }

    global $base_url;

    $ret_array = array();
    $ret_element = array();

    $time_interval = variable_get('user_block_seconds_online', 900);
    $curTime = time();

    $index = 0; 
    $found = 0;
    $flag_stop = false;

    // fetch all comments from show related 
    // 쇼와 관련 이슈들에 대한 node id 리스트
    //
    $issue_id_list = tvchat_issue_nids($showId);
    $issue_id_list[] = $showId;

    $friends_uid_list = tvchat_friends_uid_list();

    // 친구 또는 일반인들의 리스트를 구하는 query를 생성한다.
    // 
    if($friend_flag == 1) {
        $view_name = 'query_tvchat_audience_list_1';

    } else {
        $view_name = 'query_tvchat_audience_list_2';
        $friends_uid_list[] = $user->uid;
    }
    $args = array();
    $args[] = join(',', $issue_id_list);
    $args[] = join(',', $friends_uid_list);

    // get audience list from views 
    // 프로그램 관련 comment한 자를 구한다.
    // 
    $view_name = ($friend_flag == 1) ? 'query_tvchat_audience_list_1' : 'query_tvchat_audience_list_2';

    $ret = views_service_get($view_name, null, $args, $offset, $limit, FALSE);
    $audience_list = args_mapper($view_name, $ret);
    $audience_uid_list = array_value_recursive('userId',$audience_list);

    // audience 수가 아직 모자랄 때,
    // 
    if( (count($audience_list) < $limit) || $limit == 0) {

        $args = array();
        $found_count = count($audience_list);

        if($friend_flag ==1) {
            $view_name = 'query_tvchat_ext_audience_list_1';
            $add_uid_list = array_diff($friends_uid_list, $audience_uid_list);

        } else {
            $view_name = 'query_tvchat_ext_audience_list_2';
            $merged_uid_list = array_merge($audience_uid_list, $friends_uid_list);

            $add_uid_list = array_unique($merged_uid_list);
        }

        $args[] = join(',', $add_uid_list);
        $new_offset = $offset > $count ? $offset - $count : 0;
        $new_limit = ($limit == 0) ? 0: $limit - $found_count;

        $ret = views_service_get($view_name, null, $args, $offset, $new_limit, FALSE);
        $extra_audience_list = args_mapper($view_name, $ret);
    }

    if(!empty($extra_audience_list)) {
        $audience_list = array_merge($audience_list, $extra_audience_list);
    }

    if(empty($audience_list)) {
        return services_error(t('No Data Found'), 511);
    }
    return $audience_list;

   } catch (Exception $ex) {
    return services_errror(t('Error getting friend_list: $msg', array('@msg' => $ex->getMessage())));
   }
}



// 친구 또는 일반인들 해당 프로그램 관련자 + others 를 리턴한다.
// 
function anonymous_tvchat_service_audience_list($showId,$friend_flag=0,$offset=0,$limit=20) {

  module_load_include('inc', 'views_service');
  module_load_include('inc', 'tvchat');
  module_load_include('inc', 'basic_tvchat_service');

  try{
    $limit = isset($limit) ? $limit: 20;

    global $user;

    error_log('anonymous_tvchat_service_audience_list'.$friend_flag.' uid: '.$user->uid);

    if($user->uid ==0) {
        return services_error(t('Access denied'), 401);
    }

    global $base_url;

    $ret_array = array();
    $ret_element = array();

    $curTime = time();

    $index = 0; 
    $found = 0;
    $flag_stop = false;

    $friends_uid_list = tvchat_friends_uid_list();

    $args = array();
    $args_uid_list = array();
    $teeple_uid = 1;

    if($friend_flag ==1) {
        $view_name = 'query_tvchat_ext_audience_list_1';
        $args_uid_list = array_diff($friends_uid_list, (array)$teeple_uid);
    } else {
        $view_name = 'query_tvchat_ext_audience_list_2';
        $args_uid_list = $friends_uid_list;
        $args_uid_list[]  = $teeple_uid;
        $args_uid_list[]  = $user->uid;
    }

    $args[] = join(',', $args_uid_list);
    $ret = views_service_get($view_name, null, $args, $offset, $limit, FALSE);
    $audience_list = args_mapper($view_name, $ret);

    if(empty($audience_list)) {
        return services_error(t('No Data Found'), 511);
    }
    return $audience_list;

   } catch (Exception $ex) {
    return services_errror(t('Error getting friend_list: $msg', array('@msg' => $ex->getMessage())));
   }
}

function anonymous_tvchat_service_anonymous_list($offset=0, $limit=40){
  module_load_include('inc', 'views_service');

  $limit = isset($limit) ? $limit: 40;

  $view_name = 'list_top_show_json';
  $ret = views_service_get($view_name, null, null, null, null, FALSE);

  $audience_list = array();

  foreach($ret as $element) {
    $audience_list[] = tvchat_service_audience_list($element->nid,0,0,$limit);
  }
  return $audience_list;
}

