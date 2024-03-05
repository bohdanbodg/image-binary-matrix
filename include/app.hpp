#pragma once

#ifndef __IMB_APP_HPP__
#define __IMB_APP_HPP__

#include "std.hpp"
#include "gui.hpp"
#include "utils.hpp"

struct CurrentPathInfo
{
    std::filesystem::path executable, directory;
    std::filesystem::path input, output;
    std::vector<std::filesystem::path> images;
};

struct GuiInputData
{
    int selectedImageFile;
    float hsvFrom[4], hsvTo[4];
    bool imagePreviewOpened, maskPreviewOpened, binaryMatrixPreview;

    GuiInputData();

    cv::Scalar hsvCV(float *hsv) const;
    ColorRange hsvToRange() const;
    bool isImageSelected() const;
};

class IBMApplication: public Application
{
  protected:
    Image *image;
    GuiInputData data;
    std::vector<Image *> imagePool;
    BinaryMatrix matrix;
    bool isImageLoaded, isMaskProcessed;
    // To refresh the image file list
    bool toRefresh;
    // To reset UI internal states
    bool toReset;
    // To regenerate matrix and mask
    bool toRegenerate;

  public:
    CurrentPathInfo path;

  public:
    IBMApplication(const std::string &currentExecutablePath = ".");

    virtual bool init();
    virtual void draw();

    void drawMenuBar();
    void drawMainWindow();

    void drawImageTools();

    void drawImagePreview();
    void drawMaskPreview();
    void drawMatrixPreview();

    std::string getSelectedImage() const;

  protected:
    std::string buildImageTitle(const std::string &addition = "") const;
    void renderImage(Texture2D &texture, const cv::Mat &mat);

    void loadImageFileList();
    void createDirectory(const std::filesystem::path &path) const;
    std::string buildOutputImageFilename() const;

    void loadImage(const std::string &filename);
    Image *getImageFromPool(const std::string &filename) const;

    void generateBinaryMatrix();
};

#endif
