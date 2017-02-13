#include "gauss.h"

std::vector<std::vector<double>> generate_gauss_kernel(double spread, size_t range_x, size_t range_y)
{
	if (!range_y)
	range_y = range_x;

	std::vector<std::vector<std::pair<int, int> > > index;
	auto max_index_x = range_x / 2;
	auto max_index_y = range_y / 2;

	index.resize(range_y);
	for (auto& vector : index)
		vector.resize(range_x);

	for (size_t y = 0; y < index.size(); ++y)
		for (size_t x = 0; x < index[y].size(); ++x)
			index[y][x] = std::make_pair(x - max_index_x, y - max_index_y);

	std::vector<std::vector<double>> result;
	result.resize(range_y);
	for (auto& vector : result)
		vector.resize(range_x);

	for (size_t y = 0; y < result.size(); ++y)
		for (size_t x = 0; x < result[y].size(); ++x)
			result[y][x] = exp(-1 * (pow(index[x][y].first, 2) + pow(index[x][y].second, 2) ) / 2 / pow(spread, 2) ) / M_PI / 2 / pow(spread, 2);

	double sum = 0.f;
	for (const auto& vec_2d : result)
		for (const auto& value : vec_2d)
			sum += value;

	for (auto& vector : result)
		for (auto& value : vector)
			value /= sum;

	return result;
}


