<?php
/**
*implementation hook_views_data
*it takes form uid and join it with uid in user table 
*/

function pollfield_views_data(){
  $data['pollfield']['table']['group']  = t('Pollfield');

  $data['pollfield']['table']['join'] = array(
   'node' => array(
      'type' => 'INNER',      
      'left_field' => 'nid',
      'field' => 'nid',
     ),
  );
	$data['pollfield']['nid'] = array(
    'title' => t('Node ID'),
    'help' => t('Node id of node where pollfield is placed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['pollfield']['question'] = array(
    'title' => t('Question'),
    'help' => t('Question of poll.'),

    'field' => array(
      'handler' => 'views_handler_field_pollfield_question',
      'click sortable' => TRUE,
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

  $data['pollfield']['choices'] = array(
    'title' => t('Choices'),
    'help' => t('All user choices.'),

    'field' => array(
      'handler' => 'views_handler_field_pollfield_choices',
      'click sortable' => TRUE,
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

   $data['pollfield']['field_name_delta'] = array(
    'title' => t('Pollfield ID'),
    'help' => t('Id of pollfield if there is more than one pollfiend in one node.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

   $data['pollfield']['field_name'] = array(
    'title' => t('Field'),
    'help' => t('Name of field.'),

    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
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
	$data['pollfield']['field_table'] = array(
    'title' => t('Content type'),
    'help' => t('Content type where pollfield is stored.'),

    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
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

  // views for pollfield_votes 
  $data['pollfield']['uid'] = array(
    'title' => t('User ID'),
    'help' => t('User id of user who cast this vote.'),
    'field' => array(
      'handler' => 'views_handler_field_pollfield_uid',
      'click sortable' => TRUE,
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['pollfield']['results'] = array(
    'title' => t('Results'),
    'help' => t('total votes for each choices.'),

    'field' => array(
      'handler' => 'views_handler_field_pollfield_results',
      'click sortable' => TRUE,
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

  $data['pollfield']['is_vote'] = array(
    'title' => t('Already vote or not'),
    'help' => t('already voted or not.'),

    'field' => array(
      'handler' => 'views_handler_field_pollfield_is_vote',
      'click sortable' => TRUE,
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

  $data['pollfield']['choice'] = array(
    'title' => t('User Choice among choices'),
    'help' => t('choice'),

    'field' => array(
      'handler' => 'views_handler_field_pollfield_choice',
      'click sortable' => TRUE,
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
/**
* hook alter data 
*/
function pollfield_views_data_alter(&$data){

}
/**
* hook regsister handlers to views
*/
function pollfield_views_handlers() {
  
  return array(
     'handlers' => array(
      'views_handler_field_pollfield_uid' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_pollfield_choices' => array(
        'parent' => 'views_handler_field',
      ),  
      'views_handler_field_pollfield_results' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_pollfield_is_vote' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_pollfield_choice' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_pollfield_question' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}


