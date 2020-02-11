<?php

/**
 * @file
 * Utility functions.
 */

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
 * @return string
 *   A label for the object.
 */
function islandora_batch_load_children_get_parent_label($parent_pid) {
  $parent = islandora_object_load($parent_pid);
  if ($parent) {
    return $parent->label;
  }
  else {
    return '[Parent label not available]';
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
