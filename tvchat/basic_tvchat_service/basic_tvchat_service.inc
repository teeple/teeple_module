<?php
function array_value_recursive($key, array $arr){
    $val = array();
    array_walk_recursive($arr, function($v, $k) use($key, &$val){
            if($k == $key) array_push($val, $v);
            });

    return $val;
    /*
    return count($val) > 1 ? $val : array_pop($val);
    */
}

// 주소록 사용자 중 서비스 이용 중인 사용자를 가져와서 리턴한다.
// 
function _fetch_existing_phone_list($phone_list) {
    global $base_url;

    // check the phone number already exists
    // 해당 폰번호가 존재하는지를 확인한다.
    // 
    $query_phone_list = implode("','", $phone_list);

    $_query = sprintf("SELECT uid, value FROM {profile_values} WHERE fid=14 AND value IN ('".$query_phone_list."')");

    error_log('query: '.$_query);

    $db_results = db_query($_query);

    $friends_uid_list = array();

    // 폰번호가 존재한다는 얘기는 친구가 이미 가입되어 있다면, 
    //
    while ($element = db_fetch_object($db_results)) {
        $account = user_load($element->uid);

        if(!empty($account)) {
            $phone_no = $element->value;

            $friends_list[$phone_no] = array (
                'uid'       => $account->uid,
                'userId'    => $account->uid,
                'status'    => $account->status,
                'phone'     => $phone_no,
                'userName'  => $account->name,
                'userNickName' => $account->profile_nickname,
                'userPicture' => empty($account->picture) ? null : $base_url.'/'.$account->picture,
                'exist'     => 'yes'
            );
        }
    }
    return $friends_list;
}

function already_request($uid, $fr_uid) {

    $type = user_relationships_type_load(array('name' => 'Friend'));
    $rels = user_relationships_load( array('rtid' => $type->rtid, 'between' => array($uid, $fr_uid)));

    if (count($rels) > 0) {
        $rel = array_shift($rels);

        if ( ($rel->approved != '1') && ($rel->requester_id == $user->uid)) {
                return true;
        }
    } 
    return false;
}

function _request_n_make_friends($phone_uid_list, $friends_uid_list) {

    module_load_include('inc', 'views_service');
    global $user;

    // A  = (이미 존재하는 주소록 사람 ) - (친구) 
    $none_friends_uid_list = array_diff($phone_uid_list, $friends_uid_list);
    // watchdog('make_friends', 'A'.join(',', $none_friends_uid_list));

    // 해당 사용자가 나에게 친구 요청한 것을 확인한다.
    // 
    $view_name = 'query_ur_receive_json';
    $args[] = join(',', $none_friends_uid_list);
    $offset = 0; $limit = 0;

    $ret = views_service_get($view_name, null, $args, $offset, $limit, FALSE);

    $requested_relationship_list = args_mapper($view_name, $ret);
    $requested_uid_list = array_value_recursive('requesterUid', $requested_relationship_list);
    // watchdog('make_friends', 'Requested:'.join(',', $requested_uid_list));

    // 나에게 친구 요청된 것들은 승인한다.
    // 
    foreach($requested_relationship_list as $element) {
        user_relationship_service_approve("1", $element['rid']);
    }

    // B = A  - (이미 내게 친구 요청한 사람)
    //
    $non_requested_uid_list = array_diff($none_friends_uid_list, $requested_uid_list);
    // watchdog('make_friends', 'B:'.join(',', $non_requested_uid_list));
    
    // C = B 중 (내가 이미 친구 요청한 사람 리스트)
    //
    $view_name = 'query_ur_request_json';
    $args[] = join(',', $non_requested_uid_list);
    $offset = 0; $limit = 0;

    $ret = views_service_get($view_name, null, $args, $offset, $limit, FALSE);
    $requestee_relationship_list = args_mapper($view_name, $ret);
    $requestee_uid_list = array_value_recursive('requsteeUid', $requestee_relationship_list);
    // watchdog('make_friends', 'C:'.join(',', $requestee_uid_list));

    // D = B - C 
    $non_requestee_uid_list = array_diff($non_requested_uid_list, $requestee_uid_list);
    // watchdog('make_friends', 'D:'.join(',', $non_requestee_uid_list));

    foreach ($non_requestee_uid_list as $uid) {
        if($user->uid != $uid) {
            user_relationship_service_request("1", $uid, 'Friend');
        }
    }
}

function make_dummy_users($phone_list) {

    module_load_include('inc', 'anonymous_tvchat_service');

    global $base_root;

    foreach ($phone_list as $phone_no) {
        $ret_array = array();

        $nick = $phone_no;
        $email = $phone_no.'@'.$_SERVER['HTTP_HOST'];
        $phone_number = $phone_no;
        $fname = null;
        $picture = null;
        $token_APNS = null;
        $status = 0;            // 블록된 상태로 사용자 생성

        $ret_array = anonymous_tvchat_service_signup($nick,$email,$phone_number,$fname,$picture,$token_APNS, $status);

        // dummy 친구들에게 친구 요청을 해둔다.
        // 
        $ret = user_relationship_service_request("1", $ret_array['uid'], 'Friend');
        // watchdog('request_relationship:', print_r($ret));
    }
}

// tvchat_service_upload_contacts
//
function basic_tvchat_service_upload_contacts($contacts_json) {
 try {
    global $user;
    global $base_url;

    error_log('tvchat_service_upload_contacts');

    module_load_include('inc', 'tvchat');
    module_load_include('inc', 'user_service');
    module_load_include('inc', 'user_tvchat_service');
    module_load_include('inc', 'user_relationship_service');

    $fr_list = array();
    $contacts_array = array();

    $phone_name_list = array();     // 사용자 userName, picture에 대한 리스트 
    $friends_phone_list = array();
    $phone_list = array();          // 사용자로부터 받은 주소록 
    $phone_list = array_unique($contacts_json);

    // 주소록에서 나의 번호는 제외한다.
    //
    $phone_list = array_diff($phone_list, (array)$account->profile_phone_no);

    // 이미 친구인 리스트를 구한다.
    //
    $friends_uid_list = tvchat_friends_uid_list();

    if(!empty($friends_uid_list)) {
        // watchdog('make_friends', 'friends: '.join(',', $friends_uid_list));
    }

    // A = ( 주소록에 있는 사람들 중 서비스 이용 중인 사람들 )
    // 
    $ret_contact_list= _fetch_existing_phone_list($phone_list);

    if(!empty($ret_contact_list)) {
      $existing_uid_list = array_value_recursive('uid', $ret_contact_list);
      $existing_phone_list = array_value_recursive('phone', $ret_contact_list);

      // 자동 친구 요청 또는 친구 맺기 
      //
       _request_n_make_friends($existing_uid_list, $friends_uid_list);
    }

    // 이미 존재하는 사용자를 제외하고, 주소록에 있는 사용자에 대해서 임의로 계정을 생성한다.
    // B = (주소록 사람들) -  A
    //
    $friends_phone_new_list = array();
    if(!empty($existing_phone_list)) {
        $friends_phone_new_list = array_diff($phone_list, $existing_phone_list);

    } else {
        $friends_phone_new_list = $phone_list;
    }

    // B 에 해당하는 사람들은 가상의 인물로 생성한다.
    // 이 때, 친구 요청도 해둔다.
    // 
    if(!empty($friends_phone_new_list)) {
        $dummy_list = make_dummy_users($friends_phone_new_list);
        // watchdog('make_friends', 'dummy:'.join(',', $friends_phone_new_list));
    }

    $ret_contacts = array();
    foreach ((array)$ret_contact_list as $phone_element) {
        if($phone_element['status'] <> 0) {
            $ret_contacts[] = (object)$phone_element;
        }
    }
    return $ret_contacts;

  } catch (Exception $ex) {
    return services_error(t('Error listing relationship: @msg', array('@msg' => $ex->getMessage())));
  }
}

function _get_element($uid, $comment, $type, $timestamp){

    global $base_url;
    $account = user_load($uid);
    $element = array("comment" => $comment,
        "type" => $type,
        "timestamp"=> $timestamp,
        "uid" => $uid, 
        "userId" => $uid, 
        "userName" => $account->name, 
        "userNickName" => $account->profile_nickname, 
        "access" => $account->access
    );

    $element['loginStatus'] = get_login_status($uid);
    $element['userPicture'] = empty($account->picture) ? null : $base_url.'/'.$account->picture;

    return $element;
}

