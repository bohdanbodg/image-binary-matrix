#pragma once

#ifndef __IBM_UTILS_HPP___
#define __IBM_UTILS_HPP___

#include "std.hpp"

#include <opencv2/opencv.hpp>

struct ColorRange
{
    cv::Scalar from, to;

    ColorRange(const cv::Scalar &from, const cv::Scalar &to);
};

struct Image
{
    std::filesystem::path filename, path;
    cv::Mat cv;

    Image(const std::filesystem::path &path, bool load = false);

    // Load image
    void load();

    // Validate image
    void validate() const;

    // Convert image colors to HSV
    cv::Mat getHSV() const;

    // Threshold the HSV image - any specified color in range will show up as white
    cv::Mat getMaskByColorRange(const ColorRange &colorRange, bool *hasColor = nullptr) const;

    // Apply mask to image
    cv::Mat getImageByMask(const cv::Mat &mask) const;

    // Convert mask colors to BGR
    cv::Mat getMaskBGR(const cv::Mat &mask) const;

    // Check if image is currently loaded
    bool isLoaded() const;

private:
    bool loaded;
};

#endif
