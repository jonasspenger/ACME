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


#pragma once

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <boost/math/distributions/normal.hpp>

namespace {
double hamming(char a, char b) {
  if (a != b) {
    return 1.0;
  } else {
    return 0.0;
  }
}
double saxmindist(char a, char b, double lookup[256][256]) {
  return lookup[int(a)][int(b)];
}
} // end anonymous namespace

namespace distanceFunction {
std::function<double(char, char)> getFunction(const std::string dm, const std::string alphabet) {

  std::function<double(char, char)> measure;

  if (dm == "hamming") {

    using namespace std::placeholders;
    // return this function
    measure = std::bind(hamming, _1, _2);

  } else if (dm == "saxmindist") {

    using namespace std::placeholders;

    std::size_t alphabetSize = alphabet.length();

    double lookup[256][256];

    // initialize with zeroes
    for (int i = 0; i< 256;  i++) {
      for (int j = 0; j< 256;  j++) {
        lookup[i][j] = 0.0;
      }
    }

    // calculate breakpoints
    std::vector<double> breakpoints(alphabetSize - 1);
    boost::math::normal stdNormDist(0.0, 1.0); // standard normal distribution
    for (size_t i = 1; i < alphabetSize; ++i) {
      double q = quantile(stdNormDist, ((double) i) / ((double) alphabetSize));
      breakpoints[i-1] = q;
    }

    // add values to lookup table
    for (char a : alphabet) {
      for (char b : alphabet) {
        std::size_t pos_a = alphabet.find(a);
        std::size_t pos_b = alphabet.find(b);
        if (pos_a > pos_b) {
          double dist = breakpoints[pos_a - 1] - breakpoints[pos_b];
          lookup[int(a)][int(b)] = dist;
        } else if (pos_a < pos_b) {
          double dist = breakpoints[pos_b - 1] - breakpoints[pos_a];
          lookup[int(a)][int(b)] = dist;
        } else { // pos_a equals pos_b
          lookup[int(a)][int(b)] = 0.0;
        }
      }
    }

    // return this function
    measure = std::bind(saxmindist, _1, _2, lookup);
  }

  return measure;
}
}
