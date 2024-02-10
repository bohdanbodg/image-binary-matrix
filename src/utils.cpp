#include "utils.hpp"

ColorRange::ColorRange(const cv::Scalar &from, const cv::Scalar &to) : from(from), to(to) {}

Image::Image(const std::filesystem::path &path, bool load):
    filename(path.filename()),
    path(path),
    loaded(false) {
    if (load) {
        this->load();
    }
}

void Image::load() {
    if (this->loaded) {
        return;
    }

    this->cv = cv::imread(this->path.string());
    this->loaded = true;
}

void Image::validate() const {
    if (!this->loaded) {
        throw ("Image is not loaded!");
    }
}

cv::Mat Image::getHSV() const {
    this->validate();

    cv::Mat hsv;
    cv::cvtColor(this->cv, hsv, cv::COLOR_BGR2HSV);

    return hsv;
}

cv::Mat Image::getMaskByColorRange(const ColorRange &colorRange, bool *hasColor) const {
    this->validate();

    cv::Mat mask;
    cv::inRange(this->getHSV(), colorRange.from, colorRange.to, mask);

    // Check if there are any white pixels on mask
    if (hasColor != nullptr) {
        *hasColor = cv::sum(mask)[0] > 0;
    }

    return mask;
}

cv::Mat Image::getImageByMask(const cv::Mat &mask) const {
    this->validate();

    cv::Mat res;
    cv::bitwise_and(this->cv, this->cv, res, mask);

    return res;
}

cv::Mat Image::getMaskBGR(const cv::Mat &mask) const {
    cv::Mat bgr;
    cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR);

    return bgr;
}

bool Image::isLoaded() const {
    return loaded;
}
