#pragma once

#ifndef __IMB_APP_HPP__
#define __IMB_APP_HPP__

#include "std.hpp"
#include "gui.hpp"
#include "utils.hpp"

enum class ImageErrorCode
{
    FileNotFound,
};

static const char *imageErrorText(const ImageErrorCode errorCode);

struct CurrentPathInfo
{
    std::filesystem::path executable, directory;
    std::filesystem::path input, output;
};

struct GuiInputData
{
    char filename[255];
    float hsvFrom[4], hsvTo[4];
    bool imagePreviewOpened, maskPreviewOpened;

    GuiInputData();

    cv::Scalar hsvCV(float *hsv) const;
    ColorRange hsvToRange() const;
};

class IBMApplication : public Application
{
private:
    Image *image;
    cv::Mat *mask;

protected:
    GuiInputData inputData;
    std::vector<Image *> imagePool;

public:
    CurrentPathInfo path;

public:
    IBMApplication(const std::string &currentExecutablePath = ".");

    virtual bool init();
    virtual void draw();

    void drawImageTools(bool reset);

    void drawImagePreview();
    void drawMaskPreview();

protected:
    std::string buildImageTitle(const std::string &addition = "") const;
    void renderImageFromCV(const cv::Mat &img) const;

    void createDirectory(const std::filesystem::path &path) const;
    std::string buildOutputImageFilename() const;

    void loadImage(const std::string &filename);
    Image *getImageFromPool(const std::filesystem::path &filename) const;
};

#endif
