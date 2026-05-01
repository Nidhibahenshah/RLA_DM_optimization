# Record Linkage with Soundex Blocking

In this enhanced record linkage package, several blocking techniques are supported.
Most notably, a soundex-based blocking has been added recently.

_Note:_ The source code of RLA-CL (Record Linkage Algorithm using Complete Linkage Clustering)
,that this package is based on,
could be found at the following figshare link.

Source Code. figshare. Software. https://doi.org/10.6084/m9.figshare.2300302.v1


## How to install

### Cloning the repository

```shell
git clone https://anonymous.4open.science/r/RLA2-Soundex-533D.git
cd RLA2-soundex
```

### Required packages

Use the following command to install all required packages.

``` shell
apt install libxml2-dev \
libboost-regex-dev \
libmpich-dev \
libboost-log-dev \
libyaml-cpp-dev
```

### How to build

```shell
mkdir cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install # optionally install into /usr/local/bin
```

## Setting up the configuration file

Before running any linkage process, a configuration file should be prepared (see `config.xml` for a template).
Such `xml` configuration file is designed to hold all necessary parameters required by the linkage process.
Below is an overview that shows how to prepare a configuration file for a linkage process.
Note: you might want to copy one of the template configuration files and modify it according to your linkage application.

### Setting up the datasets
 
*  Navigate to the `<dataset>` tag in the config file.
*  The `<value>` component of dataset has a path to the file where dataset to be linked is stored.
*  You need to have at least two dataset files for the program to run correctly. 
   However, if you supply only one dataset file, the program will automatically proceed with a deduplication process. 

### Setting up the blocking parameters

Use the ``<blocking-stages>`` tag to configure and apply one or more blocking stages. 
Each blocking stage applies a specific blocking technique. 
The supported blocking techniques (as of version 2.0) are described below along with their associated parameters.

#### K-mer blocking:
For K-mer based blocking, use the ``<kmer-blocking>`` tag.
The associated parameters are as follows:
 - ``<index>`` ``<value>`` constitutes one or more index(es) for the blocking field(s).
 - ``<kmer>`` the k-mer blocking value(s).
 - ``<type>`` the data type of the blocking field which is one of the following options:
   - 0 for Alphabet only fields.
   - 1 for Numeric only fields.
   - 2 for Alphanumeric fields.
   - 3 for ASCII fields.

#### Multi-blocking

Multi blocking is a blocking technique that utilizes multiple fields for blocking key construction. 
Blocking keys are constructed by concatenating select prefixes from select blocking fields.
For Multi blocking, there are two versions with two different tags namely,
``<multi-blocking>`` and ``<multi-blocking-2>``.

The associated parameters are as follows:
 - ``<field> <index>``  one blocking field index or multiple blocking fields indexes separated by commas.
 - ``<field> <length>`` associated field prefix length(s) to be used for blocking key construction.

##### Example:
When the following multi blocking settings are being applied to the following sample record, the blocking key is `JoS`.

```xml
<multi-blocking>
	<field>
		<index>1,2</index>
		<length>2,1</length>
	</field>
</multi-blocking>
```

Sample record:
```shell
	Field Index:     0         1      2   
	Field Value:	01235243   John   Smith  
```


#### Soundex blocking

A blocking technique that utilizes soundex encoding of select blocking field(s) for blocking key construction. 
Use ``<soundex-blocking>`` tag to apply the soundex blocking technique on the dataset to be linked. 
The blocking key is constructed as follows. 
First, for each record, apply soundex encoding on each of the blocking field values. 
Then, truncate the generated soundex codes to the specified length (see below). 
Finally, concatenate the truncated soundex codes (in the designated order) to form the final blocking key for each record.
The associated parameters are as follows:
 - ``<soundex-blocking>`` ``<field>`` ``<index>`` specifies index(es) for one or more blocking fields.
 - ``<soundex-blocking>`` ``<field>`` ``<index>`` specifies the truncation length for generated soundex codes.

##### Example
Blocking key `Z400A535` is generated after applying below configuration
```xml
	<soundex-blocking>
		<field>
			<index>1,2</index>
			<length>4,4</length>
		</field>
	</soundex-blocking>
```

on the record shown below.
```
Field Index:     0              1            2          3            4    
Field Value:	001101557      	zuallo       antonio    05001986     06131890
```

Note that `Z400` and `A535` are the soundex codes (truncated to 4 characters with zero padding) for `zuallo` and `antonio` field values, respectively.


### Setting up the matching parameters

Inside the **ComparisonGroup** tag of the configuration file there's a property called `<dist_calc_method>`. 
The `<dist_calc_method>` tag specifies which distance metric to employ during the matching process.
Please use any of the following `RLA2` supported distance functions:
 - Edit Distance
 - Reversal Distance
 - Truncate Distance
 - Q-Gram Distance
 - Hausdorff Distance

The **comparing_attribute_indices** property mentions the position of the column from the input dataset
that is being used by the algorithm. (Incorrect values would lead to un-defined behaviour)

**threshold property** is used to identify whether to consider a string pair as similar/dissimilar.

**priority property**, like the comparing attribute indices,
specify the location of the column in input data that would be used for complete clustering.
The priority fields are used to break ties during split and merge operations of the complete linkage phase.

### Logging of results

The results of the linkage job is saved in `results.csv` file by default in the working directory.
However, one can set a different name in the configuration file.
See XML node: `results_logging/filename/value` and the associated comments.
Also, this can be overridden by specifying the name for logging the results via one of the command line switches `-L` or `--log`.


# How to run

Now, that you have your linkage configuration file ready, you can start the linkage process by invoking the following command.
```shell
$ ./rla2 <config_path>
```

or 

```shell
$ rla2 <config_path>   # if installed (see above)
```

This program takes only one argument which is the path to the xml configuration file.
The configuration file should be prepared with all parameters required for running the linkage process.

The `rla2` executable supports the following program options 
that further controls the program behavior as detailed below.

```shell
rla2 --help
[2024-02-28 20:28:06.920154] [0x00007ff8485417c0] [info]    RLA2
Linking Configuration Settings:
  --help                        displays help message
  -L [ --log ] arg              set file name for logging results
  -C [ --complete-linkage ] arg Whether to apply complete linkage
  --output-blocks arg           Whether to output the blocking information to a
                                log file.
  --limit-records arg           Limit the total number of records to be read 
                                from input dataset files.RLA2 parses same 
                                number of records from all files.
```
 - The argument for `--log` option is of type string that represents the file path to be used for storing 
                         the linkage results. Default is `results.xml`.
 - The argument for `--complete-linkage` option is of type boolean which controls whether to apply complete linkage
                        after the single linkage phase is over. It could be set using `1|0` or `on|off` or `true|false`.
                        Complete linkage is turned off by default.
 - The argument for `--output-blocks` option is of type boolean that controls logging of generated blocks to special
                        blocks logging file(s) for investigation.
                        Logging of blocking information is turned off by default.
 - The argument for `--limit-records` is an integer which indicates the maximum total number of records 
                        to be read from input dataset files.
                        The default behavior is to read all records from all input dataset files in case
                        this option is not included in the command line invocation.

## Example 1

To run the linkage job detailed by the parameters of the configuration file `config.xml` 
and limit the number of records to 50 000 records, invoke the following command.

```shell
$ rla2 config.xml --limit-records 50000
```

## Example 2

To run the above linkage job with complete linkage while saving blocking information.

```shell
$ rla2 config.xml --limit-records 50000 --complete-linkage on --output-blocks on
```

## Running a demo

### Data preparation

First, run the following commands to fetch a couple of sample dataset files from figshare into the data directory.

```shell
mkdir -p data 
pushd data
wget "https://ndownloader.figshare.com/files/34655951" -O "ds4.1.1" -nv --show-progress
wget "https://ndownloader.figshare.com/files/34655954" -O "ds4.1.2" -nv --show-progress 
popd
```

Then, copy the config file to the `experiments` directory.

```shell
mkdir -p experiments 
cp config.xml experiments/
```

Finally, start the linkage process

```shell
pushd experiments
rla2 config.xml 
```

Now, edit the configuration file and re-run the linkage process.

All detailed results could be found in `results.csv` file.
Additionally, more detailed linkage performance report could be found in the `ds4OutSingle_detailed_performance` file.

