#ifndef COORDINATECVT_H
#define COORDINATECVT_H

#include "math.h"
#include "HansDefine.h"

#include <opencv2/core.hpp>

#define C_PI   3.1415926535897932384626433832795
#define C_D2R (C_PI / 180.0)
#define C_R2D (180.0 / C_PI)

using namespace rb;
using namespace cv;

class CoordinateCvt
{
public:  
    CoordinateCvt();

    void convertPickCoordinates(DescartesPoint &pickPoint, Point2f cameraPosition, double rawAngle, Size imageSize);

    static DescartesPoint rotateAxisX(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ);
    static DescartesPoint rotateAxisY(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ);
    static DescartesPoint rotateAxisZ(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ);

    double xMaxPickDistance;
    double yMaxPickDistance;
};

#endif // COORDINATECVT_H
