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
// This is an implementation of the ACME motifs extraction algorithm as
// proposed in:
//  Sahli, Majed, Essam Mansour, and Panos Kalnis. "ACME: A scalable parallel
//  system for extracting frequent patterns from a very long sequence."
//  The VLDB Journal 23.6 (2014): 871-893.
//
// ACME uses CAST (cache aware search space traversal) a cache aware method
// for the repeated motifs problem. In short, this implementation allows the
// user to retrieve the set of approximate motifs of a sequence. The isValid
// method differs from the original ACME algorithm, in that this implementation
// does not check if a motif is left-maximal and right-maximal.
//
// The parameters that the user must provide are:
//  is: the input stream
//  os: the output stream
//  min_length: the minimum length of a valid motif
//  max_length: the maxmimum length of a valid motif
//  min_frequency: the mimimum frequency (no. of occurrences) of a valid motif
//  max_distance: the maximum hamming distance between an occurrence and the
//  motif mode: choose between silent, statistics and motif. motif will output
//  all valid motifs
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "common.h"
#include "cast.h"
#include <seqan/index.h>
#include <iostream>

class ACME{
private:
  std::istream& is;
  std::ostream& os;
  unsigned minLength;
  unsigned maxLength;
  unsigned minFrequency;
  unsigned maxDistance;
  unsigned mode; // mode 0, 1, 2: silent, statistics, motif
  std::vector<unsigned> statistics;
  cast::Search search;
  void CASTMotifsExtraction();
  bool isValid();
  bool isPromising();
  void printMotif();
  void collectStat();
  void printStat();
public:
  ACME(std::istream& is,
       std::ostream& os,
       unsigned min_length,
       unsigned max_length,
       unsigned min_frequency,
       unsigned max_distance,
       unsigned mode);
};
