#include "utils.hpp"

using namespace std;

void opencvExample() {
    auto image = Image("res/img.png", true);
    auto colorRange = ColorRange(
        cv::Scalar(37, 42, 0),
        cv::Scalar(84, 255, 255)
    );

    bool hasColor = false;
    auto mask = image.getMaskByColorRange(colorRange, &hasColor);
    auto res = image.getImageByMask(mask);

    if (hasColor) {
        cout << "Color detected!" << endl;
    }

    cv::imshow("Image", image.cv);
    cv::imshow("Res", res);
    cv::imshow("Mask", mask);
    
    cv::waitKey();
    cv::destroyAllWindows();
}

int main() {
    opencvExample();

    return 0;
}