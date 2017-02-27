#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/highgui/highgui.hpp>

#include "gauss.h"

using namespace cv;
using namespace std;



struct rgba
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct pixel_index
{
	size_t x, y;
};

inline bool operator==(const rgba& lhs, const rgba& rhs)
{
	return (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}
inline bool operator!=(const rgba& lhs, const rgba& rhs)
{
	return !(lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
}

std::vector<rgba> load_colour_whitelist_from_file(char* filepath)
{
	std::vector<rgba> result;
	ifstream file(filepath);
	if (!file.is_open())
	{
		std::cout << "couldn't open whitelist file\n";
		exit(-1);
	}
	char buff[256];
	while(file.getline(buff, 256))
	{
		unsigned char colours[4];
		char current[4];
		std::stringstream ss(buff);
		for (size_t i = 0; i < 4; ++i)
		{
			if (ss.eof())
				break;
			ss >> current;
			colours[i] = strtol(current, nullptr, 10);
		}
			
		result.push_back(rgba{ colours[0], colours[1], colours[2], colours[3] });
	}
	return result;
}

vector<pixel_index> hide_undesired_pixels(Mat& img, const vector<rgba>& color_whitelist)
{
	vector<pixel_index> result;

	for (size_t y = 0; y < img.rows; ++y)
		for (size_t x = 0; x < img.cols; ++x)
		{
			cv::Vec4b & pixel = img.at<cv::Vec4b>(y, x);
			rgba current_color {pixel[2], pixel[1] ,pixel[0],pixel[3]};
			auto found = find_if(color_whitelist.begin(), color_whitelist.end(), [current_color](const rgba& a) {
				return a == current_color;
			});

			if (found == color_whitelist.end())
			{
				// set alpha to zero:
				pixel[3] = 0;
				pixel[2] = 0;
				pixel[1] = 0;
				pixel[0] = 0;
			}
			else
				result.push_back(pixel_index{ x, y });
		}

	return result;
}

void resize_image(Mat& img, size_t cols, size_t rows)
{
	Mat copy_mat;
	copy_mat.create(cols, rows, img.type());
	copy_mat.setTo(cv::Scalar(0, 0, 0, 0));

	auto offset_x = abs((cols - img.cols) / 2);
	auto offset_y = abs((rows - img.rows) / 2);

	for (size_t y = 0; y < img.rows; ++y)
		for (size_t x = 0; x < img.cols; ++x)
		{
			cv::Vec4b & img_pixel = img.at<cv::Vec4b>(y, x);
			cv::Vec4b & resized_img_pixel = copy_mat.at<cv::Vec4b>(y + offset_y, x + offset_y);
			resized_img_pixel = img_pixel;
		}

	img = copy_mat;
}

void make_light(Mat& img, double spread, size_t range_x, size_t range_y, double intensity , vector<pixel_index> pixels)
{
	const auto kernel = generate_gauss_kernel(spread, range_x, range_y);

	vector<Vec4b> pixel_colors;
	for(const auto& pixel : pixels)
		pixel_colors.push_back(img.at<cv::Vec4b>(pixel.y, pixel.x));

	size_t i = 0;

	for (auto& pixel : pixels) {
		Vec4b center_pixel = pixel_colors[i];
		++i;
		auto center_pixel_rgba = rgba{ center_pixel[2], center_pixel[1], center_pixel[0], center_pixel[3] };
		for (size_t y = 0; y < kernel.size(); ++y)
			for (size_t x = 0; x < kernel[y].size(); ++x)
			{
				int current_index_y = pixel.y + y - range_y / 2;
				if (current_index_y < 0 || current_index_y >= img.rows)
					continue;

				int current_index_x = pixel.x + x - range_x / 2;
				if (current_index_x < 0 || current_index_x >= img.cols)
					continue;

				Vec4b& current_pixel = img.at<cv::Vec4b>(current_index_y, current_index_x);
				auto current_pixel_rgba = rgba{ current_pixel[2], current_pixel[1], current_pixel[0], current_pixel[3] };
				auto alpha = static_cast<size_t>(255 * kernel[y][x] * intensity);
				alpha = alpha > 255 ? 255 : alpha;
				if (current_pixel_rgba == rgba{ 0, 0, 0, 0 } && alpha)
				{
					current_pixel[2] = center_pixel[2];
					current_pixel[1] = center_pixel[1];
					current_pixel[0] = center_pixel[0];
				}
				else if (current_pixel_rgba != center_pixel_rgba && alpha)
				{
					current_pixel[2] = (alpha * center_pixel[2] + current_pixel[3] * current_pixel[2]) / (alpha + current_pixel[3]);
					current_pixel[1] = (alpha * center_pixel[1] + current_pixel[3] * current_pixel[1]) / (alpha + current_pixel[3]);
					current_pixel[0] = (alpha * center_pixel[0] + current_pixel[3] * current_pixel[0]) / (alpha + current_pixel[3]);
				}
				if (alpha > current_pixel[3])
					current_pixel[3] = alpha;
			}
	}

}

int main(int argc, char** argv)
{

	if (argc != 8)
	{
		cout << " Usage (args): image_input, image_output, colour_whitelist_file (rgba), standard deviaton, light width, light height, amplifier" << endl;
		return -1;
	}

	Mat image;
	image = imread(argv[1], IMREAD_UNCHANGED);

	if (!image.data)
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	auto range_x = strtoll(argv[5], nullptr, 10) * 2;
	auto range_y = strtoll(argv[6], nullptr, 10) * 2;
	auto deviation = atof(argv[4]);
	auto amplifier = atof(argv[7]);

	auto color_whitelist = load_colour_whitelist_from_file(argv[3]); /*{ rgba{0,255,255,255}, rgba{ 255, 0, 0, 255 }, rgba{ 255, 0, 255, 255 }, rgba {0, 255, 174}, rgba{ 255, 0, 228 }, {0, 198, 255} };*/
	if (range_x > image.cols || range_y > image.rows)
		resize_image(image, range_x, range_y);
	auto pixels = hide_undesired_pixels(image, color_whitelist);
	make_light(image, deviation, range_x, range_y, amplifier, pixels);


	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	cv::imwrite(argv[2], image, compression_params);
	return 0;
}