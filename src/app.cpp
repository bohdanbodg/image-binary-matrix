#include "app.hpp"

using namespace std;
namespace fs = std::filesystem;

#define DEFAULT_NONE "<none>"
#define IMAGE_EXTENSIONS \
    { ".png", ".jpg", ".jpeg" }

#define GREEN_TEXT_COLOR (ImVec4(0.455f, 0.922f, 0.543f, 1.000f))
#define RED_TEXT_COLOR (ImVec4(0.922f, 0.455f, 0.455f, 1.000f))

GuiInputData::GuiInputData()
    : selectedImageFile(-1),
      imagePreviewOpened(false),
      maskPreviewOpened(false),
      binaryMatrixPreview(false),
      hsvFrom {0.145f, 0.165f, 0.000f, 1.0f},
      hsvTo {0.329f, 1.000f, 1.000f, 1.0f} {
}

cv::Scalar GuiInputData::hsvCV(float *hsv) const {
    return cv::Scalar(
        (double)(hsv[0] * 255),
        (double)(hsv[1] * 255),
        (double)(hsv[2] * 255)
    );
}

ColorRange GuiInputData::hsvToRange() const {
    const auto from = this->hsvCV((float *)this->hsvFrom);
    const auto to = this->hsvCV((float *)this->hsvTo);

    return ColorRange(from, to);
}

bool GuiInputData::isImageSelected() const {
    return this->selectedImageFile >= 0;
}

IBMApplication::IBMApplication(const std::string &currentExecutablePath)
    : Application("Image Binary Matrix"),
      image(nullptr),
      isImageLoaded(false),
      isMaskProcessed(false),
      toRefresh(true),
      toReset(true),
      toRegenerate(false) {
    this->path.executable = fs::absolute(fs::path(currentExecutablePath));
    this->path.directory = this->path.executable.parent_path();

    this->path.input = this->path.directory / std::string("input/");
    this->path.output = this->path.directory / std::string("output/");
}

bool IBMApplication::init() {
    if (!Application::init()) {
        return false;
    }

    this->createDirectory(this->path.input);
    this->createDirectory(this->path.output);
    this->loadImageFileList();

    auto &config = ImGui::GetIO();
    config.IniFilename = nullptr;
    config.LogFilename = nullptr;

    auto &style = ImGui::GetStyle();
    style.WindowRounding = 5;
    style.ChildRounding = 5;
    style.ChildBorderSize = 1;

    return true;
}

void IBMApplication::draw() {
    this->isImageLoaded = this->data.isImageSelected() && this->image != nullptr
                          && this->image->isLoaded();
    this->isMaskProcessed
        = this->isImageLoaded && this->image->isMaskProcessed();

    this->drawMenuBar();
    this->drawMainWindow();

    if (this->isImageLoaded) {
        if (this->data.imagePreviewOpened || this->toReset) {
            this->drawImagePreview();
        }

        if (this->data.maskPreviewOpened || this->toReset
            || this->toRegenerate) {
            this->drawMaskPreview();
        }

        if (this->data.binaryMatrixPreview || this->toReset
            || this->toRegenerate) {
            this->drawMatrixPreview();
        }
    }

    if (this->toRefresh) {
        this->toRefresh = false;
    }
    if (this->toReset) {
        this->toReset = false;
    }
    if (this->toRegenerate) {
        this->image->processMaskByColorRange(this->data.hsvToRange());
        this->generateBinaryMatrix();

        this->toRegenerate = false;
    }
}

void IBMApplication::drawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("Application")) {
        if (ImGui::MenuItem("Quit")) {
            this->terminate();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void IBMApplication::drawMainWindow() {
    int width = std::min(this->windowWidth, 600),
        height = std::min(this->windowHeight, 800);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        this->windowName,
        nullptr,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    if (ImGui::CollapsingHeader(
            "Input Image",
            ImGuiTreeNodeFlags_DefaultOpen
        )) {
        ImGui::BeginChild(
            "##input_image",
            ImVec2(0, 200),
            ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize,
            ImGuiWindowFlags_HorizontalScrollbar
        );

        auto inputImageCount = this->path.images.size();
        if (inputImageCount > 0) {
            static string maxPath = DEFAULT_NONE;

            if (this->toRefresh) {
                maxPath = (*std::max_element(
                               this->path.images.cbegin(),
                               this->path.images.cend(),
                               [](const fs::path &a, const fs::path &b) {
                                   return a.string().size() < b.string().size();
                               }
                           )).string();
            }

            ImGui::Text("Image Filename");

            auto nextWidth = ImGui::CalcTextSize(maxPath.c_str()).x
                             + ImGui::GetFontSize() * 2;
            ImGui::SetNextItemWidth(nextWidth);

            ImGui::ListBox(
                "##ImageFile",
                &this->data.selectedImageFile,
                [](void *data, int idx) -> const char * {
                    const auto &images = *(decltype(this->path.images) *)data;

                    if (idx >= 0 && idx < images.size()) {
                        return images[idx].c_str();
                    }

                    return DEFAULT_NONE;
                },
                (void *)&this->path.images,
                inputImageCount
            );
        } else {
            ImGui::TextColored(
                RED_TEXT_COLOR,
                "No images found in the input folder!"
            );
        }

        static string messageToDisplay = "";
        static ImVec4 colorToDisplay;

        if (!messageToDisplay.empty()) {
            ImGui::TextColored(colorToDisplay, "%s", messageToDisplay.c_str());
        }

        if (this->data.isImageSelected()) {
            if (ImGui::Button("Load")) {
                this->toReset = true;

                if (this->isMaskProcessed) {
                    this->toRegenerate = true;
                }

                try {
                    auto imageFilename = this->getSelectedImage();
                    auto imageInPool = this->getImageFromPool(imageFilename);

                    this->loadImage(imageFilename);

                    colorToDisplay = GREEN_TEXT_COLOR;
                    messageToDisplay
                        = ("Image \"" + imageFilename + "\" loaded from ")
                          + (imageInPool == nullptr ? "file" : "cache");
                } catch (Exception e) {
                    colorToDisplay = RED_TEXT_COLOR;
                    messageToDisplay = e.message;
                }
            }

            ImGui::SameLine();
        }

        if (ImGui::Button("Refresh")) {
            this->toRefresh = true;

            this->loadImageFileList();
        }

        if (this->isImageLoaded) {
            bool previewOpened = this->data.imagePreviewOpened;
            if (ImGui::Button(
                    previewOpened ? "Close Preview" : "Open Preview"
                )) {
                this->data.imagePreviewOpened = !previewOpened;
            }
        }

        ImGui::EndChild();

        ImGui::NewLine();
    }

    if (this->isImageLoaded || this->toReset || this->toRegenerate) {
        this->drawImageTools();
    }

    ImGui::End();
}

void IBMApplication::drawImageTools() {
    static bool generatePressed;
    static bool saved, savedMatrix;
    static string savedFilename, savedFilenameMatrix;

    if (this->toReset || this->toRegenerate) {
        generatePressed = this->toRegenerate;
        saved = savedMatrix = false;
        savedFilename = savedFilenameMatrix = "";

        return;
    }

    if (ImGui::CollapsingHeader(
            "Process Image",
            ImGuiTreeNodeFlags_DefaultOpen
        )) {
        ImGui::BeginChild(
            "##process_image",
            ImVec2(0, 200),
            ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize,
            ImGuiWindowFlags_HorizontalScrollbar
        );

        ImGui::Text("Color Range");
        ImGui::ColorEdit3(
            "From",
            this->data.hsvFrom,
            ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV
                | ImGuiColorEditFlags_Uint8
        );
        ImGui::ColorEdit3(
            "To",
            this->data.hsvTo,
            ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV
                | ImGuiColorEditFlags_Uint8
        );
        ImGui::NewLine();

        if (generatePressed) {
            if (this->isMaskProcessed) { // todo
                ImGui::TextColored(
                    GREEN_TEXT_COLOR,
                    "Mask and matrix have been generated!"
                );
            } else {
                ImGui::TextColored(
                    RED_TEXT_COLOR,
                    "Image does not contain parts within specified color range!"
                );
            }
        }

        if (ImGui::Button("Generate")) {
            this->toRegenerate = generatePressed = true;
        }

        if (this->isMaskProcessed) {
            bool maskOpened = this->data.maskPreviewOpened;
            if (ImGui::Button(
                    maskOpened ? "Close Mask Preview" : "Open Mask Preview"
                )) {
                this->data.maskPreviewOpened = !maskOpened;
            }

            bool matrixOpened = this->data.binaryMatrixPreview;
            if (ImGui::Button(
                    matrixOpened ? "Close Matrix Preview"
                                 : "Open Matrix Preview"
                )) {
                this->data.binaryMatrixPreview = !matrixOpened;
            }
        }

        ImGui::EndChild();

        ImGui::NewLine();
    }

    if (!this->isMaskProcessed) {
        return;
    }

    if (!ImGui::CollapsingHeader(
            "Output Data",
            ImGuiTreeNodeFlags_DefaultOpen
        )) {
        return;
    }

    ImGui::BeginChild(
        "##output_data",
        ImVec2(0, 150),
        ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    if (!savedFilenameMatrix.empty()) {
        if (savedMatrix) {
            ImGui::TextColored(
                GREEN_TEXT_COLOR,
                "Binary matrix has been saved as \"%s\" in the \"output\" "
                "folder!",
                savedFilenameMatrix.c_str()
            );
        } else {
            ImGui::TextColored(
                RED_TEXT_COLOR,
                "Binary matrix save error (filename: \"%s\")!",
                savedFilenameMatrix.c_str()
            );
        }
    }

    if (ImGui::Button("Save Binary Matrix to File")) {
        savedFilenameMatrix = this->buildOutputImageFilename() + ".txt";
        std::ofstream fileOutput(
            (this->path.output / savedFilenameMatrix).string()
        );

        for (size_t i = 0; i < this->matrix.rows; i++) {
            for (size_t j = 0; j < this->matrix.cols; j++) {
                fileOutput << int(this->matrix.isTrue(i, j));
            }

            fileOutput << endl;
        }
        fileOutput.close();

        savedMatrix = fileOutput.good();
    }

    ImGui::NewLine();

    if (!savedFilename.empty()) {
        if (saved) {
            ImGui::TextColored(
                GREEN_TEXT_COLOR,
                "Mask has been saved as \"%s\" in the \"output\" folder!",
                savedFilename.c_str()
            );
        } else {
            ImGui::TextColored(
                RED_TEXT_COLOR,
                "Mask save error (filename: \"%s\")!",
                savedFilename.c_str()
            );
        }
    }

    if (ImGui::Button("Save Mask to File")) {
        savedFilename = this->buildOutputImageFilename();
        saved = this->image->saveMask(path.output / savedFilename);
    }

    ImGui::EndChild();
}

void IBMApplication::drawImagePreview() {
    auto &texture = this->image->texture;
    if (this->toReset) {
        texture.reset();

        return;
    }

    int width = std::min(this->image->width(), 600),
        height = std::min(this->image->height(), 600);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        this->buildImageTitle("original").c_str(),
        &this->data.imagePreviewOpened,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    this->renderImage(texture, this->image->cv);

    ImGui::End();
}

void IBMApplication::drawMaskPreview() {
    auto &texture = this->image->maskTexture;
    if (this->toReset || this->toRegenerate) {
        texture.reset();

        return;
    }

    if (!this->isMaskProcessed) {
        return;
    }

    int width = std::min(this->image->width(), 600),
        height = std::min(this->image->height(), 600);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        this->buildImageTitle("mask").c_str(),
        &this->data.maskPreviewOpened,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    this->renderImage(texture, this->image->mask);

    ImGui::End();
}

void IBMApplication::drawMatrixPreview() {
    static vector<string> matrixRows(0);

    if (this->toReset || this->toRegenerate) {
        matrixRows.clear();

        return;
    }

    if (!this->isMaskProcessed) {
        return;
    }

    int width = std::min(this->image->width(), 600),
        height = std::min(this->image->height(), 600);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        this->buildImageTitle("matrix").c_str(),
        &this->data.binaryMatrixPreview,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    if (matrixRows.empty()) {
        for (size_t i = 0; i < this->matrix.rows; i++) {
            string row = "";

            for (size_t j = 0; j < this->matrix.cols; j++) {
                row += this->matrix.isTrue(i, j) ? "1" : "0";
            }

            matrixRows.push_back(row);
        }
    }

    for (auto &&row : matrixRows) {
        ImGui::TextUnformatted(row.c_str());
    }

    ImGui::End();
}

std::string IBMApplication::getSelectedImage() const {
    return this->path.images[this->data.selectedImageFile].string();
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

void IBMApplication::renderImage(Texture2D &texture, const cv::Mat &mat) {
    texture.render(mat);

    ImGui::Image(
        (void *)(*(intptr_t *)texture.glTexture),
        ImVec2(mat.cols, mat.rows)
    );
}

void IBMApplication::loadImageFileList() {
    this->path.images.clear();

    // Seek first image in the input folder
    for (const auto &entry :
         fs::recursive_directory_iterator(this->path.input)) {
        const auto &entryPath = entry.path();
        const auto &ext = entryPath.extension().string();

        for (auto &&e : IMAGE_EXTENSIONS) {
            if (ext == e) {
                this->path.images.push_back(entryPath.filename());
            }
        }
    }
}

void IBMApplication::createDirectory(const std::filesystem::path &path) const {
    if (!fs::is_directory(path) || !fs::exists(path)) {
        fs::create_directory(path);
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
        throw Exception("Image file not found!");
    }

    auto imageInPool = this->getImageFromPool(filename);
    if (this->image != nullptr && imageInPool == nullptr) {
        this->imagePool.push_back(this->image);
    }

    this->image = imageInPool == nullptr ? new Image(path, true) : imageInPool;
}

Image *IBMApplication::getImageFromPool(const string &filename) const {
    for (auto &&img : this->imagePool) {
        if (img->filename.string() == filename) {
            return img;
        }
    }

    return nullptr;
}

void IBMApplication::generateBinaryMatrix() {
    const auto &mask = this->image->mask;
    this->matrix.reset(mask.rows, mask.cols);

    for (size_t i = 0; i < mask.rows; i++) {
        for (size_t j = 0; j < mask.cols; j++) {
            const auto &pixel = mask.at<cv::Vec4b>(i, j);
            const auto value
                = (bool(pixel[0]) && bool(pixel[1]) && bool(pixel[2]))
                      ? BinaryMatrix::True
                      : BinaryMatrix::False;
            this->matrix.set(i, j, value);
        }
    }
}
