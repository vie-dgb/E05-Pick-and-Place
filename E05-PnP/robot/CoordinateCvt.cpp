#include "CoordinateCvt.h"

CoordinateCvt::CoordinateCvt()
{

}

void CoordinateCvt::convertPickCoordinates(DescartesPoint &pickPoint, Point2f cameraPosition, double rawAngle, Size imageSize) {
    double xMaxPixel = (double)imageSize.width;
    double yMaxPixel = (double)imageSize.height;

    double xMmPerPixel = xMaxPickDistance / xMaxPixel;
    double yMmPerPixel = yMaxPickDistance / yMaxPixel;

    double xDistance = cameraPosition.x * xMmPerPixel;
    double yDistance = cameraPosition.y * yMmPerPixel;

    pickPoint.X = xDistance;
    pickPoint.Y = yDistance;
    pickPoint.Z = 0;
    pickPoint = rotateAxisX(pickPoint, -180.0, 0, yMaxPickDistance, 0);
    pickPoint = rotateAxisZ(pickPoint, -90.0, 0, xMaxPickDistance, 0);

    pickPoint.rZ = -rawAngle*C_R2D;
}

DescartesPoint CoordinateCvt::rotateAxisX(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ) {
    DescartesPoint newPoint;
    double rotateAngle = angle*C_D2R;
    double sinAngle = sin(rotateAngle);
    double cosAngle = cos(rotateAngle);
    newPoint.X = point.X;
    newPoint.Y = cosAngle*point.Y - sinAngle*point.Z;
    newPoint.Z = sinAngle*point.Y + cosAngle*point.Z;

    /// move origin
    newPoint.X += distanceX;
    newPoint.Y += distanceY;
    newPoint.Z += distanceZ;

    return newPoint;
}

DescartesPoint CoordinateCvt::rotateAxisY(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ) {
    DescartesPoint newPoint;
    double rotateAngle = angle*C_D2R;
    double sinAngle = sin(rotateAngle);
    double cosAngle = cos(rotateAngle);
    newPoint.X = cosAngle*point.X + sinAngle*point.Z;
    newPoint.Y = point.Y;
    newPoint.Z = cosAngle*point.Z - sinAngle*point.X ;

    /// move origin
    newPoint.X += distanceX;
    newPoint.Y += distanceY;
    newPoint.Z += distanceZ;

    return newPoint;
}

DescartesPoint CoordinateCvt::rotateAxisZ(DescartesPoint &point, double angle, double distanceX, double distanceY, double distanceZ) {
    DescartesPoint newPoint;
    double rotateAngle = angle*C_D2R;
    double sinAngle = sin(rotateAngle);
    double cosAngle = cos(rotateAngle);
    newPoint.X = cosAngle*point.X - sinAngle*point.Y;
    newPoint.Y = sinAngle*point.X + cosAngle*point.Y;
    newPoint.Z = point.Z;

    /// move origin
    newPoint.X += distanceX;
    newPoint.Y += distanceY;
    newPoint.Z += distanceZ;

    return newPoint;
}
