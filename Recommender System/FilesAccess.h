#ifndef FILES_ACCESS_H
#define FILES_ACCESS_H

#include <fstream>
#include <vector>
#include <string>

// renaming the 2-Dimensional vector structure to matrix. 
using matrix = std::vector<std::vector<float>>;

// function which gets documents and their corresponding categories.
void GetExistingUserData(const char* fileName, matrix& existingUserRating, std::vector<std::string>& movieList);

void GetNewUserData(const char* fileName, matrix& newUserRating);

#endif FILES_ACCESS_H