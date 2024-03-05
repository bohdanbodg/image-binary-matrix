#pragma once

#ifndef __IBM_UTILS_HPP___
#define __IBM_UTILS_HPP___

#include "std.hpp"

#include <opencv2/opencv.hpp>
#include <GL/glew.h>

struct ColorRange
{
    cv::Scalar from, to;

    ColorRange(const cv::Scalar &from, const cv::Scalar &to);
};

struct Texture2D
{
    GLuint *glTexture;

    Texture2D();

    void reset();
    void render(const cv::Mat &mat);
    void bind();
};

struct Image
{
    std::filesystem::path filename, ext, path;
    cv::Mat cv, mask;
    Texture2D texture, maskTexture;

    Image(const std::filesystem::path &path, bool load = false);

    void load();
    void validate() const;

    void processMaskByColorRange(const ColorRange &colorRange);
    bool saveMask(const std::filesystem::path &path);

    int width() const;
    int height() const;

    bool isLoaded() const;
    bool isMaskProcessed() const;

  private:
    bool loaded, maskProcessed;
};

struct Exception
{
    std::string message;

    Exception(const std::string &message);
};

struct BinaryMatrix
{
    using Type = bool;
    static const Type True = (Type)(1);
    static const Type False = (Type)(0);

    size_t rows, cols;
    std::vector<std::vector<Type>> data;

    BinaryMatrix();
    BinaryMatrix(size_t rows, size_t cols);
    BinaryMatrix(
        size_t rows,
        size_t cols,
        const std::vector<std::vector<Type>> &data
    );

    bool isEmpty() const;
    bool isValid() const;
    bool hasTrue() const;
    void clear();
    Type at(size_t row, size_t col) const;
    void set(size_t row, size_t col, Type value);
    void reset(size_t rows = 0, size_t cols = 0);
    bool isTrue(size_t row, size_t col) const;
    bool isFalse(size_t row, size_t col) const;

    // Return vector of rows,
    // each element is a sum of all the row elements
    std::vector<unsigned int> sumRows() const;

    // Return vector of columns,
    // each element is a sum of all the column
    std::vector<unsigned int> sumCols() const;

    // Flip a matrix over its diagonal
    BinaryMatrix transpose() const;

  private:
    using self = BinaryMatrix;
};

#endif
