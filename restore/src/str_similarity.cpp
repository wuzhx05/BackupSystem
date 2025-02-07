/// @file str_similarity.cpp
/// @brief Contains implementations of string similarity algorithms including Levenshtein distance and Jaro-Winkler distance.

// source: https://www.geeksforgeeks.org/jaro-and-jaro-winkler-similarity/
// source: https://www.geeksforgeeks.org/introduction-to-levenshtein-distance/

#include <algorithm>
#include <cmath>
#include <vector>

#include "str_similarity.hpp"

namespace strsimilarity {
using std::floor;
using std::max;
using std::min;
using std::vector;
double levenshteinFullMatrix(const string &str1, const string &str2) {
    int m = str1.length();
    int n = str2.length();

    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));

    for (int i = 0; i <= m; i++)
        dp[i][0] = i;
    for (int j = 0; j <= n; j++)
        dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else { // insert, remove, replace
                dp[i][j] =
                    1 + std::min(dp[i][j - 1],
                                 std::min(dp[i - 1][j], dp[i - 1][j - 1]));
            }
        }
    }
    // dp[m][n]: min edit distance
    return 1 -
           static_cast<double>(dp[m][n]) / max(str1.length(), str2.length());
}
double jaro_distance(string s1, string s2) {
    // If the strings are equal
    if (s1 == s2)
        return 1.0;

    // Length of two strings
    int len1 = s1.length(), len2 = s2.length();

    // Maximum distance upto which matching
    // is allowed
    int max_dist = floor(max(len1, len2) / 2) - 1;

    // Count of matches
    int match = 0;

    // Hash for matches
    int hash_s1[s1.length()] = {0}, hash_s2[s2.length()] = {0};

    // Traverse through the first string
    for (int i = 0; i < len1; i++) {

        // Check if there is any matches
        for (int j = max(0, i - max_dist); j < min(len2, i + max_dist + 1); j++)

            // If there is a match
            if (s1[i] == s2[j] && hash_s2[j] == 0) {
                hash_s1[i] = 1;
                hash_s2[j] = 1;
                match++;
                break;
            }
    }

    // If there is no match
    if (match == 0)
        return 0.0;

    // Number of transpositions
    double t = 0;

    int point = 0;

    // Count number of occurrences
    // where two characters match but
    // there is a third matched character
    // in between the indices
    for (int i = 0; i < len1; i++)
        if (hash_s1[i]) {

            // Find the next matched character
            // in second string
            while (hash_s2[point] == 0)
                point++;

            if (s1[i] != s2[point++])
                t++;
        }

    t /= 2;

    // Return the Jaro Similarity
    return (((double)match) / ((double)len1) +
            ((double)match) / ((double)len2) +
            ((double)match - t) / ((double)match)) /
           3.0;
}
} // namespace strsimilarity