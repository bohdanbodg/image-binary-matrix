#include "utils.hpp"

ColorRange::ColorRange(const cv::Scalar &from, const cv::Scalar &to)
    : from(from), to(to) {
}

Texture2D::Texture2D(): glTexture(nullptr) {
}

#ifdef _WIN32
#define _GL_BGR_PLATFORM GL_BGR_EXT
#define _GL_BGRA_PLATFORM GL_BGRA_EXT
#else
#define _GL_BGR_PLATFORM GL_BGR
#define _GL_BGRA_PLATFORM GL_BGRA
#endif

void Texture2D::reset() {
    if (this->glTexture != nullptr) {
        delete this->glTexture;
        this->glTexture = nullptr;
    }
}

void Texture2D::render(const cv::Mat &mat) {
    bool hasGLTexture = this->glTexture != nullptr;
    if (hasGLTexture) {
        this->bind();

        return;
    }

    static const GLenum gl_types[]
        = {GL_UNSIGNED_BYTE,
           GL_BYTE,
           GL_UNSIGNED_SHORT,
           GL_SHORT,
           GL_INT,
           GL_FLOAT,
           GL_DOUBLE};
    static const GLint gl_internal_formats[]
        = {0, GL_DEPTH_COMPONENT, 0, GL_RGB, GL_RGBA};
    static const GLenum gl_formats[]
        = {0, GL_DEPTH_COMPONENT, 0, _GL_BGR_PLATFORM, _GL_BGRA_PLATFORM};

    const auto imgType = mat.type();
    const auto depth = CV_MAT_DEPTH(imgType);
    const auto cn = CV_MAT_CN(imgType);

    this->glTexture = new GLuint;

    glGenTextures(1, this->glTexture);

    this->bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        gl_internal_formats[cn],
        mat.cols,
        mat.rows,
        0,
        gl_formats[cn],
        gl_types[depth],
        mat.data
    );
}

void Texture2D::bind() {
    glBindTexture(GL_TEXTURE_2D, *this->glTexture);
}

Image::Image(const std::filesystem::path &path, bool load)
    : filename(path.filename()),
      ext(path.extension()),
      path(path),
      loaded(false),
      maskProcessed(false) {
    if (load) {
        this->load();
    }
}

void Image::load() {
    if (this->loaded) {
        return;
    }

    cv::cvtColor(
        cv::imread(this->path.string(), cv::ImreadModes::IMREAD_COLOR),
        this->cv,
        cv::COLOR_BGR2BGRA
    );

    this->loaded = true;
}

void Image::validate() const {
    if (!this->loaded) {
        throw Exception("Image is not loaded!");
    }
}

void Image::processMaskByColorRange(const ColorRange &colorRange) {
    this->validate();

    cv::Mat mask, hsv;
    cv::cvtColor(this->cv, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, colorRange.from, colorRange.to, mask);

    // Check if there are any white pixels on mask
    bool hasColor = cv::sum(mask)[0] > 0;
    if (hasColor) {
        cv::cvtColor(mask, mask, cv::COLOR_GRAY2BGRA);
    }

    this->mask = mask;
    this->maskProcessed = hasColor;
    this->maskTexture.reset();
}

bool Image::saveMask(const std::filesystem::path &path) {
    if (this->maskProcessed == false || this->mask.cols == 0
        || this->mask.rows == 0) {
        throw Exception("Mask has not been processed yet!");
    }

    return cv::imwrite(path.string(), this->mask);
}

int Image::width() const {
    return this->cv.cols;
}

int Image::height() const {
    return this->cv.rows;
}

bool Image::isLoaded() const {
    return this->loaded;
}

bool Image::isMaskProcessed() const {
    return this->maskProcessed;
}

Exception::Exception(const std::string &message): message(message) {
}

BinaryMatrix::BinaryMatrix(): BinaryMatrix(0, 0) {
}

BinaryMatrix::BinaryMatrix(size_t rows, size_t cols) {
    this->reset(rows, cols);
}

BinaryMatrix::BinaryMatrix(
    size_t rows,
    size_t cols,
    const std::vector<std::vector<BinaryMatrix::Type>> &data
)
    : rows(rows), cols(cols) {
    if (data.size() == rows) {
        this->data = data;
    }
}

bool BinaryMatrix::isEmpty() const {
    return !this->rows || !this->cols || this->data.empty();
}

bool BinaryMatrix::isValid() const {
    if (this->rows != this->data.size()) {
        return false;
    }

    for (auto &&row : this->data) {
        if (row.size() != this->cols) {
            return false;
        }
    }

    return true;
}

bool BinaryMatrix::hasTrue() const {
    if (this->isEmpty()) {
        return false;
    }

    for (auto &&irows : this->data) {
        for (auto &&jcol : irows) {
            if (jcol == BinaryMatrix::True) {
                return true;
            }
        }
    }

    return false;
}

void BinaryMatrix::clear() {
    this->data.clear();
    this->cols = 0;
    this->rows = 0;
}

BinaryMatrix::Type BinaryMatrix::at(size_t row, size_t col) const {
    return this->data[row][col];
}

void BinaryMatrix::set(size_t row, size_t col, BinaryMatrix::Type value) {
    this->data[row][col] = value;
}

void BinaryMatrix::reset(size_t rows, size_t cols) {
    this->clear();

    this->rows = rows;
    this->cols = cols;

    if (rows > 0 && cols > 0) {
        this->data.assign(
            rows,
            std::vector<BinaryMatrix::Type>(cols, BinaryMatrix::False)
        );
    }
}

bool BinaryMatrix::isTrue(size_t row, size_t col) const {
    return this->at(row, col) == BinaryMatrix::True;
}

bool BinaryMatrix::isFalse(size_t row, size_t col) const {
    return this->at(row, col) == BinaryMatrix::False;
}

std::vector<unsigned int> BinaryMatrix::sumRows() const {
    if (this->isEmpty()) {
        return {};
    }

    std::vector<unsigned int> result(this->rows);

    for (size_t i = 0; i < this->rows; i++) {
        unsigned int sum = 0;

        for (size_t j = 0; j < this->cols; j++) {
            sum += (unsigned int)(this->at(i, j) == BinaryMatrix::True ? 1 : 0);
        }

        result[i] = sum;
    }

    return result;
}

std::vector<unsigned int> BinaryMatrix::sumCols() const {
    if (this->isEmpty()) {
        return {};
    }

    std::vector<unsigned int> result(this->cols);

    for (size_t j = 0; j < this->cols; j++) {
        unsigned int sum = 0;

        for (size_t i = 0; i < this->rows; i++) {
            sum += (unsigned int)(this->at(i, j) == BinaryMatrix::True ? 1 : 0);
        }

        result[j] = sum;
    }

    return result;
}

BinaryMatrix BinaryMatrix::transpose() const {
    BinaryMatrix matrix(this->cols, this->rows);

    for (size_t i = 0; i < this->rows; i++) {
        for (size_t j = 0; j < this->cols; j++) {
            matrix.set(j, i, this->at(i, j));
        }
    }

    return matrix;
}
