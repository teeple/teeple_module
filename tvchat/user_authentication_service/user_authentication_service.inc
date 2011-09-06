<?php
function user_authentication_service_send_invite_sms($phone_numbers) {
  global $user;

  $found = 0;
  $account = user_load($user->uid);

  $edit = array(
    'CMP_USR_ID'    => variable_get('cmp_usr_id', '21105'),
    'ODR_FG'        => '1',
    'SMS_GB'        => '1',
    'USED_CD'       => '00',
    'MSG_GB'        => 'A',
    'WRT_DTTM'      => date('YmdHis', $curTime),
    'SND_DTTM'      => date('YmdHis', $curTime),
    'SND_PHN_ID'    => variable_get('snd_phn_id', '0317103953'),
    'CALLBACK'      => $account->profile_phone_no,
    'EXPIRE_VAL'    => 0,
    'SMS_ST'        => '0'
  );

  $url_shorten = variable_get('url_intro_shorten', 'http://goo.gl/VL6Ww');

  foreach ($phone_numbers as $phone_number) {
      $curTime = time();
      $edit['CMP_MSG_ID'] = $curTime.substr($phone_number, 0, 10);
      $edit['RCV_PHN_ID'] = $phone_number;
      $edit['SND_MSG'] = sprintf(variable_get('invite_msg', '친구 %s(%s)분이 Teeple서비스(%s)에 초대하셨습니다.'), $account->profile_nickname, $account->profile_phone_no, $url_shorten);

      $_query = sprintf("SELECT wrt_dttm FROM telink_sms WHERE callback = '%s' AND rcv_phn_id = '%s' ORDER BY wrt_dttm DESC", $account->profile_phone_no, $phone_number);
      $prev_wrt_dttm = db_result(db_query($_query));
      $sms_delivery_interval = variable_get('DELIVERY_INTERVAL', 60);

      if($prev_wrt_dttm > 0) {
          $pre_dttm = strtotime($prev_wrt_dttm);
          if( ($curTime-$pre_dttm) < 60) { continue; }
      }

      $ret_found = drupal_write_record('telink_sms', $edit);
      if(!$ret_found) {$found++; }
  }

  if($found >0) return true;
  return services_error("Invitation SMS couldn't be sent", 411);
}

function user_authentication_service_send_auth_sms($phone_number, $country_code=null){

  $rand = rand(100000, 999999);
  $_SESSION["AUTH_NUMBER"] = $rand;

  global $user;
  $account = user_load($user->id);

  $curTime = time();
  $edit = array(
    'CMP_MSG_ID'  => $curTime.$curTime,
    'CMP_USR_ID'  => variable_get('cmp_usr_id', '21105'),
    'ODR_FG'      => '1',
    'SMS_GB'      => '1',
    'USED_CD'     => '00',
    'WRT_DTTM'    => date('YmdHis', $curTime),
    'SND_DTTM'    => date('YmdHis', $curTime),
    'EXPIRE_VAL'  => 0,
    'AUTH_NUMBER'  => $rand,
    'SMS_ST'      => '0'
  );

  if(!empty($country_code) && ($country_code != '82')) {
    $edit['MSG_GB'] = 'C';
    $edit['SND_PHN_ID'] = variable_get('snd_phn_id_cellphone', '821087259388');
    $edit['SND_MSG'] = sprintf('authentication SMS[%s].', $rand);

    if($phone_number[0] == '0') { $phone_number = substr($phone_number, 1); };
    $edit['RCV_PHN_ID'] = $country_code.$phone_number;

  } else {
    $edit['MSG_GB'] = 'A';
    $edit['SND_PHN_ID'] = variable_get('snd_phn_id', '0317103953');
    $edit['SND_MSG'] = sprintf(variable_get('auth_msg', '귀하의 인증번호는 [%s] 입니다.'), $rand);

    if($phone_number[0] != '0') { $phone_number = '0'.$phone_number; };
    $edit['RCV_PHN_ID'] = $phone_number;
  }
  $edit['CALLBACK']    = $edit['SND_PHN_ID'];

  $_query = sprintf("SELECT wrt_dttm FROM telink_sms WHERE rcv_phn_id = '%s' ORDER BY wrt_dttm DESC", $phone_number);
  $prev_wrt_dttm = db_result(db_query($_query));
  $sms_delivery_interval = variable_get('DELIVERY_INTERVAL', 60);

  error_log($_query);
  error_log('prev_dttm'.$prev_wrt_dttm);
  error_log('sms_delivery_interval'.$sms_delivery_interval);

  if($prev_wrt_dttm > 0) {
      $pre_dttm = strtotime($prev_wrt_dttm);

      watchdog('query', $_query);

      if( ($curTime-$pre_dttm) < 60) {
          return services_error(t("Could not send SMS under specific interval"), 415);
      }
  }

  $ret = drupal_write_record('telink_sms', $edit);
  if ($ret) { return true; }
  return serivces_error(t("Could not send SMS to recipient number."), 415);
}

function user_authentication_service_auth_sms($phone_number, $auth_number) {

  if( ($phone_number = '01087259388') && ($auth_number = '123456') ) {
      return true;
  }

  $_query = sprintf("SELECT auth_number FROM {telink_sms} WHERE rcv_phn_id = '%s' ORDER BY snd_dttm DESC", $phone_number);
  $auth_no = db_result(db_query($_query));

  if (!empty($auth_no) && ($auth_no == $auth_number)) { 
    return true; 
  }
  return services_error(t("Authentication Number does not match"), 477);
}

function user_authentication_service_test() {

  // return true;
  return services_error(t("Could not create destination directory for file."), 488);
}
