<?php
// $Id: user_service.inc,v 1.1.2.8.2.12 2010/10/26 23:54:21 ocyrus Exp $

/**
 * @file
 *  Link general user functionalities to services module.
 */
 // 사용자 profile 사진을 해당 디렉토리에 저장한다.
 // 
function _user_tvchat_validate_picture($fname, $attachment, $uid) {

  // If required, validate the uploaded picture.
  $validators = array(
    'file_validate_is_image' => array(),
    // 'file_validate_image_resolution' => array(variable_get('user_picture_dimensions', '85x85')),
    // 'file_validate_size' => array(variable_get('user_picture_file_size', '30') * 1024),
   );

  // save to temp directory 
  $file = tvchat_file_upload($fname, $attachment, 85, 85);

  // save to files table and files folder 
  // 
  if(!empty($file)) {
    $_FILES['files']['name']['user_picture_upload'] = $file->filename;
    $_FILES['files']['type']['user_picture_upload'] = $file->filemime;
    $_FILES['files']['tmp_name']['user_picture_upload'] = $file->filepath;
    $_FILES['files']['error']['user_picture_upload'] = 0;
    $_FILES['files']['size']['user_picture_upload'] = $file->filesize;

    $file = tvchat_file_save_upload('user_picture_upload', $validators, file_directory_path() . $dir);

    if (isset($file) && ($file != -2)){
        $file->list = 1;
        $file->description = $file->filename;
        $file->weight = 0;
        $file->new = TRUE;
    }
  }

  if (isset($file) && $file != -2) {
    $info = image_get_info($file->filepath);
    $destination = variable_get('user_picture_path', 'pictures').'/picture-'.$uid.'.'.$info['extension'];
    if (file_copy($file, $destination, FILE_EXISTS_REPLACE)) {
        return $file;
    }
    else {
        return -2;
        // service_error(t('File Save Error'), 500);
    }
  } 
  return -2;
}

// 사용자 profile 사진을 해당 디렉토리에 저장하고, 이를 database에 업데이트 한다.
//
function _user_save_picture($uid, $fname, $picture) {

    if (!empty($fname) && !empty($picture)) {
       $file = _user_tvchat_validate_picture($fname,$picture, $uid);

       if($file == -1) {
            return -2;
            // return services_error(t('User Picture Save Error'), 405);
       } else {

            $update_set = sprintf("picture = '%s'", $file->filepath);
            $_query = sprintf("UPDATE {users} SET picture = '%s' WHERE uid = %d", $file->filepath, $uid);
            db_query ($_query);
        }
    }
}

function _account_reset($uid, $email) {

  tvchat_log('account_reset uid:'.$uid.' email:'.$email);

  $account = user_load($uid);
  $ret_array = array();
  // $password_tmp = user_password();
  $password_tmp = rand(100000, 999999);

  // user_tvchat_service_change_password($password_tmp, $uid);
  $edit = array('name' => $account->name, 'pass' => $password_tmp);
  $ret = user_save($account, $edit);

  profile_load_profile($account);
  $edit = array(
    'profile_password_temp' => $password_tmp
  );
  tvchat_user_save_profile($edit, $account, 'Personal information');

  rules_invoke_event('account_reset', $account, $password_tmp);

  /*
  // flagging this user for rule activation
  // 
  $flag = flag_get_flag('account_reset');
  if($flag){

    // 이미 flag되어 있으면, unflag시킴
    //
    if($flag->is_flagged($account->uid, $account->uid)) {
        flag('unflag', 'account_reset', $account->uid, $account);
    }
    flag('flag', 'account_reset', $account->uid, $account);
    tvchat_log('flagged uid:'.$uid.' email:'.$email);
  }
  */

  return $account;
}

//
//
function user_tvchat_service_update($nick=null, $todayMessage=null, $fname=null, $picture=null, $message_alarm=null, $tvshow_alarm=null, $fb_id=null, $token_APNS=null, $phone_number=null, $email=null) {

  global $user;
  error_log('user_tvchat_service_update:'.$user->uid);

  global $base_url;

  if ((!empty($picture)) || !empty($_POST['picture'])) {
      $picture = str_replace('"', '', $_POST['picture']);
  }

  if(!empty($nick)) {

      $account = user_load($user->uid);
      $uid_by_nick = db_result(db_query("SELECT uid FROM {profile_values} WHERE fid=5 AND value='%s'", $nick));

      // 이메일이 기존에 있을 경우
      // 
      if (!empty($uid_by_nick) && (trim($account->profile_nickname) != trim($nick))) {
          return services_error(t('Nickname is already taken.'), 10002);
      }
  }

  $account = user_load($user->uid);

  // user picture update
  // 
  if(!empty($fname) && !empty($picture)) {
    $ret = _user_save_picture($user->uid, $fname, $picture);

    if($ret < 0) { 
        return services_error(t("User Picture Can't be save"), 502); 
    }
  }

  // profile update
  // 
  profile_load_profile(&$account);
  $edit = array(
      'profile_nickname'     => isset($nick)? $nick: $account->profile_nickname,
      'profile_todayMessage' => isset($todayMessage)? $todayMessage: $account->profile_todayMessage,
      'profile_APNS_token'  => isset($token_APNS)? $token_APNS: $account->profile_APNS_token,
      'profile_phone_no'    => isset($phone_number)? $phone_number: $account->profile_phone_no,
      'profile_message_alarm' => isset($message_alarm)? $message_alarm: $account->profile_message_alarm,
      'profile_tvshow_alarm' => isset($tvshow_alarm)? $tvshow_alarm: $account->profile_tvshow_alarm,
  );
  tvchat_user_save_profile($edit, $account, 'Personal information');

  $account = user_load($user->uid);
  $account->picture = empty($account->picture) ? null: $base_url.'/'.$account->picture;

  return $account;
}

/**
+ * Changes the password on the user with corresponding $uid
+ */
function user_tvchat_service_change_password($newPassword, $uid=0) {
  global $user;

  if (!user_is_logged_in()) {
    return services_error(t('This user is not logged in.'), 403);
  }

  // If a user id other than the current user's ID is passed,
  // validate that the authenticated user has the correct 
  // permissions to read another user's messages.
  if (is_numeric($uid) && ($uid > 0) && ($uid != $user->uid)) {
    return services_error(t('User is not the same as logged in user'));
  } // Use the current user for the account.

  if ($uid == 0) { $uid = $user->uid; }
  $account = user_load($uid);

  $edit = array('name' => $account->name, 'pass' => $newPassword);
  $ret = user_save($account, $edit);

  if ($ret) { return TRUE; 
  } else {
      return FALSE;
  }
}

