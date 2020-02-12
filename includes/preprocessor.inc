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

    // Accumulate a list of the objects to be added to the batch set.
    $added = array();

    // Check each subdirectory for the existence of files to use as OBJ datastreams.
    // If found, create a new object for each and add it to the batch set.
    foreach ($pid_path_pairs as $pid_path_pair) {
      $child_obj_filenames = array_diff(scandir($pid_path_pair['child_input_path']), array('.', '..'));

      foreach ($child_obj_filenames as $child_obj_filename) {
        $child_obj_path = $pid_path_pair['child_input_path'] . '/' . $child_obj_filename;
        // Check to see if file exists.
        // If so, create Batch object for it.
        $batch_object = new IslandoraBatchLoadChildrenObject($this->connection, $pid_path_pair['parent_pid'], $child_obj_path, $this->parameters);
        // The IslandoraBatchPreprocessor base class's getId() method uses 
        // $this->parameters['namespace']. We need to populate it dynamically
        // here to manage the namespace used in that method, so we rely on a
        // cached copy of the value of the --namespace option provided by the user.
        if ($this->parameters['namespace_from_user']) {
          $this->parameters['namespace'] = $this->parameters['namespace_from_user'];
          $batch_object->id = $this->getId($batch_object);
        }
        else {
          $this->parameters['namespace'] = islandora_batch_load_children_get_parent_namespace($pid_path_pair['parent_pid']);
          $batch_object->id = $this->getId($batch_object);
        }
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
    }
    return $added;
  }
}
