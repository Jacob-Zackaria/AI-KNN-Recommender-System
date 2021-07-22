#ifndef RECOMMENDER_SYSTEM_H
#define RECOMMENDER_SYSTEM_H

#include "FilesAccess.h"
#include "UserType.h"

#define RETURN_IGNORE(x) (void(x))

// structure to store input from user.
struct UserInput
{
	std::vector<float> targetUser;
	UserType targetType;
	uint32_t targetIndex;
	uint32_t movieIndex;
	uint32_t k;
};

// renaming pair of vectors to nearest neighbour { same as 'typedef' in C }
using NearestNeighbour = std::vector<std::pair<uint32_t, float>>;

class RecommenderSystem
{
public:

	//------------------------> Big four operators <-------------------------//

	// copy constructor.
	RecommenderSystem(const RecommenderSystem&) = delete;

	// copy assignment operator. 
	const RecommenderSystem& operator = (const RecommenderSystem&) = delete;

	// destructor.
	~RecommenderSystem();

	//----------------------------------------------------------------------//

	//------------------------> Public functions <--------------------------//

	// gets training data from input file (Users U1 - U20)
	static void GetTrainingData(const char* inputFile);

	// gets test data from input file (New Users N1-N5)
	static void GetTestData(const char* inputFile);

	// generate recommendation based on user input.
	static void  GenerateRecommendations(const UserInput& newInput);

	// read users input for movie recommendation.
	static bool ReadUserInput(UserInput& newInput);

	// prints all movie list.
	static void PrintMovieList();

	// predict exisitng rating of new users and calculate mean absolute error for each k-value.
	static void PredictExistingRating(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable);

	// calculate best k-value from mean absolute error table.
	static uint32_t ComputeBest_K_ValueFromErrorTable(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable);

	// predict rating of new users for movies not rated with the best k-value.
	static void PredictNewRating(const uint32_t k);

	//----------------------------------------------------------------------//

private:

	//------------------------> Private functions <-------------------------//

	// get singleton instance.
	static RecommenderSystem* getPrivateInstance();

	// private default constructor.
	RecommenderSystem();

	// print all users with ratings.
	void PrintUsers(matrix& pUserList);

	// print movie rating.
	static void PrintUsersMovieRating(const UserInput& newInput, const float& predictedRating);

	// print mean absolute error table.
	void PrintErrorTable(std::vector<std::pair<uint32_t, float>>& meanAbsoluteErrorTable);

	// get the nearest k-neighbours of target user.
	static void GetNearestNeighbours(const UserInput& newInput, NearestNeighbour& nearestNeighbour);

	// gets the pearson coefficient according to target user and existing user vectors.
	const float GetPearsonCoefficient(const std::vector<float>& targetUser, const std::vector<float>& existingUser) const;

	// predict movie ratings for new users according to user input and nearest neighbours { returns predicted value }.
	static const float PredictRating(const UserInput& newInput, const NearestNeighbour& nearestNeighbour);

	// comparator for vector to rank based on decreasing order.
	struct Comparator
	{
		// functional operator which compares two pair structure.
		bool operator () (const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) const;
	};

	// comparator for vector to rank based on increasing order.
	struct MeanComparator
	{
		// functional operator which compares two pair structure.
		bool operator () (const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) const;
	};

	// comparator for vector to rank based on decreasing order of predicted rating.
	struct RecommendationComparator
	{
		// functional operator which compares two pair structure.
		bool operator () (const std::pair<UserInput, float>& a, const std::pair<UserInput, float>& b) const;
	};
	

	//----------------------------------------------------------------------//

	//------------------------------->  Data <------------------------------//

	// create existing users rating.
	matrix existingUsersRating;

	// create new users rating.
	matrix newUsersRating;

	// movie names list.
	std::vector<std::string> movieList;

	//----------------------------------------------------------------------//

};


#endif RECOMMENDER_SYSTEM_H