<?php
function views_views_pre_execute(&$view) {
 drupal_set_message($view->name);

 if($view->name=='query_tvchat_audience_list_1' 
     || $view->name=='query_tvchat_audience_list_2' ) {

     if(empty($view->args)) { 
         $view->build_info['query']=
             "SELECT
             comments.cid AS cid, 
             comments.comment AS comment,
             node_comments.nid AS node_comments_nid, 
             node_comments.language AS node_comments_language, 
             node_comments.type AS node_comments_type, 
             users_comments.uid AS users_comments_uid, 
             users_comments_profile_values_profile_nickname.value AS users_comments_profile_values_profile_nickname_value,
              users_comments_profile_values_profile_nickname.uid AS users_comments_profile_values_profile_nickname_uid, 
              users_comments.access AS users_comments_access, 
              users_comments.picture AS users_comments_picture, 
              users_comments.name AS users_comments_name, 
              users_comments.mail AS users_comments_mail, 
              comments.timestamp AS comments_timestamp 
              FROM comments comments  LEFT JOIN node node_comments ON comments.nid = node_comments.nid LEFT JOIN users users_comments ON comments.uid = users_comments.uid LEFT JOIN profile_values users_comments_profile_values_profile_nickname ON users_comments.uid = users_comments_profile_values_profile_nickname.uid AND users_comments_profile_values_profile_nickname.fid = '5' 
              WHERE comments.cid IN ( SELECT max(cid) cid FROM comments GROUP BY comments.uid )
              ORDER BY comments_timestamp DESC";

     } else {
         $where[] = empty($view->args[0]) ? '': ' comments.nid IN ('.$view->args[0].')';
         if($view_name == 'query_tvchat_audience_list_2') { 
            $where[] = empty($view->args[1]) ? '':' comments.uid NOT IN ('.$view->args[1].')';
         } else {
            $where[] = empty($view->args[1]) ? '':' comments.uid IN ('.$view->args[1].')';
         }

         if(!empty($where)) {
             $where = 'WHERE '.join(' AND ', $where);
         }

         $view->build_info['query']=
             "SELECT
             comments.cid AS cid, 
             node_comments.nid AS node_comments_nid, 
             node_comments.language AS node_comments_language, 
             node_comments.type AS node_comments_type, 
             users_comments.uid AS users_comments_uid, 
             users_comments_profile_values_profile_nickname.value AS users_comments_profile_values_profile_nickname_value,
              users_comments_profile_values_profile_nickname.uid AS users_comments_profile_values_profile_nickname_uid, 
              users_comments.access AS users_comments_access, 
              users_comments.picture AS users_comments_picture, 
              users_comments.name AS users_comments_name, 
              users_comments.mail AS users_comments_mail, 
              comments.timestamp AS comments_timestamp 
              FROM comments comments  LEFT JOIN node node_comments ON comments.nid = node_comments.nid LEFT JOIN users users_comments ON comments.uid = users_comments.uid LEFT JOIN profile_values users_comments_profile_values_profile_nickname ON users_comments.uid = users_comments_profile_values_profile_nickname.uid AND users_comments_profile_values_profile_nickname.fid = '5' WHERE
              comments.cid IN ( SELECT max(cid) cid FROM comments ".$where." GROUP BY comments.uid )
              ORDER BY comments_timestamp DESC";
     }
   }
}

function args_mapper($view_name, $view_results){
  $ret_array = array();

  switch($view_name) {
    case: 'query_tvchat_audience_list_1':
    case: 'query_tvchat_audience_list_2':
    default:
      break;
  }
}
?>
