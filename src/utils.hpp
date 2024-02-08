#pragma once

#ifndef __IBM_UTILS_HPP___
#define __IBM_UTILS_HPP___

#include <fstream>
#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>

struct ColorRange
{
    cv::Scalar from, to;

    ColorRange(const cv::Scalar &from, const cv::Scalar &to) : from(from), to(to) {}
};

struct Image
{
    std::filesystem::path filename, path;
    cv::Mat cv;
    bool loaded;

    Image(std::string filename, bool load = false):
        filename(std::filesystem::path(filename)),
        path(std::filesystem::absolute(this->filename)),
        loaded(false) {
        if (load) {
            this->load();
        }
    }

    // Load image
    void load() {
        if (this->loaded) {
            return;
        }

        this->cv = cv::imread(this->path);
        this->loaded = true;
    }

    // Validate image
    void validate() const {
        if (!this->loaded) {
            throw ("Image is not loaded!");
        }
    }

    // Convert to HSV
    cv::Mat getHSV() const {
        this->validate();

        cv::Mat hsv;
        cv::cvtColor(this->cv, hsv, cv::COLOR_BGR2HSV);

        return hsv;
    }

    // Threshold the HSV image - any specified color in range will show up as white
    cv::Mat getMaskByColorRange(const ColorRange &colorRange, bool *hasColor = nullptr) const {
        this->validate();

        cv::Mat mask;
        cv::inRange(this->getHSV(), colorRange.from, colorRange.to, mask);

        // Check if there are any white pixels on mask
        if (hasColor != nullptr) {
            *hasColor = cv::sum(mask)[0] > 0;
        }

        return mask;
    }

    // Apply mask to image
    cv::Mat getImageByMask(const cv::Mat &mask) const {
        this->validate();

        cv::Mat res;
        cv::bitwise_and(this->cv, this->cv, res, mask);

        return res;
    }
};

#endif
