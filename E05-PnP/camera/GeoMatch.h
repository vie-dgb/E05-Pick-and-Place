#ifndef GEOMATCH_H
#define GEOMATCH_H

#include <chrono>
//#include <opencv2/highgui.hpp>

#include "camera/GeoModel.h"

#define MAX_NUM_MODEL (int)10
#define OBJECT_ROI_BORDER_OFFSET (int)2
#define SOURCE_THERSHOLD_TOLERANCE  (double)0.3

using namespace cv;
using namespace std;

namespace ImageMatch {

    struct RotatedObject {
        Mat image;
        Point2f centerCrop;
        Point2f centerMaxScores;
        double angle;
    };

    struct PossibleObject
    {
        // angle find by pca
        double pcaAngle;
        // center find by pca
        Point2f pcaCenter;
        // contours index of contours source
        int conIndex;
        // contours area
//        int conArea;
        // contours center
//        Point2f conCenter;
        // contours bouding rectangle
        Rect conBoundingRect;
        // contours min Rectange area
        RotatedRect conMinRectArea;
        // list model need check
        vector<int> modelCheckList;
        // rotated image with neative offset
        RotatedObject rotNegative;
        // rotated image with positive offset
        RotatedObject rotPositive;
        // rotated image with positive offset reserve 180 deg
        RotatedObject rotNegative_reverse;
        // rotated image with positive offset reserve 180 deg
        RotatedObject rotPositive_reverse;
    };

    struct MatchedObjects
    {
        std::string name;
        double scores;
        int indexOfSample;
        Point2f coordinates;
        double angle;
        RotatedRect pickingBox;
        Mat image;
    };

    struct MatchResult
    {
        vector<MatchedObjects> Objects;
        double ExecutionTime;
        cv::Mat Image;
        int imageCols;
        int imageRows;
        bool isFoundMatchObject;
        bool hasObjectsInPlate;
        bool isAreaLessThanLimits;
    };

    class GeoMatch
    {
    public:
        GeoMatch();
        ~GeoMatch();

        bool addGeoMatchModel(String pathTemplate, String modelName);
        void clearMatchModel(void);
        void removeMatchModel(int index);
        void removeMatchModel(int startIndex, int endIndex);
        void modifyMatchModelAt(int index, GeoModel model);
        vector<GeoModel> getModelSource();
        int getModelSourceSize();
        void setImageSource(string path);
        void setImageSource(Mat img);

        void matching(Mat &image, bool boudingBoxChecking, int ObjectsNum);
        void matching(bool boudingBoxChecking, int ObjectsNum);

    private:
        void getRotatedROI(Mat& matSrc, RotatedObject& object, Point center, RotatedRect minRectArea);
        bool matchingScores(RotatedObject& objectRotated, GeoModel& model, double& lastMaxScores);
        void saveMatchedObjectInfo(MatchedObjects& matched, RotatedObject& objectRotated, GeoModel model, double scores, Point2f pcaCenter);
        bool checkPickBoxCollision(std::vector<Point2f> pickingBox, std::vector<Point> contour);

        static Mat cropImageWithBorderOffset(Mat sourceImage, Rect boxBounding,int border);
        static void drawPickingBox(Mat& matSrc, RotatedRect rectRot, Scalar color);

    public:
        // matching result data
        MatchResult matchingResult;
        // picking box size
        Size2f pickingBoxSize;

    private:
        // model source for matching
        vector<GeoModel> ModelSrc;
        // matched object list after matching
        vector<MatchedObjects> matchedList;
        // image source for matching
        Mat imageSource;
        // lower object threshold ratio
        const double lowerThreshRatio = 1.0 - SOURCE_THERSHOLD_TOLERANCE;
        // upper object threshold ratio
        const double upperThreshRatio = 1.0 + SOURCE_THERSHOLD_TOLERANCE;
    };

}

#endif // GEOMATCH_H
