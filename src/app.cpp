#include "app.hpp"

using namespace std;
namespace fs = std::filesystem;

static const auto greenTextColor = ImVec4(0.455f, 0.922f, 0.543f, 1.000f);
static const auto redTextColor = ImVec4(0.922f, 0.455f, 0.455f, 1.000f);

const char *imageErrorText(const ImageErrorCode errorCode) {
    static const char *_imageErrors[] = {
        "Image file not found!"
    };

    const auto arrSize = sizeof(_imageErrors) / sizeof(*_imageErrors);
    const auto arrIndex = static_cast<int>(errorCode);

    if (arrIndex >= arrSize) {
        return "Unknown error!";
    }

    return _imageErrors[arrIndex];
}

GuiInputData::GuiInputData() :
    imagePreviewOpened(false),
    maskPreviewOpened(false),
    hsvFrom{0.145f, 0.165f, 0.000f, 1.0f},
    hsvTo{0.329f, 1.000f, 1.000f, 1.0f} {
}

cv::Scalar GuiInputData::hsvCV(float *hsv) const {
    return cv::Scalar((double)(hsv[0] * 255), (double)(hsv[1] * 255), (double)(hsv[2] * 255));
}

ColorRange GuiInputData::hsvToRange() const {
    const auto from = this->hsvCV((float *)this->hsvFrom);
    const auto to = this->hsvCV((float *)this->hsvTo);

    return ColorRange(from, to);
}

IBMApplication::IBMApplication(const std::string &currentExecutablePath):
    Application("Image Binary Matrix"), image(nullptr), mask(nullptr) {
    this->path.executable = fs::absolute(fs::path(currentExecutablePath));
    this->path.directory = this->path.executable.parent_path();

    this->path.input = this->path.directory / std::string("input/");
    this->path.output = this->path.directory / std::string("output/");
}

bool IBMApplication::init() {
    if (!Application::init()) {
        return false;
    }

    // Seek first image in the input folder
    for (const auto &entry : fs::recursive_directory_iterator(this->path.input)) {
        const auto &entryPath = entry.path();
        const auto &ext = entryPath.extension().string();

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            strcpy(this->inputData.filename, entryPath.filename().c_str());

            break;
        }
    }

    return true;
}

void IBMApplication::draw() {
    ImGui::Begin(this->windowName);

    ImGui::BeginGroup();
    ImGui::InputText("Image Filename", this->inputData.filename, sizeof(this->inputData.filename));
    ImGui::SameLine();

    static bool reset = false;
    if (reset) {
        reset = false;
    }

    static const char *messageToDisplay = nullptr;
    static ImVec4 colorToDisplay;
    if (ImGui::Button("Load")) {
        reset = true;

        try {
            this->loadImage(this->inputData.filename);

            auto ptr = this->getImageFromPool((char *)this->inputData.filename);
            colorToDisplay = greenTextColor;
            messageToDisplay = ptr == nullptr ? "Image loaded from file!" : "Image loaded from cache!";
        } catch (ImageErrorCode code) {
            colorToDisplay = redTextColor;
            messageToDisplay = imageErrorText(code);
        } catch (...) {
            colorToDisplay = redTextColor;
            messageToDisplay = "Image unknown error!";
        }
    }

    if (messageToDisplay != nullptr) {
        ImGui::TextColored(colorToDisplay, "%s", messageToDisplay);
    }

    ImGui::EndGroup();

    if (image != nullptr && image->isLoaded()) {
        this->drawImageTools(reset);
        
        if (this->inputData.imagePreviewOpened) {
            this->drawImagePreview();
        }

        if (mask != nullptr && this->inputData.maskPreviewOpened) {
            this->drawMaskPreview();
        }
    }

    ImGui::End();
}

void IBMApplication::drawImageTools(bool reset) {
    static cv::Mat pmask;
    static bool hasColor;
    static bool saved, savedMatrix;
    static string savedFilename, savedFilenameMatrix;

    if (reset) {
        pmask = cv::Mat();
        hasColor = true;
        saved = savedMatrix = false;
        savedFilename = savedFilenameMatrix = "";
    }

    ImGui::NewLine();

    bool previewOpened = this->inputData.imagePreviewOpened;
    if (ImGui::Button(previewOpened ? "Close Preview" : "Open Preview")) {
        this->inputData.imagePreviewOpened = !previewOpened;
    }

    ImGui::NewLine();

    ImGui::Text("Color Range");
    ImGui::ColorEdit3("From", this->inputData.hsvFrom, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV);
    ImGui::ColorEdit3("To", this->inputData.hsvTo, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV);
    ImGui::NewLine();


    if (ImGui::Button("Process Mask")) {
        pmask = this->image->getMaskByColorRange(this->inputData.hsvToRange(), &hasColor);

        this->mask = hasColor ? &pmask : nullptr;
    }

    if (!hasColor) {
        ImGui::TextColored(redTextColor, "Image does not contain parts within specified color range!");
    } else if (this->mask != nullptr) {
        ImGui::TextColored(greenTextColor, "Mask has been processed!");
    }

    if (this->mask == nullptr) {
        return;
    } else if (!hasColor) {
        return;
    }

    ImGui::NewLine();

    bool maskOpened = this->inputData.maskPreviewOpened;
    if (ImGui::Button(maskOpened ? "Close Mask Preview" : "Open Mask Preview")) {
        this->inputData.maskPreviewOpened = !maskOpened;
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    if (ImGui::Button("Save Binary Matrix to File")) {
        this->createOutputDirectory();

        savedFilenameMatrix = this->buildOutputImageFilename() + ".txt";
        std::ofstream fileOutput((this->path.output / savedFilenameMatrix).string());

        const auto &mask = *this->mask;
        for (size_t i = 0; i < mask.rows; i++) {
            for (size_t j = 0; j < mask.cols; j++) {
                const auto &pixel = mask.at<cv::Vec3b>(i, j);
                fileOutput << int(bool(pixel[0]) || bool(pixel[1]) || bool(pixel[2]));
            }

            fileOutput << endl;
        }
        fileOutput.close();

        savedMatrix = fileOutput.good();
    }

    if (!savedFilenameMatrix.empty()) {
        if (savedMatrix) {
            ImGui::TextColored(greenTextColor, "Binary matrix has been saved as \"%s\" in the \"output\" folder!", savedFilenameMatrix.c_str());
        } else {
            ImGui::TextColored(redTextColor, "Binary matrix save error (filename: \"%s\")!", savedFilenameMatrix.c_str());
        }
    }

    ImGui::NewLine();

    if (ImGui::Button("Save Mask to File")) {
        this->createOutputDirectory();

        savedFilename = this->buildOutputImageFilename();
        saved = cv::imwrite((this->path.output / savedFilename).string(), *this->mask);
    }

    if (!savedFilename.empty()) {
        if (saved) {
            ImGui::TextColored(greenTextColor, "Mask has been saved as \"%s\" in the \"output\" folder!", savedFilename.c_str());
        } else {
            ImGui::TextColored(redTextColor, "Mask save error (filename: \"%s\")!", savedFilename.c_str());
        }
    }
}

void IBMApplication::drawImagePreview() {
    ImGui::Begin(this->buildImageTitle("original").c_str(), &this->inputData.imagePreviewOpened);

    this->renderImageFromCV(this->image->cv);

    ImGui::End();
}

void IBMApplication::drawMaskPreview() {
    ImGui::Begin(this->buildImageTitle("mask").c_str(), &this->inputData.maskPreviewOpened);

    // TODO: Fix mask preview rendering
    // This is optional feature, so not in priority yet
    ImGui::TextDisabled("TODO: Fix mask preview rendering");
    // this->renderImageFromCV(*this->mask);

    auto img = *this->mask;

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
    ImGui::Image((void *)(intptr_t)(texture), ImVec2(img.cols, img.rows));

    ImGui::End();
}

std::string IBMApplication::buildImageTitle(const std::string &addition) const {
    std::string result = this->image->filename.string();

    const auto &img = this->image->cv;

    result += " [" + to_string(img.cols) + "x" + to_string(img.rows) + "]";
    if (!addition.empty()) {
        result += " (" + addition + ")";
    }

    return result;
}

void IBMApplication::renderImageFromCV(const cv::Mat &img) const {
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
    ImGui::Image((void *)(intptr_t)(texture), ImVec2(img.cols, img.rows));
}

void IBMApplication::createOutputDirectory() const {
    const auto &output = this->path.output;
    if (!fs::is_directory(output) || !fs::exists(output)) {
        fs::create_directory(output);
    }
}

std::string IBMApplication::buildOutputImageFilename() const {
    const auto &img = this->image->cv;

    return this->image->filename.stem().string()
        + (string(".") + to_string(img.cols) + "x" + to_string(img.rows))
        + this->image->filename.extension().string();
}

void IBMApplication::loadImage(const string &filename) {
    const auto path = this->path.input / filename;
    if (!fs::exists(path)) {
        throw (ImageErrorCode::FileNotFound);
    }

    auto imageInPool = this->getImageFromPool(path);
    if (this->image != nullptr && imageInPool == nullptr) {
        this->imagePool.push_back(this->image);
    }

    this->image = imageInPool == nullptr ? new Image(path, true) : imageInPool;
}

Image *IBMApplication::getImageFromPool(const fs::path &filename) const {
    for (auto &&img : this->imagePool) {
        if (img->filename == filename) {
            return img;
        }
    }

    return nullptr;
}
