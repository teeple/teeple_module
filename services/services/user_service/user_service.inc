<?php
// $Id: user_service.inc,v 1.1.2.8.2.12 2010/10/26 23:54:21 ocyrus Exp $

/**
 * @file
 *  Link general user functionalities to services module.
 */

/**
 * Delete a user.
 *
 * @param $uid
 *   UID of the user to be deleted.
 *
 * @see user_delete()
 */
function user_service_delete($uid) {
  $account = user_load($uid);
  if (empty($account)) {
    return services_error(t('There is no user with such ID.'), 404);
  }
  user_delete($account, $uid);

  // Everything went right.
  return TRUE;
}

/**
 * Check to see if the services account is allowed to delete a user.
 *
 * @param $uid
 *   UID of the user to be deleted.
 */
function user_service_delete_access($uid) {
  global $user;
  return (($user->uid == $uid && user_access('delete own user')) ||
    ($user->uid != $uid && user_access('delete any user')));
}

/**
 * Get user details.
 *
 * @param $uid
 *   UID of the user to be loaded.
 *
 * @return
 *   A user object.
 *
 * @see user_load()
 */
function user_service_get($uid) {
  $account = user_load($uid);

  if (empty($account)) {
    return services_error(t('There is no user with such ID.'), 404);
  }

  // Everything went right.
  return $account;
}

/**
 * Check if the user is allowed to get the user data.
 *
 * @param $uid
 *   Number. The user ID.
 */
function user_service_get_access($uid) {
  global $user;
  return (($user->uid == $uid && user_access('get own user data')) || ($user->uid != $uid && user_access('get any user data')));
}

/**
 * Login a user using the specified credentials.
 *
 * Note this will transfer a plaintext password.
 *
 * @param $username
 *   Username to be logged in.
 * @param $password
 *   Password, must be plain text and not hashed.
 * @param $phone_number
 *   Phone number
 *
 * @return
 *   A valid session object.
 */
function user_service_login($username, $password) {
  global $user;

  if ($user->uid) {
    return services_error(t('Already logged in as !user.', array('!user' => $user->name)), 406);
  }

  $user = user_authenticate(array('name' => $username, 'pass' => $password));
  if ($user->uid) {
    // Regenerate the session ID to prevent against session fixation attacks.
    sess_regenerate();
    $array = array();
    user_module_invoke('login', $array, $user);

    $return = new stdClass();
    $return->sessid = session_id();
    $return->user = $user;

    return $return;
  }
  session_destroy();
  return services_error(t('Wrong username or password. '), 401);
}

/**
 * Logout the current user.
 */
function user_service_logout() {
  global $user;

  if (!$user->uid) {
    // User is not logged in
    return services_error(t('User is not logged in.'), 406);
  }

  watchdog('user', 'Session closed for %name.', array('%name' => theme('placeholder', $user->name)));

  // Destroy the current session:
  session_destroy();
  $array = array();
  user_module_invoke('logout', $array, $user);

  // Load the anonymous user
  $user = drupal_anonymous_user();

  return TRUE;
}

/**
 * Save user data.
 *
 * This can create a new user or modify an existing user, depending on the
 * information passed to the function. This function uses drupal_execute()
 * and as such exepects all input to match the submitting form in question.
 *
 * @param $account
 *   An array of account information.
 *
 * @return
 *   The UID of the user whose information was saved.
 */
function user_service_save($account) {

	// bhtak 2011/1/9
  // json_decode returns an object
  // watchdog('user.save1', print_r( $account,TRUE));
  // watchdog('user.save1', drupal_to_js($account));
  if (is_object($account)) {
    $account = (array)$account;
  }

  // Load the required includes for saving profile information
  // with drupal_execute().
  module_load_include('inc', 'user', 'user.pages');

  // if uid is present then update, otherwise insert
  $update = user_load($account['uid']);	
  $form_state = array();

  // Any logged in user is by default authenticated,
  // and leaving this role set in the user's roles array
  // causes big problems because of a FAPI hack that controls
  // this checkbox on the user create and edit form (and thus
  // causes problems with drupal_execute()). Therefore we just
  // force it to 0 here.
  if (isset($account['roles'][2])) {
    $account['roles'][2] = 0;
  }

  //  watchdog('user.save2', print_r( $update,TRUE));
  if (!isset($update->uid)) {
    // register a new user
    $form_state['values'] = $account;
    $form_state['values']['pass'] = array(
      'pass1' => $account['pass'],
      'pass2' => $account['pass'],
    );
    $form_state['values']['op'] = t('Create new account');
    $ret = drupal_execute('user_register', $form_state);
    watchdog('user.save3', 'form='.print_r( $form_state,TRUE).'ret='.print_r($ret,TRUE));
  }
  else {
    // If a profile category was passed in, use it. Otherwise default
    // to 'account' (for saving core user data.)
    $category = 'account';
    if (isset($account['category'])) {
      $category = $account['category'];
      unset($account['category']);
    }
    
    // Drop any passed in values into the $account var. Anything
    // unused by the form just gets ignored.
    foreach ($account as $key => $value) {
      $form_state['values'][$key] = $value;
    }

    // watchdog('user.save4', print_r( $account,TRUE));
    $form_state['values']['op'] = 'Save';
    $form_state['values']['_category'] = $category;
    $form_state['values']['_account'] = $account;
    $ret = drupal_execute('user_profile_form', $form_state, (object) $account, $category);
  }

  // Error if needed.
  if ($errors = form_get_errors()) {
    // watchdog('user.save5', 'errors='.print_r( $errors,TRUE));
    return services_error(implode("\n", $errors), 401);
  }
  else {
    return $form_state['user']->uid;
  }
}

/**
 * Check if the services account is allowed to create or edit user data.
 *
 * @param $account
 *   An array of user account information (note that UID is the only
 *   property used.)
 */
function user_service_save_access($account) {
	// bhtak 2011/1/9
  // json_decode returns an object
  watchdog('save.access',print_r($accoaunt,TRUE));
  if (is_object($account)) {
    $account = (array)$account;
  }

  global $user;
  return ((empty($account['uid']) && user_access('create new users')) ||
    ($user->uid == $account['uid'] && user_access('update own user data')) ||
    ($user->uid !=  $account['uid'] && user_access('update any user data')));
}

/**
 * Update user data.
 *
 * This can modify an existing user after creating, depending on the
 * information passed to the function. This function uses drupal_execute()
 * and as such exepects all input to match the submitting form in question.
 *
 * @param $uid.
 *    integer 
 *    user id.
 * @param $pass.
 *    string
 *    user password. md5 encoded.
 * @param $nick
 *    string
 *    user nick name. (profile nick name)
 * @param $todayMessage
 *    string
 *    user today Message. (profile todayMessage)
 * @param $picture
 *    string
 *    user picture. base64 encoded
 *
 * @return
 *   The UID of the user whose information was updated.
 */
function user_service_update_org($uid=0, $nick=null, $todayMessage=null, $fname=null, $picture=null, $message_alarm=null, $tvshow_alarm=null, $fb_id=null, $token_APNS=null, $phone_number=null) {

  // User needs to be authenticated to proceed.
  global $user;
  if (!user_is_logged_in()) {
    return services_error(t('This user is not logged in.'), 403);
  }

  // If a user id other than the current user's ID is passed,
  // validate that the authenticated user has the correct 
  // permissions to read another user's messages.
  if (is_numeric($uid) && ($uid != $user->uid)) {
      $account = user_load($uid);
  } // Use the current user for the account.
  else {
    $account = $user;
  }

  if (is_object($account)) {
    $account = (array)$account;
  }

  // Load the required includes for saving profile information
  // with drupal_execute().
  module_load_include('inc', 'user', 'user.pages');

  // if uid is present then update, otherwise insert
  $update = user_load($account['uid']);
  $form = array();
  $form_state = array();

  /*
  if (isset($account['roles'][2])) {
    $account['roles'][2] = 0;
  }
  */

  if (isset($update->uid)) {
    $category = 'account';
    if (isset($nick) || isset($todayMessage) || isset($message_alarm) || isset($tvshow_alarm)) {
      $account['category'] = 'Personal information';
      if (isset($nick)) {
        $account['profile_nickname'] = $nick;
      }
      if (isset($todayMessage)) {
        $account['profile_todayMessage'] = $todayMessage;
      }
      if (isset($message_alarm)) {
        $account['profile_message_alarm'] = $message_alram;
      }
      if (isset($tvshow_alarm)) {
        $account['profile_tvshow_alarm'] = $tvshow_alram;
      }
      if (isset($fb_id)) {
        $account['profile_fb_account'] = $fb_id;
      }
      if (isset($token_APNS)) {
        $account['profile_APNS_token'] = $token_APNS;
      }
      if (isset($phone_number)) {
        $account['profile_phone_no'] = $phone_number;
      }
    }

    if (isset($account['category'])) {
      $category = $account['category'];
      unset($account['category']);
    }

    $list = Array( 'profile_nickname', 'profile_todayMessage', 'profile_message_larm', 'profile_tvshow_alarm', 'profile_fb_account', 'profile_APNS_token', 'profile_phone_no', 'name', 'mail');
    foreach( $list as $key) {
      if (isset($account[$key])) {
        $form_state['values'][$key] = $account[$key];
        watchdog('user_service', 'KEY:'.$key.' VALUE: '.print_r( $account[$key], TRUE));
      }
    }

    // user picture manipulation
    /*
    if (isset($fname) && isset($picture)) {
      $file_picture = _user_picture_upload((object)$account,$fname,$picture);
      $_FILES['files']['name']['picture_upload'] = $file_picture->filename;
      $_FILES['files']['type']['picture_upload'] = $file_picture->filemime;
      $_FILES['files']['tmp_name']['picture_upload'] = $file_picture->filepath;
      $_FILES['files']['error']['picture_upload'] = 0;
      $_FILES['files']['size']['picture_upload'] = $file_picture->filesize;

      $form['#uid'] = $uid;
    }
    */

    $form_state['values']['op'] = 'Save';
    $form_state['values']['_category'] = $category;
    $form_state['values']['_account'] = $account;

    if ($category == 'account') {
      $ret = drupal_execute('user_profile_form', $form_state, (object)$account, $category);

    } else {

      // for other catebgory (Personal Information);
      // 
      $form_state['values']['_category'] = $category;
      $ret = drupal_execute('user_profile_form', $form_state, (object)$account, $category);
 
      // for image upload (account)
      //
      /*
      $form_state['values']['_category'] = 'account';
      $ret = drupal_execute('user_profile_form', $form_state, (object)$account, 'account');
      */
    }

  } else {
    return services_error("user id doesn't exist or can't be logged in");
  }

  // Error if needed.
  if ($errors = form_get_errors()) {
    return services_error(implode("\n", $errors), 401);
  }
  else {
    return $update->uid;
  }
}

