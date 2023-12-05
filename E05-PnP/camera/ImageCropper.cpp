#include "ImageCropper.h"
#include "math.h"

ImageCropper::ImageCropper()
{

}

bool ImageCropper::mousePressCropRuntimeImg(Point mousePosition) {
    for(int idxCounter = 0; idxCounter < 2; idxCounter++) {
        int coorX = cropPointRuntimeImage[idxCounter].x;
        int coorY = cropPointRuntimeImage[idxCounter].y;
        if( (mousePosition.x >= (coorX - WARP_POINT_OFFSET)) && (mousePosition.x <= (coorX  + WARP_POINT_OFFSET)) ) {
            if( (mousePosition.y >= (coorY - WARP_POINT_OFFSET)) && (mousePosition.y <= (coorY  + WARP_POINT_OFFSET)) )  {
                mouseCropRuntimeImgIndex = idxCounter + 1;
                return true;
            }
        }
    }

    return false;
}

void ImageCropper::mouseReleaseCropRuntimeImg() {
    mouseCropRuntimeImgIndex = 0;
}

bool ImageCropper::mouseMoveCropRuntimeImg(Point mousePosition) {
    if (mouseCropRuntimeImgIndex == 0) {
        return false;
    }
    cropPointRuntimeImage[mouseCropRuntimeImgIndex - 1] = mousePosition;
    return true;
}

cv::Mat ImageCropper::drawCropRuntimeImageRect(cv::Mat imageSource) {
    cv::Mat returnImg = imageSource.clone();
    cv::rectangle(returnImg, cropPointRuntimeImage[0], cropPointRuntimeImage[1], DRAW_COLOR_RED, RECT_LINE_SIZE, LINE_AA);
    cv::circle(returnImg, cropPointRuntimeImage[0], DRAG_CIRCLE_SIZE, DRAW_COLOR_GREEN, CIRCLE_LINE_SIZE, LINE_AA);
    cv::circle(returnImg, cropPointRuntimeImage[1], DRAG_CIRCLE_SIZE, DRAW_COLOR_GREEN, CIRCLE_LINE_SIZE, LINE_AA);
    return returnImg;
}

bool ImageCropper::mousePressCropPatternImg(Point mousePosition) {
    for(int idxCounter = 0; idxCounter < 2; idxCounter++) {
        int coorX = cropPointPatternImage[idxCounter].x;
        int coorY = cropPointPatternImage[idxCounter].y;
        if( (mousePosition.x >= (coorX - WARP_POINT_OFFSET)) && (mousePosition.x <= (coorX  + WARP_POINT_OFFSET)) ) {
            if( (mousePosition.y >= (coorY - WARP_POINT_OFFSET)) && (mousePosition.y <= (coorY  + WARP_POINT_OFFSET)) )  {
                mouseCropPatternImgIndex = idxCounter + 1;
                return true;
            }
        }
    }

    return false;
}

void ImageCropper::mouseReleaseCropPatternImg() {
    mouseCropPatternImgIndex = 0;
}

bool ImageCropper::mouseMoveCropPatternImg(Point mousePosition) {
    if (mouseCropPatternImgIndex == 0) {
        return false;
    }
    cropPointPatternImage[mouseCropPatternImgIndex - 1] = mousePosition;
    return true;
}

cv::Mat ImageCropper::drawCropPatternImageRect(cv::Mat imageSource) {
    cv::Mat returnImg = imageSource.clone();

    cv::rectangle(returnImg, cropPointPatternImage[0], cropPointPatternImage[1], DRAW_COLOR_RED, RECT_LINE_SIZE, LINE_AA);

    cv::circle(returnImg, cropPointPatternImage[0], DRAG_CIRCLE_SIZE, DRAW_COLOR_GREEN, CIRCLE_LINE_SIZE, LINE_AA);
    cv::circle(returnImg, cropPointPatternImage[1], DRAG_CIRCLE_SIZE, DRAW_COLOR_GREEN, CIRCLE_LINE_SIZE, LINE_AA);
    return returnImg;
}

cv::Mat ImageCropper::cropRuntimeImage(cv::Mat imageSource, double scale) {

    Point2i topLeft, botRight;

    topLeft.x = (int)(cropPointRuntimeImage[0].x * (1 / scale));
    topLeft.y = (int)(cropPointRuntimeImage[0].y * (1 / scale));

    botRight.x = (int)(cropPointRuntimeImage[1].x * (1 / scale));
    botRight.y = (int)(cropPointRuntimeImage[1].y * (1 / scale));

    return imageSource(Range(topLeft.y, botRight.y), Range(topLeft.x, botRight.x));
}

cv::Mat ImageCropper::cropRuntimeImage(cv::Mat imageSource) {
    return imageSource(Range(cropPointRuntimeImage[0].y, cropPointRuntimeImage[1].y),
                       Range(cropPointRuntimeImage[0].x, cropPointRuntimeImage[1].x));
}

cv::Mat ImageCropper::cropPatternImage(cv::Mat imageSource, double scale) {
    Point2i topLeft, botRight;

    topLeft.x = (int)(cropPointPatternImage[0].x * (1 / scale));
    topLeft.y = (int)(cropPointPatternImage[0].y * (1 / scale));

    botRight.x = (int)(cropPointPatternImage[1].x * (1 / scale));
    botRight.y = (int)(cropPointPatternImage[1].y * (1 / scale));

    return imageSource(Range(topLeft.y, botRight.y), Range(topLeft.x, botRight.x));
}

cv::Mat ImageCropper::cropPatternImage(cv::Mat imageSource) {
    return imageSource(Range(cropPointPatternImage[0].y, cropPointPatternImage[1].y),
                       Range(cropPointPatternImage[0].x, cropPointPatternImage[1].x));
}

Point ImageCropper::getCropPointRuntimeImage(int index){
    if((index >= 0) && (index < 2)) {
        return cropPointRuntimeImage[index];
    }

    return Point(0, 0);
}

Point ImageCropper::getCropPointPatternImage(int index) {
    if((index >= 0) && (index < 2)) {
        return cropPointPatternImage[index];
    }

    return Point(0, 0);
}

void ImageCropper::setCropPointRuntimeImage(Point setPoint,int index) {
    if((index >= 0) && (index < 2)) {
        cropPointRuntimeImage[index] = setPoint;
    }
}

void ImageCropper::setCropPointPatternImage(Point setPoint, int index) {
    if((index >= 0) && (index < 2)) {
        cropPointPatternImage[index] = setPoint;
    }
}

