# Islandora Batch Load Children

Islandora batch module for ingesting objects that are children of other objects.


## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Batch](https://github.com/Islandora/islandora_batch)

## Usage

Enable this module, then run its drush command to import objects:

`drush --user=admin islandora_batch_load_children_preprocess --namespace=mynamespace --cmodel=xxx`

Then, to perform the ingest:

`drush --user=admin islandora_batch_ingest`

## Preparing Islandora for ingesting

## Preparing your content files for ingesting


Some points to note:

### Example input directories

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Support, feedback, and development

Feel free to open issues in this Github repo. Use cases and suggestions are welcome, as are pull requests (but before you open a pull request, please open an issue).

## License

 [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
