# ACME
This is a reimplementation of the serial execution (not the parallel) (and not restricted to left or right-maximal motifs) algorithm (ACME / CAST Motifs Extraction) proposed in:
> Sahli, Majed, Essam Mansour, and Panos Kalnis. "ACME: A scalable parallel system for extracting frequent patterns from a very long sequence." The VLDB Journal 23.6 (2014): 871-893.

## Prerequisites
* [SeqAn](https://github.com/seqan/seqan) - The Library for Sequence Analysis (v. 2.3.2 tested)
* Boost
* CMake

## Installing
```
cd build;
cmake -DCMAKE_BUILD_TYPE=Release ..;
make;
cd ..;
```
or
```
sh build.sh;
```

## Testing
Run test script:
```
sh test.sh;
```

## Examples
Find and print all approximate motifs (output: motif : frequency : [list of occurrences])
of inputfilename.txt having minimum frequency 2 (at least 2 occurrences in inputfilename)
and maximum distance 1 (the approximate matches are at most hamming distance 1 from the motif)
from the motif.
```
./build/ACME -i test/test_sequence.txt -f 2 -d 1
```

For more information:
```
./build/ACME -h
```

## Author
* Jonas Spenger

## License
* This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments
* The software was developed as part of the Bachelor's thesis in 2017 at Humboldt University of Berlin.
