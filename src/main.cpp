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
#include "distance.hpp"
#include <seqan/arg_parse.h>
#include <seqan/basic.h>
#include <iostream>
#include "occurrenceelement.h"

int main(int argc, char const ** argv) {

  //////////////////////////////////////////////////////////////////////////////
	// Handle user input and implement help method.
  //////////////////////////////////////////////////////////////////////////////

  // Setup argumentParser.
  seqan::ArgumentParser parser("ACME");

  // Set short description, version, date and licensing and copyright information.
  seqan::setShortDescription(parser, "Motifs Extraction");
  seqan::setVersion(parser, "1.0");
  seqan::setDate(parser, "August 2017");

  // Define usage line and long description.
  seqan::addUsageLine(parser,
    "[-i \\fIFILENAME\\fP | -s] [\\fIOPTIONS\\fP]");
  seqan::addDescription(parser,
    "This is an implementation of the ACME motifs extraction algorithm as "
    "proposed in:\n"
    "Sahli, Majed, Essam Mansour, and Panos Kalnis. \"ACME: A scalable parallel"
    "system for extracting frequent patterns from a very long sequence.\" "
    "The VLDB Journal 23.6 (2014): 871-893.\n"
    "This implementation differs from the original ACME algorithm, in that "
    "this implementation does not check if a motif is left-maximal and/or "
    "right-maximal.");

  // Add examples section.
  seqan::addTextSection(parser, "Examples");
  seqan::addListItem(parser,
    "\\fB"
    "ACME"
    "\\fP "
    "-i \\fIinputfilename.txt\\fP "
    "\\fB-f\\fP \\fI2\\fP "
    "\\fB-d\\fP \\fI1\\fP ",
    "Finds and prints all approximate motifs (output: motif : frequency : [list of occurrences]) "
    "of inputfilename.txt having "
    "minimum frequency 2 (at least 2 occurrences in inputfilename) and \n\r"
    "maxmimum distance 1 (the approximate matches are at most hamming distance 1 from the motif) from the motif.");

  // Define options.
  // The filename of the input sequence.
  seqan::addOption(parser, seqan::ArgParseOption(
    "i", "input", "Input file name.",
    seqan::ArgParseArgument::INPUT_FILE, "FILENAME"));
    seqan::setValidValues(parser, "input", "txt");

  seqan::addOption(parser, seqan::ArgParseOption(
    "s", "stream", "Read data from standard input stream instead of input file."));

  // The frequency threshold.
  seqan::addOption(parser, seqan::ArgParseOption(
    "f", "frequency", "Minimum frequency (no. of occurrences) of an approximate motif.",
    seqan::ArgParseArgument::INTEGER, "INT"));
  seqan::setDefaultValue(parser, "f", "2");
  seqan::setMinValue(parser, "f", "1");

  // The distance threshold.
  seqan::addOption(parser, seqan::ArgParseOption(
    "d", "distance", "Maximum allowed distance for approximate matches (hamming distance if not otherwise specified).",
    seqan::ArgParseArgument::DOUBLE, "DOUBLE"));
  seqan::setDefaultValue(parser, "d", "0");
  seqan::setMinValue(parser, "d", "0");

  // The minimum motif length.
  seqan::addOption(parser, seqan::ArgParseOption(
    "minl", "minlength", "Minimum length of a motif.",
    seqan::ArgParseArgument::INTEGER, "INT"));
  seqan::setDefaultValue(parser, "minl", "1");
  seqan::setMinValue(parser, "minl", "1");

  // The maximum motif length.
  seqan::addOption(parser, seqan::ArgParseOption(
    "maxl", "maxlength", "Maximum length of a motif.",
    seqan::ArgParseArgument::INTEGER, "INT"));
  seqan::setDefaultValue(parser, "maxl", "20");
  seqan::setMinValue(parser, "maxl", "1");

  // The mode of what should be output (motif, statistics or silent).
  seqan::addOption(parser, seqan::ArgParseOption(
    "m", "mode", "Select the mode of the output.", seqan::ArgParseArgument::STRING, "STR"));
  seqan::setValidValues(parser, "m", "silent statistics motif");
  seqan::setDefaultValue(parser, "m", "motif");

  // If SAX MINDIST should be used
  seqan::addOption(parser, seqan::ArgParseOption(
    "sm", "saxmindist", "If selected, uses the SAX MINDIST instead of the Hamming distance. "
    "The SAX alphabet must be passed as a string in the correct order.",
    seqan::ArgParseArgument::STRING, "STR"));

  // Hide the version-check option in help.
  seqan::hideOption(parser, "version-check");

  // Parse the command line.
  seqan::ArgumentParser::ParseResult res = seqan::parse(parser, argc, argv);

  // If parsing was not successful then exit with code 1 if there were errors.
  // Otherwise, exit with code 0 (e.g. help was printed).
  if (res != seqan::ArgumentParser::PARSE_OK)
    return res == seqan::ArgumentParser::PARSE_ERROR;
  if (seqan::isSet(parser, "input") && seqan::isSet(parser, "stream")) {
    std::cerr << seqan::getAppName(parser) << ": you cannot specify both -input and -stream." << "\n";
    return seqan::ArgumentParser::PARSE_ERROR;
  }
  if (!seqan::isSet(parser, "input") && !seqan::isSet(parser, "stream")) {
    std::cerr << seqan::getAppName(parser) << ": you must specify either -input or -stream." << "\n";
    return seqan::ArgumentParser::PARSE_ERROR;
  }
  if (seqan::isSet(parser, "saxmindist")) {
    // check for only unique characters in alphabet
    std::string a;
    seqan::getOptionValue(a, parser, "saxmindist");
    for (std::size_t i = 0; i < a.length(); i++) {
      for (std::size_t j = i+1; j < a.length(); j++) {
        if (a[i] == a[j]) {
          std::cerr << seqan::getAppName(parser) << ": the alphabet can only consist of unique characters." << "\n";
          return seqan::ArgumentParser::PARSE_ERROR;
        }
      }
    }
  }


  // Extract option values.
  seqan::CharString filename; // input file name
  seqan::getOptionValue(filename, parser, "input");
  unsigned frequency = 0; // minimum frequency (no. of occurrence) of approximate matches
  seqan::getOptionValue(frequency, parser, "frequency");
  double distance = 0.0; // maximum hamming distance of approximate matches to motif
  seqan::getOptionValue(distance, parser, "distance");
  unsigned minLength = 0; // minimum motif length
  seqan::getOptionValue(minLength, parser, "minlength");
  unsigned maxLength = 0; // maximum motif length
  seqan::getOptionValue(maxLength, parser, "maxlength");
  seqan::CharString modeOpt = "";
  seqan::getOptionValue(modeOpt, parser, "mode");
  unsigned mode;
  if (modeOpt == "silent") mode = 0;
  if (modeOpt == "statistics") mode = 1;
  if (modeOpt == "motif") mode = 2;
  std::function<double(char, char)> distMeasure;
  if (seqan::isSet(parser, "saxmindist")) {
    std::string alphabet;
    seqan::getOptionValue(alphabet, parser, "saxmindist");
    distMeasure = distanceFunction::getFunction("saxmindist" ,alphabet);
  } else {
    distMeasure = distanceFunction::getFunction("hamming", "");
  }



  //////////////////////////////////////////////////////////////////////////////
  // Load sequence
  //////////////////////////////////////////////////////////////////////////////

  // Set istream is to either be from input filename or from standard input stream.
  std::ifstream ifs;
  std::istream* is = nullptr;
  if (seqan::isSet(parser, "input")) {
    ifs.open(seqan::toCString(filename));
    if (!ifs.is_open()) {
      std::cerr << seqan::getAppName(parser) << ": the given path '" << filename << "' was not opened." << "\n";
      return seqan::ArgumentParser::PARSE_ERROR;
    }
    is = &ifs;
  } else if (seqan::isSet(parser, "stream")) {
    is = &std::cin;
  }


  //////////////////////////////////////////////////////////////////////////////
  // Motif discovery.
  //////////////////////////////////////////////////////////////////////////////

  // input stream, output stream, min_length, max_length, min_frequency, max_distance, mode, distMeasure
  ACME::ACME(*is, std::cout, minLength, maxLength, frequency, distance, mode, distMeasure);

  return 0;
}
