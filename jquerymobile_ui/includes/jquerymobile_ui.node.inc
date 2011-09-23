<?php 

/**
 * @file
 * jquerymobile_ui.node.inc
 *
 * node related hook overrides
 */

function jquerymobile_ui_menu_alter(&$items) {
  $items['node']['page callback'] = 'jquerymobile_ui_page_default';
  return $items;
}

function jquerymobile_ui_page_default() {

	$jquerymobile_ui_front = variable_get('jquerymobile_ui_front', TRUE);
	
  $result = pager_query(db_rewrite_sql('SELECT n.nid, n.sticky, n.created FROM {node} n WHERE n.promote = 1 AND n.status = 1 ORDER BY n.sticky DESC, n.created DESC'), variable_get('default_nodes_main', 10)); 

  if ($jquerymobile_ui_front) {
	  $output = '<ul data-role="listview">';
	  $num_rows = FALSE;
	  while ($value = db_fetch_object($result)) {
	    $node = node_load($value->nid);
	    $output .= '<li>';
	    $output .= l('<h2>' . $node->title . '</h2>' . check_markup($node->teaser, $node->format, FALSE ) . '<span class="ui-li-count">' . $node->comment_count . '</span>', 'node/' . $node->nid, array('title' => $node->title, 'html' => TRUE));
	    $output .= '</li>';
	    $num_rows = TRUE;
	  }
	  $output .= '</ul>';
  }
  else {
	  $output = '';
	  $num_rows = FALSE;
	  while ($node = db_fetch_object($result)) {
	    $output .= node_view(node_load($node->nid), 1);
	    $num_rows = TRUE;
	  }
  }

  if ($num_rows) {
    $feed_url = url('rss.xml', array('absolute' => TRUE));
    drupal_add_feed($feed_url, variable_get('site_name', 'Drupal') .' '. t('RSS'));
    $output .= theme('pager', NULL, variable_get('default_nodes_main', 10));
  }
  else {
    $default_message = t('<h1 class="title">Welcome to your new Drupal website!</h1><p>Please follow these steps to set up and start using your website:</p>');
    $default_message .= '<ol>';

    $default_message .= '<li>'. t('<strong>Configure your website</strong> Once logged in, visit the <a href="@admin">administration section</a>, where you can <a href="@config">customize and configure</a> all aspects of your website.', array('@admin' => url('admin'), '@config' => url('admin/settings'))) .'</li>';
    $default_message .= '<li>'. t('<strong>Enable additional functionality</strong> Next, visit the <a href="@modules">module list</a> and enable features which suit your specific needs. You can find additional modules in the <a href="@download_modules">Drupal modules download section</a>.', array('@modules' => url('admin/build/modules'), '@download_modules' => 'http://drupal.org/project/modules')) .'</li>';
    $default_message .= '<li>'. t('<strong>Customize your website design</strong> To change the "look and feel" of your website, visit the <a href="@themes">themes section</a>. You may choose from one of the included themes or download additional themes from the <a href="@download_themes">Drupal themes download section</a>.', array('@themes' => url('admin/build/themes'), '@download_themes' => 'http://drupal.org/project/themes')) .'</li>';
    $default_message .= '<li>'. t('<strong>Start posting content</strong> Finally, you can <a href="@content">create content</a> for your website. This message will disappear once you have promoted a post to the front page.', array('@content' => url('node/add'))) .'</li>';
    $default_message .= '</ol>';
    $default_message .= '<p>'. t('For more information, please refer to the <a href="@help">help section</a>, or the <a href="@handbook">online Drupal handbooks</a>. You may also post at the <a href="@forum">Drupal forum</a>, or view the wide range of <a href="@support">other support options</a> available.', array('@help' => url('admin/help'), '@handbook' => 'http://drupal.org/handbooks', '@forum' => 'http://drupal.org/forum', '@support' => 'http://drupal.org/support')) .'</p>';

    $output = '<div id="first-time">'. $default_message .'</div>';
  }
  drupal_set_title('');

  return $output;
}

