<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_batch_load_children_drush_command() {
  $items = array();

  $items['islandora_batch_load_children_preprocess'] = array(
    'description' => 'Preprocess Islandora objects with derivatives into batch queue entries.',
    'drupal dependencies' => array('islandora_batch'),
    'options' => array(
      'namespace' => array(
        'description' => 'The namespace for objects created by this command. If not specified, objects inherit the namespace of their parent.',
        'value' => 'optional',
      ),
      'parent_relationship_uri' => array(
        'description' => 'The namespace URI of the relationship to the parent. Defaults to "info:fedora/fedora-system:def/relations-external#".',
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => 'The predicate of the relationship to the parent. Defaults to "isConstituentOf".',
        'value' => 'optional',
      ),
      'create_dc' => array(
        'description' => "If true, the DC datastream will be generated from the MODS.xml or MAds.xml key datastream. If false, the DC datastream will be generated from the object's label and PID.",
        'value' => 'required',
      ),
      'content_models' => array(
        'description' => 'The content model to use for objects that do not have an OBJ derivative file. If there is an OBJ file, the provided content model overrides the automatically assigned one.',
        'value' => 'optional',
      ),
      'input_file' => array(
        'description' => "Absolute path to a comma-delimited file containing parent PID / input path pairs.",
        'value' => 'required',
        'required' => TRUE,
      ),
      'child_title_label' => array(
        'description' => "Text that will be used in child objects' titles to indicate their relationship to their parent, e.g., 'part', 'page', etc. If not specified, defaults to 'part'. For values that contain spaces, wrap the value in quotation marks.",
        'value' => 'required',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $items;
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_batch_load_children_preprocess() {
  // Due to how Drush bootstrapping works, the connection may be created
  // without credentials (when your site's front page is
  // 'islandora/object/some:object', for example). Resetting to ensure a new
  // connection gets created should fix it.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', 'info:fedora/fedora-system:def/relations-external#'),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isConstituentOf'),
    'create_dc' => drush_get_option('create_dc', TRUE),
    'input_file' => drush_get_option('input_file'),
    'content_models' => drush_get_option('content_models', NULL),
    'namespace' => drush_get_option('namespace', NULL),
    'child_title_label' => drush_get_option('child_title_label', 'part'),
  );
  // We need to cache the value of the 'namespace' option because we populate
  // it dynamically in the preprocessor. See note there for more detail.
  $parameters['namespace_from_user'] = drush_get_option('namespace', NULL);

  // Run the preprocessor.
  $preprocessor = new IslandoraBatchLoadChildrenPreprocessor($connection, $parameters);
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  drush_log(t("SetId: @s", array('@s' => $preprocessor->getSetId())), "ok");
}
