
#include "RecommenderSystem.h"

int main()
{
	// input file. { 0 means 'nan' rating }
	const char* inputFile = "knn-csc480-a4.csv";

	// gets training data from input file (Users U1 - U20)
	RecommenderSystem::GetTrainingData(inputFile);

	// gets test data from input file (New Users N1-N5)
	RecommenderSystem::GetTestData(inputFile);

	// mean absolute error table for each k-value { pair = k-value, mean absolute error value }
	std::vector<std::pair<uint32_t, float>> meanAbsoluteErrorTable = {};

	// print all movie list.
	RecommenderSystem::PrintMovieList();

	// predict exisitng rating of new users and calculate mean absolute error for each k-value.
	RecommenderSystem::PredictExistingRating(meanAbsoluteErrorTable);

	// calculate best k-value from mean absolute error table.
	const uint32_t k = RecommenderSystem::ComputeBest_K_ValueFromErrorTable(meanAbsoluteErrorTable);

	// predict rating of new users for movies not rated with the best k-value.
	RecommenderSystem::PredictNewRating(k);

	// structure to store input from user for movie recommendations.
	UserInput newInput = {};

	// read users input for movie recommendation.
	while (RecommenderSystem::ReadUserInput(newInput))
	{
		// generate recommendation based on user input.
		RecommenderSystem::GenerateRecommendations(newInput);
	}

	// success.
	return 0;
}