////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2017 Jonas Spenger
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////
//
// This is an implementation of the CAST (cache aware search space traversal)
// algorithm as proposed in:
//  Sahli, Majed, Essam Mansour, and Panos Kalnis. "ACME: A scalable parallel
//  system for extracting frequent patterns from a very long sequence."
//  The VLDB Journal 23.6 (2014): 871-893.
// The authors describe CAST as:
//  "a representation of the search space trie together with access methods,
//  that is specifically designed to be cache efficient by exhibiting spatial and
//  temporal locality. For spatial locality, CAST utilizes an array of symbols
//  to recover all branches sharing the same prefix."
// and as:
//  "a cache-aware method for solving the combinatorial repeated motifs problem.
//  CAST arranges the suffix tree in continuous memory blocks and accesses it
//  in a way that exhibits spatial and temporal locality, thus minimizing cache
//  misses"
//
// In short, this implementation allows for efficient retrieval of approximate
// motifs (from a suffix tree) through providing an easy to use interface for
// traversing (efficiently) over the search space trie (all possible motifs).
// The (imaginary) trie can be traversed by the methods:
//  goDown()
//  goRight()
//  goUp().
// The frequency, the length and the list of all occurrences of the current
// motif (search branch / node) is updated with every step (as well as the motif
// itself) and can be retrieved by the methods:
//  getFrequency()
//  getLength()
//  getOccurrences()
//  getMotif()
//
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"
#include "branchelement.h"
#include <seqan/index.h>
#include <vector>

namespace cast {

struct Search {
  public:
    Search(STIndex& stIndex,
           const double maxDistance,
           std::function<double(char, char)> distMeasure);
    Search() {}
    double maxDistance;
    STIndex stIndex;
    seqan::String<unsigned> pmFrequency;
    seqan::String<unsigned> pmLength;
    std::vector<BranchElement> branchVector;
    std::vector<char> alphabet;
    unsigned alphabetSize;
    std::function<double(char, char)> distMeasure;
};

bool goRight(Search& search);
bool goDown(Search& search);
bool goUp(Search& search);
std::vector<unsigned> getOccurrences(Search& search);
unsigned getFrequency(Search& search);
unsigned getLength(Search& search);
seqan::String<char> getMotif(Search& search);

}  // namespace cast
