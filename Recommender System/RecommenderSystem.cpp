#include "RecommenderSystem.h"
#include<algorithm>

RecommenderSystem* RecommenderSystem::getPrivateInstance()
{
	static RecommenderSystem instance;
	return &instance;
}

RecommenderSystem::RecommenderSystem()
	:
	existingUsersRating{},
	newUsersRating{},
	movieList{}
{
}

RecommenderSystem::~RecommenderSystem()
{
}

void RecommenderSystem::GetTrainingData(const char* inputFile)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();
	
	// read the existing users rating from input file. { row = users, column = movies }
	GetExistingUserData(inputFile, pRecommender->existingUsersRating, pRecommender->movieList);
}

void RecommenderSystem::GetTestData(const char* inputFile)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// read the new users rating from input file. { row = users, column = movies }
	GetNewUserData(inputFile, pRecommender->newUsersRating);
}

// get the nearest k-neighbours of target user.
void RecommenderSystem::GetNearestNeighbours(const UserInput& newInput, NearestNeighbour& nearestNeighbour)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// for each existing user.
	for (uint32_t i = 0; i < pRecommender->existingUsersRating.size(); i++)
	{
		// if target is an existing user and if current iteration user is equal to our target user
		if ((newInput.targetType == UserType::EXISTING_USER) && (newInput.targetIndex == i))
		{
			// skip the same exisitng user.
			continue;
		}

		// calculate pearson coefficient of target user with all existing users. 
		nearestNeighbour.push_back({ i, pRecommender->GetPearsonCoefficient(newInput.targetUser, pRecommender->existingUsersRating[i]) });
	}

	// sort all the vector pairs based on pearson coefficient { ie, similarity, similiar items are placed first }
	std::sort(nearestNeighbour.begin(), nearestNeighbour.end(), Comparator());
}

// predict movie ratings for new users according to user input and nearest neighbours { returns predicted value }.
const float RecommenderSystem::PredictRating(const UserInput& newInput, const NearestNeighbour& nearestNeighbour)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// sum of (product of (rating of users upto k and similarity of users upto k)).
	float numeratorVal = 0.0f;

	// sum of all similarities of users upto k
	float denominatorVal = 0.0f;

	// variable to store number of users rated out of  k-neighbours.
	uint32_t numberOfUsersRated = newInput.k;

	// using weighted average aproach of k-neighbours to predict the rating given by target user to movie.
	for (uint32_t i = 0; i < newInput.k; i++)
	{
		// filter out users who haven't rated the movie.
		if (pRecommender->existingUsersRating[nearestNeighbour[i].first][newInput.movieIndex] == 0.0f)
		{
			// decrement number of users rated.
			numberOfUsersRated--;

			// dont execute, this user.
			continue;
		}

		// calulate numerator of weighted average function. { sum(r(Ui, It) * sim(NU, Ui)) }
		numeratorVal += (pRecommender->existingUsersRating[nearestNeighbour[i].first][newInput.movieIndex] * nearestNeighbour[i].second);
		
		// calculate denominator of weighted average function. { sum (sim(NU, Ui)) }
		denominatorVal += nearestNeighbour[i].second;
	}

	float predictedRating = 0.0f;

	// check if there exist some users who rated the movie.
	if (numberOfUsersRated > 0)
	{
		// check if denominator is zero.
		if (denominatorVal != 0.0f)
		{
			// predicted rating.
			predictedRating = numeratorVal / denominatorVal;
		}
		else
		{
			// set to zero.
			predictedRating = 0.0f;
		}	
	}
	else
	{
		// if no users rated the movie. { return the average rating of movie across all exisitng users who rated the movie. }
		for (uint32_t i = 0; i < pRecommender->existingUsersRating.size(); i++)
		{
			// check if user rated the movie.
			if (pRecommender->existingUsersRating[i][newInput.movieIndex] > 0.0f)
			{
				// if yes add to average value.
				predictedRating += pRecommender->existingUsersRating[i][newInput.movieIndex];

				// increment count of users rated.
				numberOfUsersRated++;
			}
		}

		// check if any user rated.
		if (numberOfUsersRated != 0)
		{
			// take average { predicted rating = predicted rating / total number of user rated }
			predictedRating /= (float)numberOfUsersRated;
		}
		else
		{
			// set to zero, if no user rated.
			predictedRating = 0.0f;
		}
	}

	// if negative predicting occurs.
	if (predictedRating < 0.0f)
	{
		// reset to zero.
		predictedRating = 0.0f;
	}

	// return predicted value.
	return (predictedRating);
}

// generate recommendation based on user input.
void RecommenderSystem::GenerateRecommendations(const UserInput& newInput)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// variable to read number of recommendation.
	uint32_t numberOfRecommendation = 0;

	// vector of pearson coefficients for all existing users with their index. { pair : neighbour index, pearson coefficient }
	NearestNeighbour nearestNeighbour = {};

	// find nearest neighbour for new / existing user.
	pRecommender->GetNearestNeighbours(newInput, nearestNeighbour);

	while (true)
	{
		printf("\nEnter number of movie recommendations needed { 1 - %zu }:\n\n>> ", pRecommender->movieList.size());
		RETURN_IGNORE(scanf_s("%u", &numberOfRecommendation));

		if (numberOfRecommendation < 1 || numberOfRecommendation > pRecommender->movieList.size())
		{
			printf("\n Incorrect value or movie list size exceeded! Try again!");
		}
		else
		{
			break;
		}
	}

	// vector to store predicted ratings with user input. { pair = User input, predicted rating }
	std::vector<std::pair<UserInput, float>> recommendations;

	// temporary input variable.
	UserInput tempInput = newInput;

	printf("\nTop Recommended movies (which are not rated by the specified user):");
	// for each movie in the target users vector.
	for (uint32_t i = 0; i < newInput.targetUser.size(); i++)
	{
		// if movie is not rated by the user.
		if (newInput.targetUser[i] == 0.0f)
		{
			// set movie index.
			tempInput.movieIndex = i;

			// add recommendation with input data and predicted rating.
			recommendations.push_back( { tempInput, pRecommender->PredictRating(tempInput, nearestNeighbour) } );
		}
	}

	// rank each recommendations using predicetd rating. { decreasing order of predicted rating }
	std::sort(recommendations.begin(), recommendations.end(), RecommendationComparator());

	// if recommendations are less than given user need.
	if (recommendations.size() < numberOfRecommendation)
	{
		// set input to recommendation size.
		numberOfRecommendation = (uint32_t)recommendations.size();
	}

	// print all recommendations.
	for (uint32_t i = 0; i < numberOfRecommendation; i++)
	{
		// print recommendations.
		pRecommender->PrintUsersMovieRating(recommendations[i].first, recommendations[i].second);
	}
}

// read users input for movie recommendation.
bool RecommenderSystem::ReadUserInput(UserInput& newInput)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	uint32_t selectUser = 0;
	while (true)
	{
		printf("\n\nSelect User To Recommend Movies: { Press '0' to exit! }");
		printf("\n1) Existing User  2)New User\n\n>> ");

		RETURN_IGNORE(scanf_s("%u", &selectUser));

		if (selectUser < 0 || selectUser > 2)
		{
			printf("\n Sorry Wrong Target! Try again!");
		}
		else if (selectUser == 0)
		{
			return false;
		}
		else 
		{
			break;
		}
	}

	newInput.targetType = (UserType)(selectUser - 1);

	// variable to select target user.
	uint32_t targetUser = 0;

	// if existing user.
	if (newInput.targetType == UserType::EXISTING_USER)
	{
		// print exisitng users and ratings
		//pRecommender->PrintUsers(pRecommender->existingUsersRating);

		while (true)
		{
			printf("\nEnter Target User: { 1 - %zu }\n\n>> ", pRecommender->existingUsersRating.size());

			RETURN_IGNORE(scanf_s("%u", &targetUser));

			if (targetUser < 1 || targetUser > pRecommender->existingUsersRating.size())
			{
				printf("\n Sorry Wrong Target! Try again!");
			}
			else if (targetUser == 0)
			{
				return false;
			}
			else
			{
				newInput.targetIndex = (targetUser - 1);
				newInput.targetUser = pRecommender->existingUsersRating[newInput.targetIndex];
				break;
			}
		}
	}
	else
	{
		// print new users and ratings
		//pRecommender->PrintUsers(pRecommender->newUsersRating);

		while (true)
		{
			printf("\nEnter Target User: { 1 - %zu }\n\n>> ", pRecommender->newUsersRating.size());

			RETURN_IGNORE(scanf_s("%u", &targetUser));

			if (targetUser < 1 || targetUser > pRecommender->newUsersRating.size())
			{
				printf("\n Sorry Wrong Target! Try again!");
			}
			else if (targetUser == 0)
			{
				return false;
			}
			else
			{
				newInput.targetIndex = (targetUser - 1);
				newInput.targetUser = pRecommender->newUsersRating[newInput.targetIndex];
				break;
			}
		}
	}

	// iterate for each 'k' value.
	while (true)
	{
		printf("\nEnter value of 'K' for K-nearest rating: { 1 - %zu }\n\n>> ", pRecommender->existingUsersRating.size());
		RETURN_IGNORE(scanf_s("%u", &newInput.k));

		if (newInput.k < 1 || newInput.k > pRecommender->existingUsersRating.size())
		{
			printf("\n Sorry Wrong K value! Try again!");
		}
		else if (newInput.k == 0)
		{
			return false;
		}
		else
		{
			// break from input loop.
			break;
		}
	}

	return (true);
}

void RecommenderSystem::PrintUsersMovieRating(const UserInput& newInput, const float& predictedRating)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// predicted rating of new user on the movie.
	printf("\nPredicted rating of User:%d on movie:%s = %f", (newInput.targetIndex + 1), pRecommender->movieList[newInput.movieIndex].c_str(), predictedRating);
}

// predict exisitng rating of new users and calculate mean absolute error for each k-value.
void RecommenderSystem::PredictExistingRating(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// new input for each user.
	UserInput newInput = {};

	// vector of pearson coefficients for all existing users with their index. { pair : neighbour index, pearson coefficient }
	NearestNeighbour nearestNeighbour = {};

	// preedicted value.
	float predictedValue = 0.0f;

	// average absolute error value for each user.
	float averageAbsoluteError = 0.0f;

	// average error value for each movie.
	float absoluteError = 0.0f;

	// count of movies rated.
	float countOfMoviesRated = 0;

	// loop based on k-value
	for (uint32_t i = 0; i < pRecommender->existingUsersRating.size(); i++)
	{
		// assign k-value.
		newInput.k = (i + 1);

		// reset average absolute error value.
		averageAbsoluteError = 0.0f;

		// loop based on new users.
		for (uint32_t j = 0; j < pRecommender->newUsersRating.size(); j++)
		{
			// clear nearest neighbour for each user.
			nearestNeighbour.clear();

			// set target user
			newInput.targetUser = pRecommender->newUsersRating[j];

			// set user type.
			newInput.targetType = UserType::NEW_USER;

			// set user index.
			newInput.targetIndex = j;

			// find nearest neighbour for new user.
			pRecommender->GetNearestNeighbours(newInput, nearestNeighbour);

			// reset absolute error value.
			absoluteError = 0.0f;

			// reset count.
			countOfMoviesRated = 0;

			// loop for all movies.
			for (uint32_t z = 0; z < pRecommender->movieList.size(); z++)
			{
				// if movie 'z' is already rated by the new user 'j'
				if (pRecommender->newUsersRating[j][z] > 0.0f)
				{
					// increment count of movies rated.
					countOfMoviesRated++;

					// assign movie index
					newInput.movieIndex = z;

					// predict value for existing rated movie with  k-value.
					predictedValue = pRecommender->PredictRating(newInput, nearestNeighbour);

					// error rate { | predicted value - actual value | }
					absoluteError += abs(predictedValue - pRecommender->newUsersRating[j][z]);
				}
			}

			// average of absolute errors of movies rated.
			absoluteError /= countOfMoviesRated;

			// sum the error value for current user.
			averageAbsoluteError += absoluteError;
		}

		// average of absolute errors of all users.
		averageAbsoluteError /= pRecommender->newUsersRating.size();

		// push the average error value to the mean absolute error table.
		meanAbsoluteErrorTable.push_back( { newInput.k, averageAbsoluteError } );
	}

	// print mean absolute error table.
	pRecommender->PrintErrorTable(meanAbsoluteErrorTable);
}

// calculate best k-value from mean absolute error table.
uint32_t RecommenderSystem::ComputeBest_K_ValueFromErrorTable(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable)
{
	// sort the table based on best K-value { ie; decreaing order of error rate (reusing comparator code) }
	std::sort(meanAbsoluteErrorTable.begin(), meanAbsoluteErrorTable.end(), MeanComparator());

	// return k-value of last element
	return (meanAbsoluteErrorTable[0].first);
}

// predict rating of new users for movies not rated with the best k-value.
void RecommenderSystem::PredictNewRating(const uint32_t k)
{
	printf("\n\nPredicted Ratings for movies not rated by new users!");

	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// new input for each user.
	UserInput newInput = {};

	// assign k-value
	newInput.k = k;

	// predicted rating.
	float predictedRating = 0.0f;

	// vector of pearson coefficients for all existing users with their index. { pair : neighbour index, pearson coefficient }
	NearestNeighbour nearestNeighbour = {};

	// for all new users.
	for (uint32_t i = 0; i < pRecommender->newUsersRating.size(); i++)
	{
		// clear nearest neighbour for each user.
		nearestNeighbour.clear();

		// set target user
		newInput.targetUser = pRecommender->newUsersRating[i];

		// set target type.
		newInput.targetType = UserType::NEW_USER;

		// set user index.
		newInput.targetIndex = i;

		// find nearest neighbour for new user.
		pRecommender->GetNearestNeighbours(newInput, nearestNeighbour);

		printf("\n");

		// for all movies.
		for (uint32_t j = 0; j < pRecommender->movieList.size(); j++)
		{
			// if movie is not rated.
			if (pRecommender->newUsersRating[i][j] == 0.0f)
			{
				// assign movie index.
				newInput.movieIndex = j;

				// predict rating.
				predictedRating = pRecommender->PredictRating(newInput, nearestNeighbour);

				// print details.
				pRecommender->PrintUsersMovieRating(newInput, predictedRating);
			}
		}
	}
}


void RecommenderSystem::PrintUsers(matrix& pUserList)
{
	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	for (uint32_t i = 0; i < pRecommender->movieList.size(); i++)
	{
		printf("\t\t%s", pRecommender->movieList[i].c_str());
	}

	// print existing users and their ratings.
	for (uint32_t i = 0; i < pUserList.size(); i++)
	{
		printf("\nUser %d:", i);
		for (uint32_t j = 0; j < pUserList[0].size(); j++)
		{
			printf("\t%f", pUserList[i][j]);
		}
		
	}
}

void RecommenderSystem::PrintMovieList()
{
	printf("\nMovies Availiable:");

	// get class instacne.
	RecommenderSystem* pRecommender = RecommenderSystem::getPrivateInstance();

	// for each movie in list:
	for (uint32_t i = 0; i < pRecommender->movieList.size(); i++)
	{
		// print movie names.
		printf("\n %d) %s", (i + 1), pRecommender->movieList[i].c_str());
	}
}

// print mean absolute error table.
void RecommenderSystem::PrintErrorTable(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable)
{
	printf("\n\n Mean Absolute Table for all new users using existing ratings!\n");

	for (uint32_t i = 0; i < meanAbsoluteErrorTable.size(); i++)
	{
		printf("\n K = %d || Absolute Error Value: %f", meanAbsoluteErrorTable[i].first, meanAbsoluteErrorTable[i].second);
	}
}

// gets the pearson coefficient according to target user and existing user vectors.
const float RecommenderSystem::GetPearsonCoefficient(const std::vector<float>& targetUser, const std::vector<float>& existingUser) const
{
	// number of co-rated items. { n }
	float numberOfCorated = 0.0f;

	// coeficient variables.
	float sumOfTarget = 0.0f, sumOfExisting = 0.0f;
	float sumOfTargetExisting = 0.0f, sumOfTargetSquare = 0.0f, sumOfExistingSquare = 0.0f;

	// loop size of elements.
	for (uint32_t i = 0; i < targetUser.size(); i++)
	{
		// only find the coeficient values for co-rated items.
		if (targetUser[i] > 0.0f && existingUser[i] > 0.0f)
		{
			// increment number of co-rated values.
			numberOfCorated++;

			// sum of taget elements.
			sumOfTarget += targetUser[i];

			// sum of existing elements.
			sumOfExisting += existingUser[i];

			// sum of Target * Exisitng elements.
			sumOfTargetExisting += (targetUser[i] * existingUser[i]);

			// sum of sqaure of target elements.
			sumOfTargetSquare += (targetUser[i] * targetUser[i]);

			// sum of sqaure of existing elements.
			sumOfExistingSquare += (existingUser[i] * existingUser[i]);
		}
	}

	float pearsonCoefficient = 0.0f;

	// if co-rated values is 2 and above.
	if(numberOfCorated > 1)
	{ 
		// calculate pearson coefficient. { p = (n * sum(xy)) - (sum(x)*sum(y)) } 
		pearsonCoefficient = ((numberOfCorated * sumOfTargetExisting) - (sumOfTarget * sumOfExisting));

		// check if numerator is not zero.
		if (pearsonCoefficient != 0.0f)
		{
			// calculate denominator value. { ((n*sum(x2)) - (sum(x2))) * ((n * sum(y2)) - (sum(y2))) }
			float denominatorVal = ((numberOfCorated * sumOfTargetSquare) - (sumOfTarget * sumOfTarget)) *
								   ((numberOfCorated * sumOfExistingSquare) - (sumOfExisting * sumOfExisting));

			// check for negative values or zero to square root.
			if (denominatorVal > 0.0f)
			{
				// { p = p / sqrt(denominator value) }
				pearsonCoefficient /= sqrtf(denominatorVal);
			}
		}
	}
	else
	{
		// avoid this neighbour { dis-similar neighbour }
		pearsonCoefficient = -1.0f;
	}
	

	// return coefficient value.
	return (pearsonCoefficient);
}

// functional operator which compares two pair structure.
bool RecommenderSystem::Comparator::operator()(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) const
{
	// return true if second float value of existing pair is less. { 'second' means pearson coefficient in this case }
	return (a.second > b.second);
}

// functional operator which compares two pair structure.
bool RecommenderSystem::RecommendationComparator::operator()(const std::pair<UserInput, float>& a, const std::pair<UserInput, float>& b) const
{
	// return true if second float value of existing pair is less. { 'second' means predicted rating in this case }
	return (a.second > b.second);
}

// functional operator which compares two pair structure.
bool RecommenderSystem::MeanComparator::operator()(const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) const
{
	// return true if second float value of existing pair is less. { 'second' means mean error value in this case }
	return (a.second < b.second);
}
