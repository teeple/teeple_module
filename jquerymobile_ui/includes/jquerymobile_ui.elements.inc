<?php
/**
 * Implementation of hook_elements().
 *
 * This defines a new form element types.
 *
 * - jquerymobile_ui_textfield: This is actually just a textfield, but provides
 *   the new type. If more were to be done with it a theme function could be
 *   provided.
 * - jquerymobile_ui_checkbox: Nothing more than a regular checkbox, but uses
 *   an alternate theme function provided by this module.
 * - jquerymobile_ui_phonenumber_discrete: Provides a North-American style
 *   three-part phonenumber where the value of the phonenumber is managed
 *   as an array of three parts.
 * - jquerymobile_ui_phonenumber_combined: Provides a North-American style
 *   three-part phonenumber where the actual value is managed as a 10-digit
 *   string and only broken up into three parts for the user interface.
 *
 * See hook_elements() and the
 * @link http://drupal.org/node/169815 Creating Custom Elements @endlink
 * handbook page.
 */
function _jquerymobile_ui_elements() {
  $types['jquerymobile_ui_checkbox'] = array(
    '#input' => TRUE,
    '#return_value' => TRUE,
  ); 
  $types['jquerymobile_ui_checkboxes'] = array(
    '#input' => TRUE,
    '#return_value' => TRUE,
  );
  $types['jquerymobile_ui_select'] = array(
    '#input' => TRUE,
    '#return_value' => TRUE,
  );   
  $types['jquerymobile_ui_textfield'] = array(
    '#input' => TRUE,
    //'#theme' => array('textfield'),
    '#autocomplete_path' => FALSE,
  );
  
}

/**
 * Helper function to determine the value for an jquerymobile_ui_checkbox.
 *
 * Required for the element type 'jquerymobile_ui_checkbox' to work.
 * Copied from form.inc.
 *
 * @param $form
 *   The form element whose value is being populated.
 * @param $edit
 *   The incoming POST data to populate the form element. If this is FALSE,
 *   the element's default value should be returned.
 * @return
 *   The data that will appear in the $form_state['values'] collection
 *   for this element. Return nothing to use the default.
 */
function form_type_jquerymobile_ui_checkbox_value($form, $edit = FALSE) {
  if ($edit !== FALSE) {
    if (empty($form['#disabled'])) {
      return !empty($edit) ? $form['#return_value'] : 0;
    }
    else {
      return $form['#default_value'];
    }
  }
}

/**
 * Helper function to determine the value for a checkboxes form element.
 *
 * @param $form
 *   The form element whose value is being populated.
 * @param $edit
 *   The incoming POST data to populate the form element. If this is FALSE,
 *   the element's default value should be returned.
 * @return
 *   The data that will appear in the $form_state['values'] collection
 *   for this element. Return nothing to use the default.
 */
function form_type_jquerymobile_ui_checkboxes_value($form, $edit = FALSE) {
  if ($edit === FALSE) {
    $value = array();
    $form += array('#default_value' => array());
    foreach ($form['#default_value'] as $key) {
      $value[$key] = 1;
    }
    return $value;
  }
  elseif (!isset($edit)) {
    return array();
  }
}


/**
 * Helper function to determine the value for a textfield form element.
 *
 * @param $form
 *   The form element whose value is being populated.
 * @param $edit
 *   The incoming POST data to populate the form element. If this is FALSE,
 *   the element's default value should be returned.
 * @return
 *   The data that will appear in the $form_state['values'] collection
 *   for this element. Return nothing to use the default.
 */
function form_type_jquerymobile_ui_textfield_value($form, $edit = FALSE) {
  if ($edit !== FALSE) {
    // Equate $edit to the form value to ensure it's marked for
    // validation.
    return str_replace(array("\r", "\n"), '', $edit);
  }
}

/**
 * Helper function to determine the value for a select form element.
 *
 * @param $form
 *   The form element whose value is being populated.
 * @param $edit
 *   The incoming POST data to populate the form element. If this is FALSE,
 *   the element's default value should be returned.
 * @return
 *   The data that will appear in the $form_state['values'] collection
 *   for this element. Return nothing to use the default.
 */
function form_type_jquerymobile_ui_select_value($form, $edit = FALSE) {
  if ($edit !== FALSE) {
    if (isset($form['#multiple']) && $form['#multiple']) {
      return (is_array($edit)) ? drupal_map_assoc($edit) : array();
    }
    else {
      return $edit;
    }
  }
}
