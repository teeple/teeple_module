<?php

/**
 * Implementation of hook_views_data().
 */
function comment_upload_views_data() {
  $data = array();
  $data['comment_upload']['table']['group'] = t('Comment Upload');

  $data['comment_upload']['table']['join'] = array(
    'comments' => array(
      'left_field' => 'cid',
      'field' => 'cid',
    ),
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
    'files' => array(
      'left_field' => 'fid',
      'field' => 'fid',
    ),
  );

  $data['comment_upload']['nid'] = array(
    'title' => t('Node'),
    'help' => t('The parent node of the comment'),
    'relationship' => array(
      'label' => t('Node'),
      'base' => 'node',
      'base field' => 'nid',
      'skip base' => array('node'),
    ),
  );

  $data['comment_upload']['cid'] = array(
    'title' => t('Comment'),
    'help' => t('The comment that the file is attached to'),
    'relationship' => array(
      'label' => t('Comment'),
      'base' => 'comments',
      'base field' => 'cid',
      'skip base' => array('comments'),
    ),
  );

  // quantity field
  $data['comment_upload']['fid'] = array(
    'title' => t('File'),
    'help' => t('The uploaded file.'),
    'relationship' => array(
      'label' => t('File'),
      'base' => 'files',
      'base field' => 'fid',
      'skip base' => array('files'),
    ),
  );

  // Color field    
  $data['comment_upload']['description'] = array(
    'title' => t('Description'),
    'help' => t('Description of the uploaded file.'),

    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => FALSE,
     ),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
     'argument' => array(
       'handler' => 'views_handler_argument_string',
     ),
     'sort' => array(
      'handler' => 'views_handler_sort',
     ),
  );

  return $data;
}
?>
