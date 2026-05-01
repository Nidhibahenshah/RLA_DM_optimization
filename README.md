# Optimized Double Metaphone Blocking for Record Linkage

This repository provides an optimized implementation of the Double Metaphone Blocking Algorithm (DMBA) within the RLA2 framework for scalable record linkage.

This work extends the baseline implementation by introducing **system-level optimizations in both phonetic encoding and block construction**, significantly improving runtime performance while preserving linkage quality.

*Note:* The source code of RLA-CL (Record Linkage Algorithm using Complete Linkage Clustering),
that this package is based on,
could be found at the following figshare link.

Source Code. figshare. Software. https://doi.org/10.6084/m9.figshare.2300302.v1

## How to install

### Cloning the repository

```shell
git clone https://github.com/<your-username>/optimized-dmba.git
cd optimized-dmba
```

### Required packages

Use the following command to install all required packages.

```shell
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

* Navigate to the `<dataset>` tag in the config file.
* The `<value>` component of dataset has a path to the file where dataset to be linked is stored.
* You need to have at least two dataset files for the program to run correctly.
  However, if you supply only one dataset file, the program will automatically proceed with a deduplication process.

### Setting up the blocking parameters

Use the `<blocking-stages>` tag to configure and apply one or more blocking stages.
Each blocking stage applies a specific blocking technique.
The supported blocking techniques (as of version 2.0) are described below along with their associated parameters.

#### K-mer blocking:

For K-mer based blocking, use the `<kmer-blocking>` tag.
The associated parameters are as follows:

* `<index>` `<value>` constitutes one or more index(es) for the blocking field(s).
* `<kmer>` the k-mer blocking value(s).
* `<type>` the data type of the blocking field which is one of the following options:

  * 0 for Alphabet only fields.
  * 1 for Numeric only fields.
  * 2 for Alphanumeric fields.
  * 3 for ASCII fields.

#### Multi-blocking

Multi blocking is a blocking technique that utilizes multiple fields for blocking key construction.
Blocking keys are constructed by concatenating select prefixes from select blocking fields.
For Multi blocking, there are two versions with two different tags namely,
`<multi-blocking>` and `<multi-blocking-2>`.

The associated parameters are as follows:

* `<field> <index>` one blocking field index or multiple blocking fields indexes separated by commas.
* `<field> <length>` associated field prefix length(s) to be used for blocking key construction.

##### Example:

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
Field Value:     01235243   John   Smith  
```

---

#### Double Metaphone Blocking (Optimized)

A blocking technique that utilizes **Double Metaphone phonetic encoding** of select blocking field(s) for blocking key construction.
This implementation includes **optimized encoding and efficient block construction**.

Blocking key construction process:

1. For each record, apply Double Metaphone encoding on each selected blocking field
2. Use the generated phonetic code (primary code)
3. Concatenate the phonetic codes to form the final blocking key

Optimizations include:

* Constant-time character classification using lookup tables
* Boundary check elimination via input padding
* Structured pattern matching for phonetic rules
* Hash-based block construction
* Vector-based insertion for efficiency
* Deferred deduplication of block entries
* Caching of phonetic encodings to avoid redundant computation

The associated parameters are as follows:

* `<doublemetaphone-blocking>` `<field>` `<index>` specifies index(es) for blocking fields
* `<doublemetaphone-blocking>` `<field>` `<length>` specifies prefix length (optional usage depending on implementation)

##### Example

```xml
<doublemetaphone-blocking>
	<field>
		<index>1</index>
		<length>4</length>
	</field>
</doublemetaphone-blocking>
<doublemetaphone-blocking>
	<field>
		<index>2</index>
		<length>4</length>
	</field>
</doublemetaphone-blocking>
```

Example record:

```
Field Index:     0              1            2          
Field Value:     001101557      Smith        John
```

Generated blocking key (example):

```
From field 1 : SM0
From field 2 : JN
```

---

### Setting up the matching parameters

Inside the **ComparisonGroup** tag of the configuration file there's a property called `<dist_calc_method>`.
The `<dist_calc_method>` tag specifies which distance metric to employ during the matching process.

Supported distance functions:

* Edit Distance
* Reversal Distance
* Truncate Distance
* Q-Gram Distance
* Hausdorff Distance

The **comparing_attribute_indices** property specifies the position of the column from the input dataset.

**threshold property** is used to determine similarity.

**priority property** specifies the column used for clustering decisions.

### Logging of results

Results are saved in `results.csv` by default.

To modify:

```
results_logging/filename/value
```

Or via command line:

```
--log <filename>
```

---

# How to run

```shell
$ ./rla2 <config_path>
```

or

```shell
$ rla2 <config_path>
```

---

## Summary

This repository provides an optimized implementation of Double Metaphone Blocking that significantly improves runtime efficiency while maintaining high linkage quality, making it suitable for large-scale record linkage applications.
