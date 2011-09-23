README.txt
==========

To use the module in your theme simply add the following code to template.php:

function THEME_preprocess_page(&$vars) {
  if (module_exists('jquerymobile_ui')) {
     $vars['jqm_scripts'] = jquerymobile_ui_load_files($vars); 
  }
}

Then create a <head> section like the following in your page.tpl.php file:

<head> 
  <meta name="viewport" content="width=device-width, initial-scale=1"> 
  <title><?php print $head_title; ?></title>
  <?php print $styles; ?>
  <?php print $scripts; ?>
  <?php if (!empty($jqm_scripts)) { print $jqm_scripts; } ?>
</head> 
