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


#include "cast.h"

namespace cast {

// Hide the decleration of extend in an anonymous namespace so that it cannot be
// accessed from other files.
namespace {

// Extend the current motif (search branch) by the character
// alphabet[alphabetIndex], i.e. move the search space trie node to the child
// with the character alphabet[alphabetIndex].
void extend(Search& search, unsigned alphabetIndex);
}  // anonymous namespace


// Move the search space trie node one step down, i.e. add the first character
// of the alphabet to the back of the motif.
bool goDown(Search& search) {
  extend(search, 0);
  return true;
}

// Move the search space trie node one step up, i.e. remove the last character
// of the motif.
bool goUp(Search& search) {
  if (search.branchVector.back().length != 0) {
    search.branchVector.pop_back();
    return true;
  } else { return false;}
}

// Move the search space trie node one step to the right, i.e. increment the
// last character of the motif.
bool goRight(Search& search) {
  if (search.branchVector.back().alphabetIndex != search.alphabetSize - 1 && // if last character of alphabet
      search.branchVector.back().alphabetIndex != search.alphabetSize) { // or special root symbol
    unsigned nextAlphabetIndex = search.branchVector.back().alphabetIndex + 1;
    goUp(search);
    extend(search, nextAlphabetIndex);
    return true;
  } else {return false;}
}

// Get the list of all occurrences of the current search branch (motif
// candidate, i.e the top of the branch vector).
std::vector<unsigned> getOccurrences(Search& search) {
  BranchElement branchElement = search.branchVector.back();
  std::vector<unsigned> ret;
  for (OccurrenceElement oe : branchElement.occurrenceVector) {
    for (auto occurrence : seqan::getOccurrences(oe.stIter)) {
      ret.push_back(occurrence);
    }
  }
  return ret;
}

// Get the frequency (number of occurrences) of the current search branch
// (motif candidate).
unsigned getFrequency(Search& search) {
  return search.branchVector.back().frequency;
}

// Get the length of subsequence represented by the current search branch
// (motif candidate).
unsigned getLength(Search& search) {
  return search.branchVector.back().length;
}

// Get the subsequence (i.e. the motif) represented by the current search
// branch (motif candidate).
seqan::String<char> getMotif(Search& search) {
  seqan::String<char> motif;
  for (size_t i = 1; i < search.branchVector.size(); ++i) { // character at position 0 is root, i.e. empty
    motif += search.alphabet[ search.branchVector[i].alphabetIndex ];
  }
  return motif;
}

// A representation of the search space trie (all possible motifs) and relevant
// parameters for the CAST algorithm.
Search::Search(STIndex& stIndex, const double maxDistance, std::function<double(char, char)> distMeasure) :
maxDistance(maxDistance), stIndex(stIndex) {
  this->distMeasure = distMeasure;
  // annotate suffix tree with the frequency (number of occurrences of
  // represented subsequence in sequence) and the length of each nodes
  // represented subsequence in property maps.
  // access the property maps through: this->pmFrequency and this->pmLength
  STIterator stIterator(stIndex);
  std::stack<STIterator> iterStack;
  seqan::goRoot(stIterator);   // move iterator to the root node.
  iterStack.push(stIterator);  // push root to stack, nonrecursive solution
  seqan::resizeVertexMap(this->pmFrequency, this->stIndex);
  seqan::resizeVertexMap(this->pmLength, this->stIndex);
  while(!iterStack.empty()) {  // DFS in preorder each node and assign property
    seqan::assignProperty(this->pmFrequency, seqan::value(iterStack.top()), seqan::countOccurrences(iterStack.top()));
    seqan::assignProperty(this->pmLength, seqan::value(iterStack.top()), seqan::repLength(iterStack.top()));
    STIterator topDown = iterStack.top();
    STIterator topRight = iterStack.top();
    iterStack.pop();
    if (seqan::goDown(topDown)) {
      iterStack.push(topDown);
    }
    if (seqan::goRight(topRight)) {
      iterStack.push(topRight);
    }
  }

  // generate the alphabet from the suffix tree, i.e. the alphabet over which
  // the search space traversal is performed.
  seqan::goRoot(stIterator); // move iterator to the root node
  seqan::goDown(stIterator); // move iterator down one step
  do { // visit all children of root and append leading character of edges to alphabet
    this->alphabet.push_back(seqan::parentEdgeFirstChar(stIterator));
  } while(seqan::goRight(stIterator));
  // calculate alphabet size
  this->alphabetSize = this->alphabet.size();

  // initialize the branch array
  seqan::goRoot(stIterator); // move iterator to the root node.
  std::vector<OccurrenceElement> occurrenceVector;
  occurrenceVector.push_back(OccurrenceElement{
    stIterator, // iterator of approximately matching node of suffix tree
    0.0 // distance of occurrence from current search_pair to motif pattern
  });
  // The branch vector containins branchelements. A branchelement represents
  // a node of the search space trie (i.e. a motif candidate), and contains
  // the frequency and the list of all occurrences of that motif candidate.
  // The branch vector contains the current search branch (motif) at the top,
  // and the branches of its ancestors.
  this->branchVector.push_back(BranchElement{
    this->alphabetSize, // one greater than largest index in alphabet - unique character representing root
    seqan::getProperty(pmFrequency, value(stIterator)), // number of leafs, i.e. no. of matches between root (empty pattern) and sequence
    0, // length of the search branch (motif)
    occurrenceVector
  });
}

namespace {

// Extend the search pattern by the character given by alphabetIndex.
// i.e. push a new branch element with alphabetIndex and the corresponding
// occurrenceelements.
void extend(Search& search, unsigned alphabetIndex) {
  // the current branch that we are expanding.
  BranchElement branchElement = search.branchVector.back();
  // the amount of occurrences of the current search branch (motif).
  unsigned newBranchFrequency = 0;
  // the new occurrenceVector
  std::vector<OccurrenceElement> occurrenceVector;
  // for each occurrence element that belongs to the branch element
  for (OccurrenceElement occurrenceElement: branchElement.occurrenceVector) {
    // if the length of the sequence represented by the occurrenceElement
    // is the same length as the search branch (motif)
    if ( branchElement.length == seqan::getProperty(search.pmLength, value(occurrenceElement.stIter))) {
      // if the occurrenceElement is a leaf, then we cannot expand and continue
      if (seqan::isLeaf(occurrenceElement.stIter)) {
        continue;
      }
      // expand the node of occurrenceElement to all its children
      STIterator child = occurrenceElement.stIter;
      seqan::goDown(child); // the left most child
      do { // visit all children
        // update the distance new_distance of child to the current search branch (motif)
        // if the first character of the traversed edge to child
        // does not equal the currently expanded character alphabetIndex
        // increment the new distance by 1 (or the distance given by distance measure)
        double new_distance = occurrenceElement.distance;
        if (seqan::parentEdgeFirstChar(child) != search.alphabet[alphabetIndex]) {
          new_distance += search.distMeasure(seqan::parentEdgeFirstChar(child), search.alphabet[alphabetIndex]);
        }
        // if the new_distance is less than the distance threshold
        // add the child as a new occurrence to occurrenceArray
        // and increase the newBranchFrequency accordingly
        if (new_distance <= search.maxDistance) {
          occurrenceVector.push_back(OccurrenceElement{child, new_distance});
          newBranchFrequency += seqan::getProperty(search.pmFrequency, value(child));
        }
      } while(seqan::goRight(child));

    // else if the length of the sequence represented by the occurrenceElement
    // is longer than the current search branch (motif)
    } else {
      double new_distance = occurrenceElement.distance;
      STIterator child = occurrenceElement.stIter;
      // if the expanded character (alphabetIndex) does not equal
      // the corresponding character of the edge from the occurrence node to child
      // increment the new distance by 1 (or the distance given by distance measure)
      if (seqan::parentEdgeLabel(child)[branchElement.length - seqan::parentRepLength(child)]
        != search.alphabet[alphabetIndex]) {
        new_distance += search.distMeasure(
          seqan::parentEdgeLabel(child)[branchElement.length - seqan::parentRepLength(child)],
          search.alphabet[alphabetIndex]
        );
      }
      // if the new_distance is less than the distance threshold
      // add the child as a new occurrence to occurrenceArray
      // and increase the newBranchFrequency accordingly
      if (new_distance <= search.maxDistance) {
        occurrenceVector.push_back(OccurrenceElement{child, new_distance});
        newBranchFrequency += seqan::getProperty(search.pmFrequency, value(child));
      }
    }
  }
  // push a new branch element to the back of the branch array
  search.branchVector.push_back(BranchElement{
    alphabetIndex, // the expanded character at alphabetIndex
    newBranchFrequency, // the number of approximate matches of the branch pattern (motif)
    branchElement.length + 1, // the length of the current branch pattern (motif)
    occurrenceVector
  });
  return;
}
} // anonymous namespace

}  // namespace cast
