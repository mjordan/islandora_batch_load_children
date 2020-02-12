# Islandora Batch Load Children

Islandora batch module for ingesting objects that are children of other objects. The original use case for this module was to load fold-out "pages" in a digitized scrapbook. The scrapbook itself was ingested using the book content model, and the fold-outs were then ingested using this module as children of the book pages they were attached to.

## Usage

This module only provides a Drush script. There is no graphical user interface.

To load child objects, prepare their content as described in the next section, and then run the `islandora_batch_load_children_preprocess` Drush command, specifying a user with sufficient priviledges to create Islandora objects, and specifying an input file:

`drush --user=admin islandora_batch_load_children_preprocess  --input_file=/tmp/test.txt`

Run `drush --user=admin islandora_batch_load_children_preprocess  --help` for information on other options.


## Preparing the child files

This module uses a comma-delimited input file to associate parent objects with children. Child object files (TIFFs, PDFs, video files, etc.) for each parent listed in the input file need to be located in a directory and named so that the sequence of the children can be determined by sorting the filenames. For example, given an input file like this:

```
islandora:100,/tmp/100
islandora:300,/tmp/300
```

Files in `/tmp/100` will be ingested so they are children of the parent with PID `islandora:100`. If the files are named

```
foo_01.tif
foo_02.tif
```

`foo_01.tif` will be the first child and `foo_02.tif` will be the second child.

## Metadata

A minimal MODS XML document is generated for each child object, containing a title and a local identifier derived from the child object's filename.

By default, child objects inherit their parent's title, with 'part X' appended. For example, if children are added to a parent whose title it 'Sample object', the children will get the titles 'Sample object, part 1', Sample object, part 2', etc. You can override 'part' with the `'child_title_label` Drush option:

`drush --user=admin islandora_batch_load_children_preprocess  --input_file=/tmp/test.txt --child_title_label=segment`

will result in children with titles 'Sample object, segment 1', 'Sample object, segment 2', etc.


## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Batch](https://github.com/Islandora/islandora_batch)

## Usage

Enable this module, then run its drush command to import objects:

`drush --user=admin islandora_batch_load_children_preprocess --namespace=mynamespace --input_file=/tmp/input.txt --template=/tmp/MODS.xml`

Then, to perform the ingest:

`drush --user=admin islandora_batch_ingest`


## Preparing your content files for ingesting


Some points to note:

### Example input directories

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Support, feedback, and development

Feel free to open issues in this Github repo. Use cases and suggestions are welcome, as are pull requests (but before you open a pull request, please open an issue).

## License

 [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
