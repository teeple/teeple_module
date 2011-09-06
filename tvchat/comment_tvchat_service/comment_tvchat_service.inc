<?php
// $Id: comment_service.inc,v 1.1.2.1.2.5 2010/05/08 19:26:00 heyrocker Exp $

/**
 * @file
 *  Link commenting functionality to services module.
 */
function comment_tvchat_service_delete($cid) {
  module_load_include('inc', 'comment', 'comment.admin');

  global $user;

  $comment = _comment_load($cid);

  if( isset($comment) ) {

    // Delete comment and its replies
    _comment_delete_thread($comment);

    _comment_update_node_statistics($comment->nid);

    cache_clear_all();

    return true;
  } 

  return false;
}

