<?php
// $Id: user_relationship_service.inc,v 1.1.2.3 2010/05/14 07:47:40 alexk Exp $
/**
 * @author Drupal 6 port by Darren Ferguson <http://drupal.org/user/70179>
 * @author Written by scottgifford http://drupal.org/user/245699
 * @file Link general user relationship functionalities to services module.
 */
function user_relationship_service_mine_with_type($version, $type_name) {
  try {
    global $user;
    $type = user_relationships_type_load(array('name' => $type_name));
    $param = array('user' => $user->uid, 'rtid' => $type->rtid, 'approved' => '1');
    $rels = user_relationships_load($param);
    if (!$rels || !is_array($rels)) {
      throw new Exception("User relationship load failed");
    }
    return array_values($rels);
  } catch (Exception $ex) {
    return services_error(t('Error getting user relationships: @msg', array('@msg' => $ex->getMessage())));
  }
}

function user_relationship_service_mine_with_params($version, $type_name, $received = 0) {
  try {
    global $user;
    $type = user_relationships_type_load(array('name' => $type_name));

    $param = array('user' => $user->uid, 'rtid' => $type->rtid, 'approved' => '1');
    if ( $received > 0 ) {
        $param['requestee_id'] = $user->uid;
        $param['approved'] = '0';
    }

    $rels = user_relationships_load($param);
    if (!$rels || !is_array($rels)) {
      throw new Exception("User relationship load failed");
    }
    return array_values($rels);
  } catch (Exception $ex) {
    return services_error(t('Error getting user relationships: @msg', array('@msg' => $ex->getMessage())));
  }
}

function user_relationship_service_approve_with_rids($version, $rid_list) {
  try {
    global $user;

    foreach($rid_list as $rid) {
        $rels = user_relationships_load(array('rid' => $rid , 'requestee_id' => $user->uid, 'approved' => 0));
        if (!$rels || !is_array($rels) || count($rels) != 1) {
          throw new Exception("User relationship load failed");
        }
        $rel = array_shift($rels);
        if ($rel->requestee_id != $user->uid) {
          throw new Exception("Access denied");
        }
        $ret = user_relationships_save_relationship($rel, 'approve');

        if (isset($ret )) {
            $ret_array[] = $rid;
        } 

    }
    return $ret_array;

  } catch (Exception $ex) {
    return services_error(t('Error approving relationship: @msg', array('@msg' => $ex->getMessage())));
  }
}

function user_relationship_service_approve_with_params($version, $type_name, $fr_uid_list) {
  try {
    global $user;
    $rels = user_relationships_load(array('rid' => $rid , 'requestee_id' => $user->uid, 'approved' => 0));
    if (!$rels || !is_array($rels) || count($rels) != 1) {
      throw new Exception("User relationship load failed");
    }
    $rel = array_shift($rels);
    if ($rel->requestee_id != $user->uid) {
      throw new Exception("Access denied");
    }
    user_relationships_save_relationship($rel, 'approve');
    return $rel;
  } catch (Exception $ex) {
    return services_error(t('Error approving relationship: @msg', array('@msg' => $ex->getMessage())));
  }
}

function user_relationship_service_fb_list($version, $fb_friends) {
  global $user;

  try {
    $fb_list = array();

    if (!is_array($fb_friends)) {
      $fb_friends = json_decode($fb_friends);
    }

    foreach ($fb_friends->data as $element) {
      $fb_list[] = $element->id; // only fetch the facebook id not name.
    }

    $arguments = implode("','", $fb_list);

    $_query = sprintf("SELECT users.uid AS uid, users.name AS users_name, users.picture AS users_picture, users.mail AS users_mail, profile_values_profile_todayMessage.value AS profile_values_profile_todayMessage_value, profile_values_profile_todayMessage.uid AS profile_values_profile_todayMessage_uid, profile_values_profile_fb_account.value AS profile_values_profile_fb_account_value, profile_values_profile_fb_account.uid AS profile_values_profile_fb_account_uid FROM users users  LEFT JOIN profile_values profile_values_profile_todayMessage ON users.uid = profile_values_profile_todayMessage.uid AND profile_values_profile_todayMessage.fid = '6' LEFT JOIN profile_values profile_values_profile_fb_account ON users.uid = profile_values_profile_fb_account.uid AND profile_values_profile_fb_account.fid = '3' WHERE profile_values_profile_fb_account.value IN ('".$arguments."')");
    $db_results = db_query($_query);

    watchdog('user_relationship_service', 'arguments:'.$arguments);

    $friends_list = array();
    while ($friend = db_fetch_object($db_results)) {
        $friends_list[] = $friend;
    }

    return $friends_list;

  } catch (Exception $ex) {
    return services_error(t('Error listing relationship: @msg', array('@msg' => $ex->getMessage())));
  }
}

function user_relationship_service_delete_with_uid($version, $fr_uid, $type_name) {
  try {
    module_load_include('inc', 'user_relationship_service');
    global $user;

    $type = user_relationships_type_load(array('name' => $type_name));
    $rels = user_relationships_load(array('rtid' => $type->rtid, 'between' => array($user->uid, $fr_uid)));

    if (!$rels || !is_array($rels) || count($rels) != 1) {
     throw new Exception('User relationship load failed');
    }

    $rel = array_shift($rels);
    if ($rel->requestee_id != $user->uid && $rel->requester_id != $user->uid) {      throw new Exception("Access denied");
    }

    $reason = 'Delete relationship';
    user_relationships_delete_relationship($rel, $user, $reason);
    return true;

  } catch (Exception $ex) {
    return services_error(t('Error removing relationship: @msg', array('@msg' => $ex->getMessage())), 699);
  }
}

