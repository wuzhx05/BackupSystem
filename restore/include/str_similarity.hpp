// source: https://www.geeksforgeeks.org/jaro-and-jaro-winkler-similarity/
// source: https://www.geeksforgeeks.org/introduction-to-levenshtein-distance/

#pragma once
#ifndef _STR_SIMILARITY_HPP_
#define _STR_SIMILARITY_HPP_

#include <string>
namespace str_similarity {
using std::string;
double jaro_distance(string s1, string s2);
double levenshteinFullMatrix(const string &str1, const string &str2);

} // namespace str_similarity

#endif