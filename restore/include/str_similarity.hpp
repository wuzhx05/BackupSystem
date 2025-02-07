/// @file str_similarity.hpp
/// @brief Header file for string similarity algorithms including Levenshtein distance and Jaro-Winkler distance.

#pragma once
#ifndef _STR_SIMILARITY_HPP_
#define _STR_SIMILARITY_HPP_

#include <string>
namespace strsimilarity {
using std::string;

/// @brief Calculates the Jaro similarity between two strings.
/// @param s1 The first string.
/// @param s2 The second string.
/// @return A similarity score between 0 and 1, where 1 means identical strings.
double jaro_distance(string s1, string s2);

/// @brief Calculates the Levenshtein distance between two strings and normalizes it to a similarity score.
/// @param str1 The first string.
/// @param str2 The second string.
/// @return A similarity score defined as "jaro_distance / max(len(str1), len(str2))" which is between 0 and 1.
double levenshteinFullMatrix(const string &str1, const string &str2);

} // namespace strsimilarity

#endif