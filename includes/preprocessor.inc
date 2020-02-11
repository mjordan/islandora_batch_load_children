<?php

/**
 * @file
 * Implementation of preprocessor class for the batch ingest framework.
 */

/**
 * Batch preprocessor class.
 */
class IslandoraBatchLoadChildrenPreprocessor extends IslandoraBatchPreprocessor {

  /**
   * Generates objects from input files and adds them to the batch queue.
   */
  public function preprocess() {
    module_load_include('inc', 'islandora_batch_load_children', 'includes/utilities');

    $input_file = $this->parameters['input_file'];
    $pid_path_pairs = islandora_batch_load_children_read_input_file($input_file);

    var_dump($pid_path_pairs);

    $parent_label =  islandora_batch_load_children_get_parent_label($pid_path_pair['parent_pid']);
    $child_identifier = '12345667890';
    $mods_xml = islandora_batch_load_children_generate_mods($child_title, $child_identifier);
    print $mods_xml;

    // Accumulate a list of the objects to be added to the queue.
    $added = array();

    // Check each subdirectory for the existence of files to use as OBJ datastreams.
    // If found, add it to the batch set.
    foreach ($pid_path_pairs as $pid_path_pair) {
      $child_obj_filenames = array_diff(scandir($pid_path_pair['child_input_path']), array('.', '..'));
      // natsort($child_obj_filenames);
      var_dump($child_obj_filenames);

      foreach ($child_obj_filenames as $child_obj_filename) {
        // Assemble the full path to the file.
        $child_obj_path = $pid_path_pair['child_input_path'] . '/' . $child_obj_filename;
        // Check to see if file exists.
        // If so, create Batch object.
        // $batch_object = new IslandoraBatchLoadChildrenObject($this->connection, $pid_path_pair['parent_pid'], $child_obj_path, $this->parameters);
        if ($this->parameters['namespace']) {
          $this->id = $this->getId($batch_object);
        }
        else {
          $parent_namespace = islandora_batch_load_children_get_parent_namespace($pid_path_pair['parent_pid']);
        }
        // Add to database and $added[]:
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
    }
    return $added;
  }
}
