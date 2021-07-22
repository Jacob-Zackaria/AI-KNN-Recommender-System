#include "FilesAccess.h"

void GetExistingUserData(const char* fileName, matrix& existingUserRating, std::vector<std::string>& movieList)
{
	//open file to read.
	std::ifstream myFile(fileName);

	// string to store from file.
	char* newString = new char[1000];
	 
	//  { movie names }
	myFile.getline(newString, 1000);

	// store in process string.
	std::string processString(newString);

	// find ',' character. { csv file format }
	size_t pos = processString.find(',');

	// truncate the string. { skipping first column }
	processString = processString.substr(pos + 1);

	// find ',' character.
	pos = processString.find(',');

	// iterate until process string is empty.
	while (pos != -1)
	{
		// store value temporary string.
		movieList.push_back(processString.substr(0, pos));

		// truncate the string.
		processString = processString.substr(pos + 1);

		// find next ',' character.
		pos = processString.find(',');

		if (pos == -1)
		{
			// store value temporary string.
			movieList.push_back(processString);
		}
	}

	// temporary structure.
	std::vector<float> temporaryFloat = {};

	// unwanted value 
	*newString = 'x';

	// read until existing user are done.
	while (*newString != ',')
	{
		// clear temporary.
		temporaryFloat.clear();

		// read line { user ratings }
		myFile.getline(newString, 1000);

		if (*newString != ',')
		{
			// store in process string.
			processString = newString;

			// find ',' character.
			pos = processString.find(',');

			// truncate the string. { skipping first column users name }
			processString = processString.substr(pos + 1);

			// find ',' character.
			pos = processString.find(',');

			// iterate until process string is empty.
			while (pos != -1)
			{
				// store value temporary float.
				temporaryFloat.push_back(std::stof(processString.substr(0, pos)));

				// truncate the string.
				processString = processString.substr(pos + 1);

				// find next '\t' character.
				pos = processString.find(',');

				if (pos == -1)
				{
					// store value temporary int.
					temporaryFloat.push_back(std::stof(processString));
				}
			}

			// store temporary vector to new matrix vector.
			existingUserRating.push_back(temporaryFloat);
		}
	}

	//delete storage.
	delete[] newString;

	// close file.
	myFile.close();
}

void GetNewUserData(const char* fileName, matrix& newUserRating)
{
	//open file to read.
	std::ifstream myFile(fileName);

	// string to store from file.
	char* newString = new char[1000];

	// skip first line { movie names }
	myFile.getline(newString, 1000);

	// store in process string.
	std::string processString = {};

	// find ',' character. { csv file format }
	size_t pos = 0;

	// unwanted value 
	*newString = 'x';

	while (*newString != ',')
	{
		// read line { skip existing user ratings }
		myFile.getline(newString, 1000);
	}

	// temporary structure.
	std::vector<float> temporaryFloat = {};

	// read until file is empty.
	while (!myFile.eof())
	{
		// clear temporary.
		temporaryFloat.clear();

		// read line { new user ratings }
		myFile.getline(newString, 1000);

		if (*newString != '\0')
		{
			// store in process string.
			processString = newString;

			// find ',' character.
			pos = processString.find(',');

			// truncate the string. { skipping first column users name }
			processString = processString.substr(pos + 1);

			// find ',' character.
			pos = processString.find(',');

			// iterate until process string is empty.
			while (pos != -1)
			{
				// store value temporary float.
				temporaryFloat.push_back(std::stof(processString.substr(0, pos)));

				// truncate the string.
				processString = processString.substr(pos + 1);

				// find next ',' character.
				pos = processString.find(',');

				if (pos == -1)
				{
					// store value temporary float.
					temporaryFloat.push_back(std::stof(processString));
				}
			}

			// store temporary vector to new matrix vector.
			newUserRating.push_back(temporaryFloat);
		}
	}

	//delete storage.
	delete[] newString;

	// close file.
	myFile.close();
}
