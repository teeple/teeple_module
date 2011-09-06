<?php
// $Id: views_examples.pages.inc,v 1.1 2010/03/08 18:51:57 kepol Exp $

/**
 * @file
 * Provides infrequently used pages for Views Examples.
 */

/**
 */
function views_examples_help_text($path, $arg) {
  $output = '';
  switch ($path) {
    case 'admin/help#views_examples':
      $output = 'fixme';
      break;
  }
  return $output;
}
