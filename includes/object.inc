<?php

/**
 * @file
 * Implementation of classes from the batch ingest framework.
 */

/**
 * Batch object class.
 */
class IslandoraBatchLoadChildrenObject extends IslandoraBatchObject {
  protected $objFilePath;
  protected $preprocessorParameters;

  /**
   * Constructor.
   */
  public function __construct(IslandoraTuque $connection, $parent_pid, $obj_datastream_path, $batch_parameters) {
    parent::__construct(NULL, $connection->repository);
    $this->parentPid = $parent_pid;
    $this->objectContentPath = $obj_datastream_path;
    $this->preprocessorParameters = $batch_parameters;
    if ($this->preprocessorParameters['create_dc']) {
      $this->modsToDcTransform = drupal_get_path('module', 'islandora_batch') . '/transforms/mods_to_dc.xsl';
    }
  }

  /**
   * Assemble the datastreams and relationships.
   */
  public function batchProcess() {
    module_load_include('inc', 'islandora_batch_load_children', 'includes/utilities');

    $datastream = $this->getDatastreams();
    $this->ingestDatastream($datastream);

    $parent_label = islandora_batch_load_children_get_parent_label($this->parentPid);
    $child_identifier = pathinfo($this->objectContentPath, PATHINFO_FILENAME);
    $child_title = $parent_label . ', ' . $this->preprocessorParameters['child_title_label'] . ' x';

    $mods_xml = islandora_batch_load_children_generate_mods($child_title, $child_identifier);
    $mods = $this->constructDatastream('MODS', 'X');
    $mods->mimetype = 'application/xml';
    $mods->label = 'MODS Record';
    $mods->setContentFromString($mods_xml);
    $this->ingestDatastream($mods);

    if ($this->preprocessorParameters['create_dc'] === TRUE) {
      $dc_content = islandora_batch_load_children_get_dc_from_mods($mods_xml, $this->modsToDcTransform);
      $dc = $this->constructDatastream('DC', 'X');
      $dc->mimetype = 'text/xml';
      $dc->label = 'Dublin Core Record for this object';
      $dc->setContentFromString($dc_content);
      $this->ingestDatastream($dc);
    }

    $this->addRelationships();
    $this->inheritXacmlPolicies();

    // Indicate that this object is ready to be ingested.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Add relationships.
   */
  public function addRelationships() {
    module_load_include('inc', 'islandora_batch_load_children', 'includes/utilities');
    $child_identifier = pathinfo($this->objectContentPath, PATHINFO_FILENAME);

    // Get content model from OBJ file extension. Or, if the content model is
    // provided as a Drush option, use it instead.
    if (is_null($this->preprocessorParameters['content_models'])) {
      $ext = pathinfo($this->objectContentPath, PATHINFO_EXTENSION);
      $this->models = islandora_batch_load_children_get_cmodel($ext);
    }
    // A content model was provided as a Drush option. Use it instead.
    else {
       $this->models = $this->preprocessorParameters['content_models'];
    }

    // Add relationship to parent.
    $sequences = islandora_batch_load_children_get_child_sequences($this->parentPid);
    // @todo: get sequence number for $child_identifier from $sequences.
    $sequence_number = '1';

    $rels_predicate = $this->preprocessorParameters['parent_relationship_pred'];
    $rels_predicate_uri = $this->preprocessorParameters['parent_relationship_uri'];
    $this->relationships->add($rels_predicate_uri, $rels_predicate, $this->parentPid);
    $escaped_parent_pid = str_replace(':', '_', $this->parentPid);
    $this->relationships->add(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_parent_pid", $sequence_number, RELS_TYPE_PLAIN_LITERAL);
  }

  /**
   * Add inheritXacmlFrom relationship.
   */
  protected function inheritXacmlPolicies() {
    if (module_exists('islandora_xacml_editor')) {
      $parent = islandora_object_load($this->parentPid);
      if ($parent) {
        islandora_xacml_editor_apply_parent_policy($this, $parent);
      }
    }
  }

  /**
   * Get a list of resources.
   */
  public function getResources() {
    return array();
  }

  /**
   * Creates OBJ datastream for the object based on filename.
   *
   * @return object
   *   The datastream to ingest with the current object.
   */
  public function getDatastreams() {
    $ds = $this->constructDatastream('OBJ', 'M');
    $mime_detector = new MimeDetect();
    $ds->mimetype = $mime_detector->getMimetype($this->objectContentPath);
    $ds->label = 'OBJ datastream';
    $ds->setContentFromFile($this->objectContentPath, FALSE);
    return $ds;
  }
}
