#pragma once

#define _USE_MATH_DEFINES
#include <vector>
#include <cmath>

std::vector<std::vector<double>> generate_gauss_kernel(double spread, size_t range_x, size_t range_y = 0);

