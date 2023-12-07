#ifndef IMAGECROPPER_H
#define IMAGECROPPER_H

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#define WARP_POINT_OFFSET   10
#define DRAG_CIRCLE_SIZE    5
#define RECT_LINE_SIZE      1
#define CIRCLE_LINE_SIZE    2

// BRG color format
#define DRAW_COLOR_RED      Scalar(0, 0, 255)
#define DRAW_COLOR_GREEN    Scalar(0, 255, 0)
#define DRAW_COLOR_BLUE     Scalar(255, 0, 0)
#define DRAW_COLOR_PURPLE   Scalar(250, 2, 250)

using namespace cv;
using namespace std;

class ImageCropper
{
public:
    ImageCropper();

    bool mousePressCropRuntimeImg(Point mousePosition);
    void mouseReleaseCropRuntimeImg();
    bool mouseMoveCropRuntimeImg(Point mousePosition);
    cv::Mat drawCropRuntimeImageRect(cv::Mat imageSource);

    bool mousePressCropPatternImg(Point mousePosition);
    void mouseReleaseCropPatternImg();
    bool mouseMoveCropPatternImg(Point mousePosition);
    cv::Mat drawCropPatternImageRect(cv::Mat imageSource);

    cv::Mat cropRuntimeImage(cv::Mat imageSource, double scale);
    cv::Mat cropRuntimeImage(cv::Mat imageSource);
    cv::Mat cropPatternImage(cv::Mat imageSource, double scale);
    cv::Mat cropPatternImage(cv::Mat imageSource);

    Point getCropPointRuntimeImage(int index);
    Point getCropPointPatternImage(int index);
    void setCropPointRuntimeImage(Point setPoint, int index);
    void setCropPointPatternImage(Point setPoint, int index);

private:
    int mouseCropRuntimeImgIndex = 0;
    int mouseCropPatternImgIndex = 0;
    Point cropPointRuntimeImage[2] = {Point(0,0), Point(800,500)};
    Point cropPointPatternImage[2] = {Point(0,0), Point(800,500)};
};

#endif // IMAGECROPPER_H
