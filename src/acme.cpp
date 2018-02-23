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


#include "acme.h"

ACME::ACME(std::istream &is,
           std::ostream& os,
           unsigned minLength,
           unsigned maxLength,
           unsigned minFrequency,
           unsigned maxDistance,
           unsigned mode) :
           is(is),
           os(os),
           minLength(minLength),
           maxLength(maxLength),
           minFrequency(minFrequency),
           maxDistance(maxDistance),
           mode(mode) {

  // read input stream to a sequence
  std::string line;
  seqan::String<char> seq;
  while(this->is >> line) { // for every new line
    seqan::append(seq, line); // append line to seq
  }

  // construct suffix tree from sequence
  STIndex stIndex(seq);

  // construct cast search object
  this->search = cast::Search(stIndex, this->maxDistance);

  // init statistics vector
  this->statistics = std::vector<unsigned>(101); // Motifs are not expected to be longer than 100

  // print motif models start message
  if (this->mode == 2) {
    this->os << "# Motif Models (motif : frequency : [list of occurrences]):\n";
  }

  // perform search
  ACME::CASTMotifsExtraction();

  // print statistics
  ACME::printStat();

  return;
}

// Check if the current motif (branch) is valid according to our model
// current search branch is valid if it the motif length is in range [min_lengt, maxLength]
// and the frequency is at least minFrequency
bool ACME::isValid() {
  if (cast::getLength(this->search) <= this->maxLength &&
      cast::getLength(this->search) >= this->minLength &&
      cast::getFrequency(this->search) >= this->minFrequency) {
    return true;
  } else {
    return false;
  }
}

// Check if the current motif (branch) is promising, i.e. if is interesting.
bool ACME::isPromising() {
  if (cast::getFrequency(this->search) >= this->minFrequency &&
      cast::getLength(this->search) <= this->maxLength) {
    return true;
  } else {
    return false;
  }
}

// Search and output approximate motifs using cache aware search space
// traversal (CAST).
void ACME::CASTMotifsExtraction() {
  // perform DFS in postorder
  while (true) {
    // if current search branch is promising
    // then continue search down the tree
    if(ACME::isPromising()) {
      cast::goDown(this->search);
      continue;
    } else {
      // else if current search branch is not promising
      //  if can go right then go right
      //  else, repeat go up and output valid motifs until can go right
      while(!cast::goRight(this->search)) {
        // if can not go up, then we must be back at root and end DFS
        if(!cast::goUp(this->search)) {
          return;
        }
        // if motif is valid print motif and add info to statistics
        if (ACME::isValid()) {
          ACME::printMotif();
          ACME::collectStat();
        }
      }
    }
  }
  return;
}

// Print the current search branch (motif).
void ACME::printMotif() {
  // do not print if silent mode is set
  if (this->mode < 2) return;
  // print motif and frequency
  this->os << cast::getMotif(this->search);
  this->os << " ";
  this->os << cast::getFrequency(this->search);
  this->os << " ";
  // print occurrences
  this->os << "[";
    for (auto occurrence : cast::getOccurrences(this->search)) {
    this->os << " " << occurrence;
    }
  this->os << " ]";
  this->os << "\n";
}

// Add the current search branch (motif) to the statistics.
void ACME::collectStat() {
  // if mode statistics is set
  if (this->mode < 1) return;
  // add current search branch (motif) to the statistics
  unsigned length = cast::getLength(this->search);
  if (length >= 100) length = 100; // statistics vector is 101 long
  this->statistics[length] += 1;
}

// Print the statistics.
void ACME::printStat() {
  // if mode statistics is set
  if (this->mode < 1) return;
  // print the statistics
  this->os << "# Statistics (motif length : number of motifs): \n";
    int sum = 0;
    for (size_t i = 0; i < this->statistics.size(); ++i) {
      if (this->statistics[i] != 0) {
      this->os << "# " << i << " : " << this->statistics[i] << "\n";
        sum += this->statistics[i];
      }
    }
  this->os << "# total number of motifs: " << sum << "\n";
}
