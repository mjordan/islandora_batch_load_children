<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Sets child sequences for a batch set.
 *
 * @param array $sequences
 *   Associative array containing child sequences for all objects in the current batch set.
 */
function islandora_batch_load_children_set_child_sequences($sequences) {
  // Add the current set's sequences to existing values in the
  // islandora_batch_load_children variable. In other words, merge the array
  // in that variable with $sequences. We want to do this so we don't overwrite
  // existing values.
  $existing_sequences = variable_get('islandora_batch_load_children_child_sequences', array());
  $new_sequences = array_merge($sequences, $existing_sequences);
  variable_set('islandora_batch_load_children_child_sequences', $new_sequences);
}

/**
 * Gets child sequences for a batch set.
 *
 * @param string $path
 *   The path to the child object's file.
 *
 * @return array
 *   The child sequence array for the specified set, or an empty array if variable has not been set.
 */
function islandora_batch_load_children_get_child_sequence($path) {
  // Pick out the sequences for the current PID. Use a static cache so we're not
  // hammering the db.
  static $sequences;
  if (!isset($sequences)) {
    $sequences = variable_get('islandora_batch_load_children_child_sequences', array());
  }
  return $sequences[$path];
}

/**
 * Reads the input directory containing child OBJ files for a single parent.
 *
 * @param string $path
 *   The absolute path to the child object's input directory.
 *
 * @return array
 *   A list of files in the directory, minus . and .., indexes reset, and natsorted.
 */
function islandora_batch_load_children_get_child_files($path) {
  $child_obj_filenames = array_diff(scandir($path), array('.', '..'));
  // Reset index keys.
  $child_obj_filenames = array_values($child_obj_filenames);
  // Use PHP's natsort(). @todo: Maybe provide option to use other sorts?
  natsort($child_obj_filenames);
  return $child_obj_filenames;
}

/**
 * Determines content model based on file extension.
 *
 * @param string $extension
 *   The file extension of the OBJ datastream.
 *
 * @return string
 *   A content model PID.
 */
function islandora_batch_load_children_get_cmodel($extension) {
  $map = array(
    'jpeg' => 'islandora:sp_basic_image',
    'jpg' => 'islandora:sp_basic_image',
    'gif' => 'islandora:sp_basic_image',
    'png' => 'islandora:sp_basic_image',
    'tif' => 'islandora:sp_large_image_cmodel',
    'tiff' => 'islandora:sp_large_image_cmodel',
    'jp2' => 'islandora:sp_large_image_cmodel',
    'pdf' => 'islandora:sp_pdf',
    'mp3' => 'islandora:sp-audioCModel',
    'mp4a' => 'islandora:sp-audioCModel',
    'm4a' => 'islandora:sp-audioCModel',
    'oga' => 'islandora:sp-audioCModel',
    'ogg' => 'islandora:sp-audioCModel',
    'flac' => 'islandora:sp-audioCModel',
    'wav' => 'islandora:sp-audioCModel',
    'mp4' => 'islandora:sp_videoCModel',
    'm4v'  => 'islandora:sp_videoCModel',
    'mkv'  => 'islandora:sp_videoCModel',
    'mpeg' => 'islandora:sp_videoCModel',
    'mpe' => 'islandora:sp_videoCModel',
    'mpg' => 'islandora:sp_videoCModel',
    'qt' => 'islandora:sp_videoCModel',
    'mov' => 'islandora:sp_videoCModel',
    'ogv' => 'islandora:sp_videoCModel',
  );
  return $map[$extension];
}

/**
 * Gets the object label from the parent.
 *
 * @param string $parent_pid
 *   The parent object's PID.
 *
 * @return bool|string
 *   A label for the object, or if the parent cannot be loaded, FALSE.
 */
function islandora_batch_load_children_get_parent_label($parent_pid) {
  $parent = islandora_object_load($parent_pid);
  if ($parent) {
    return $parent->label;
  }
  else {
    // return '[Parent label not available]';
    return FALSE;
  }
}

/**
 * Gets the namespace from the parent.
 *
 * @param string $parent_pid
 *   The parent object's PID.
 *
 * @return string
 *   The namespace.
 */
function islandora_batch_load_children_get_parent_namespace($parent_pid) {
  return preg_replace('/:.*$/', '', $parent_pid);
}

/**
 * Generates DC XML from MODS XML via XSLT.
 *
 * @param string $xml
 *   An XML string.
 * @param string $path_to_xslt
 *   Path to the XSLT stylesheet.
 *
 * @return string
 *   The transformed DC XML.
 */
function islandora_batch_load_children_get_dc_from_mods($xml, $path_to_xslt) {
  $xsl_doc = new DOMDocument();
  $xsl_doc->load($path_to_xslt);
  $xml_doc = new DOMDocument();
  $xml_doc->loadXML($xml);
  $xslt_proc = new XSLTProcessor();
  $xslt_proc->importStylesheet($xsl_doc);
  $dc_xml = $xslt_proc->transformToXML($xml_doc);
  return $dc_xml;
}

/**
 * Returns a list of parent PID / child input directory pairs.
 *
 * @param string $input_file_path
 *   The absolute path to the PID file.
 *
 * @return array
 *   A list of PID / input directory pairs.
 */
function islandora_batch_load_children_read_input_file($input_file_path) {
  if (!file_exists($input_file_path)) {
    drush_set_error('FILE_NOT_FOUND',
    dt('The specified input file (!file) does not exist.',
      array('!file' => $input_file_path)));
    drupal_exit();
  }

  $pairs = array();
  $lines = file($input_file_path, FILE_IGNORE_NEW_LINES);
  foreach ($lines as $pair) {
    list($pid, $child_input_path) = explode(',', $pair);
    // Skip commented out rows.
    if (!preg_match('!(#|//)!', $pair)) {
      $pairs[] = array('parent_pid' => $pid, 'child_input_path' => $child_input_path);
    }
  }
  return $pairs;
}

/**
 * Returns a simple MODS XML file for use with a child object.
 *
 * @param string $child_title
 *   The title to assign to the child.
 * @param string $child_identifier
 *   The identifier to assign to the child.
 *
 * @return string
 *   The MODS XML.
 */
function islandora_batch_load_children_generate_mods($child_title, $child_identifier) {
  $child_mods_xml = <<<EOQ
<mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <titleInfo>
    <title>{$child_title}</title>
  </titleInfo>
  <identifier type="local">{$child_identifier}</identifier>
</mods>
EOQ;

  return $child_mods_xml;
}

/**
 * Generate title of child object.
 *
 * @param string $parent_label
 *   The parent object's label.
 * @param string $child_title_label
 *   The child title label supplied as a Drush option.
 * @param string $sequence
 *   The child's sequence number.
 *
 * @return string
 *   The MODS XML.
 */
function islandora_batch_load_children_generate_child_title($parent_label, $child_title_label, $sequence) {
  $child_title = $parent_label . ', ' . $child_title_label . ' ' . $sequence;
  return $child_title;
}

/**
 * Checks for the existence of parent objects and child files.
 *
 * @param array $options
 *   Associative array containing some options supplied on the Drush command line.
 */
function islandora_batch_load_children_check_input($options) {
  $pid_path_pairs = islandora_batch_load_children_read_input_file($options['input_file']);

  // Check each subdirectory for the existence of files to use as OBJ datastreams.
  // If found, create a new object for each and add it to the batch set.
  foreach ($pid_path_pairs as $pid_path_pair) {
    $parent_label = islandora_batch_load_children_get_parent_label($pid_path_pair['parent_pid']);
    $child_obj_filenames = islandora_batch_load_children_get_child_files($pid_path_pair['child_input_path']);
    foreach ($child_obj_filenames as $index => $filename) {
      $child_obj_path = $pid_path_pair['child_input_path'] . '/' . $filename;
      if (is_file($child_obj_path)) {
        $sequence = $index + 1;
        if ($options['namespace']) {
          $namespace = $options['namespace'];
        }
        else {
          $namespace = islandora_batch_load_children_get_parent_namespace($pid_path_pair['parent_pid']);
        }
        if ($parent_label) {
          $child_title = islandora_batch_load_children_generate_child_title($parent_label, $options['child_title_label'], $sequence);
        }
        else {
          $child_title = NULL;
        }
        if ($child_title) {
          $message = t("@path will be used to create child of @parent using namespace '@namespace' and title '@child_title'.", array('@path' => $child_obj_path, '@parent' => $pid_path_pair['parent_pid'], '@namespace' => $namespace, '@child_title' => $child_title));
          drush_log($message, "ok");
          if ($options['log_file']) {
            islandora_batch_load_children_write_log($options['log_file'], 'ok', $message);
          }
        }
        else {
          $message = t("Parent object @pid not found or not accessible.", array('@pid' => $pid_path_pair['parent_pid']));
          drush_log($message, "error");
          if ($options['log_file']) {
            islandora_batch_load_children_write_log($options['log_file'], 'error', $message);
          }
        }
      }
      else {
        $message = t("File @path is not a regular file.", array('@path' => $child_obj_path));
        drush_log($message, "error");
          if ($options['log_file']) {
            islandora_batch_load_children_write_log($options['log_file'], 'error', $message);
          }
      }
    }
  }
}

/**
 * Writes the log file.
 *
 * @param string $path
 *   The path to the log file.
 * @param string $status
 *   'ok', 'error', etc.
 * @param string $message
 *   The message to write to the log.
 */
function islandora_batch_load_children_write_log($path, $status, $message) {
  $entry = $status . "\t" . trim($message);
  file_put_contents($path, $entry . PHP_EOL, FILE_APPEND);
}

