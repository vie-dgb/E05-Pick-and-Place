#include "GeoMatch.h"

namespace ImageMatch {

    GeoMatch::GeoMatch()
    {

    }
    GeoMatch::~GeoMatch()
    {

    }

    bool GeoMatch::addGeoMatchModel(String pathTemplate, String modelName) {
        GeoModel tempModel(pathTemplate, modelName);
        if (tempModel.isImageEmpty()) {
            return false;
        }
        tempModel.modelLearnPattern();

        ModelSrc.push_back(tempModel);
        return true;
    }

    void GeoMatch::clearMatchModel(void) {
        ModelSrc.clear();
    }

    void GeoMatch::removeMatchModel(int index) {
        if ((index < 0) || (index >= MAX_NUM_MODEL)) {
            return;
        }
        ModelSrc.erase(ModelSrc.begin() + index);
    }

    void GeoMatch::removeMatchModel(int startIndex, int endIndex) {
        if (startIndex >= endIndex) {
            return;
        }
        if ( ((startIndex < 0) || (startIndex >= MAX_NUM_MODEL))
            || ((endIndex < 0) || (endIndex >= MAX_NUM_MODEL))) {
            return;
        }
        ModelSrc.erase(ModelSrc.begin() + startIndex, ModelSrc.begin() + endIndex);
    }

    void GeoMatch::modifyMatchModelAt(int index, GeoModel model) {
        ModelSrc[index] = model;
    }

    vector<GeoModel> GeoMatch::getModelSource() {
        return ModelSrc;
    }

    int GeoMatch::getModelSourceSize(void) {
        return (int)ModelSrc.size();
    }

    void GeoMatch::setImageSource(string path) {
        imageSource = imread(path);
    }

    void GeoMatch::setImageSource(Mat img) {
        img.copyTo(imageSource);
    }

//    bool GeoMatch::haveObjectsInPlate() {
//        return objectsInPlate;
//    }

    void GeoMatch::matching(Mat &image, bool boudingBoxChecking, int ObjectsNum) {
        image.copyTo(imageSource);
        matching(boudingBoxChecking, ObjectsNum);
    }

    void GeoMatch::matching(bool boudingBoxChecking, int ObjectsNum) {
        if (imageSource.empty()) {
            return;
        }

        imageSource.copyTo(matchingResult.Image);
        matchingResult.isFoundMatchObject = false;
        if (ModelSrc.empty()) {
            matchingResult.Objects.clear();
            matchingResult.ExecutionTime = 0;
            matchingResult.hasObjectsInPlate = false;
            matchingResult.isAreaLessThanLimits = false;
            return;
        }

        //// START MATCHING -> TIME MARKED
        auto start = std::chrono::high_resolution_clock::now();

        /// MATCHING START
        Mat imageGray;
        Mat imageThresh;
        vector<vector<Point>> srcContours;
        vector<Vec4i> srcHierarchy;
        // pre-processing
        cvtColor(imageSource, imageGray, COLOR_RGB2GRAY);
        GaussianBlur(imageGray, imageGray, cv::Size(3,3), 0);
        // find contours
        threshold(imageGray, imageThresh, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
        findContours(imageThresh, srcContours, srcHierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

//        cv::imshow("Thresh contours", imageThresh);

        // reset flag indicator object in plate
        bool objectsInPlate = false;
        int sumArea = 0;

        // check contours possible matched object
        vector<PossibleObject> objects;
        vector<int> possibleCollisionContourIndex;
//        vector<int> collisionIndex;
//        const int minNoiseArea = (int)(imageSource.cols*imageSource.rows*0.01);
        const int maxNoiseArea = (int)(imageSource.cols*imageSource.rows*0.9);
        for(int conCounter=0;conCounter<srcContours.size();conCounter++) {
            PossibleObject tempObject;
            int area = contourArea(srcContours[conCounter]);

            // check has any object in plate, use condition to ignore noise
            // when running in release case use suitables condition
//            if(area <= minNoiseArea) {
//                continue;
//            }

            // find outside contours for check picking box collision
            possibleCollisionContourIndex.push_back(conCounter);

            objectsInPlate = true;
            // ignore largest contours and remove children relate with this contours.
            if(area >= maxNoiseArea) {
                continue;
            }

            sumArea += area;

            // check objects possible match with models in model source
            for(int modelCounter=0;modelCounter<ModelSrc.size();modelCounter++) {
                // get area in limit lower and upper based model area
                int lowerArea = ModelSrc[modelCounter].getModelContoursSelectedArea()*lowerThreshRatio;
                int upperArea = ModelSrc[modelCounter].getModelContoursSelectedArea()*upperThreshRatio;
                if((area <= lowerArea) || (area >= upperArea)) {
                    continue;
                }
                tempObject.modelCheckList.push_back(modelCounter);
            }

            if(tempObject.modelCheckList.size() > 0) {
                tempObject.conIndex = conCounter;
                tempObject.conBoundingRect = boundingRect(srcContours[conCounter]);
                tempObject.conMinRectArea = minAreaRect(srcContours[conCounter]);
                GeoModel::computePcaOrientation(srcContours[conCounter], tempObject.pcaAngle, tempObject.pcaCenter);
                objects.push_back(tempObject);
            }
        }

        // matching with input model source
        matchingResult.Objects.clear();
        for(int objCounter=0;objCounter<objects.size();objCounter++) {
            double maxScores = 0.0;
            MatchedObjects matchObj;

            for(int modelCounter=0;modelCounter<objects[objCounter].modelCheckList.size();modelCounter++) {
                int modelIndex = objects[objCounter].modelCheckList[modelCounter];

                double rawAngle = ModelSrc[modelIndex].getModelRawPcaAngle();
                Point2f ROICenter = objects[objCounter].pcaCenter;

                objects[objCounter].rotPositive.angle = objects[objCounter].pcaAngle + rawAngle;
                getRotatedROI(imageSource, objects[objCounter].rotPositive, ROICenter, objects[objCounter].conMinRectArea);
                if(matchingScores(objects[objCounter].rotPositive, ModelSrc[modelIndex], maxScores)) {
                    saveMatchedObjectInfo(matchObj, objects[objCounter].rotPositive, ModelSrc[modelIndex], maxScores, objects[objCounter].pcaCenter);
                    break;
                }

                objects[objCounter].rotNegative.angle = objects[objCounter].pcaAngle - rawAngle;
                getRotatedROI(imageSource, objects[objCounter].rotNegative, ROICenter, objects[objCounter].conMinRectArea);
                if(matchingScores(objects[objCounter].rotNegative, ModelSrc[modelIndex], maxScores)) {
                    saveMatchedObjectInfo(matchObj, objects[objCounter].rotNegative, ModelSrc[modelIndex], maxScores, objects[objCounter].pcaCenter);
                    break;
                }

                objects[objCounter].rotPositive_reverse.angle = objects[objCounter].pcaAngle + rawAngle + CV_PI;
                getRotatedROI(imageSource, objects[objCounter].rotPositive_reverse, ROICenter, objects[objCounter].conMinRectArea);
                if(matchingScores(objects[objCounter].rotPositive_reverse, ModelSrc[modelIndex], maxScores)) {
                    saveMatchedObjectInfo(matchObj, objects[objCounter].rotPositive_reverse, ModelSrc[modelIndex], maxScores, objects[objCounter].pcaCenter);
                    break;
                }

                objects[objCounter].rotNegative_reverse.angle = objects[objCounter].pcaAngle - rawAngle + CV_PI;
                getRotatedROI(imageSource, objects[objCounter].rotNegative_reverse, ROICenter, objects[objCounter].conMinRectArea);
                if(matchingScores(objects[objCounter].rotNegative_reverse, ModelSrc[modelIndex], maxScores)) {
                    saveMatchedObjectInfo(matchObj, objects[objCounter].rotNegative_reverse, ModelSrc[modelIndex], maxScores, objects[objCounter].pcaCenter);
                    break;
                }
            }

            if(!matchObj.image.empty()) {
                //// CHECK PICKING BOX HAS INTERSECTION WITH ANOTHER BOUNDING BOX
                bool hasCollision = false;
                // create object picking box
                matchObj.pickingBox = RotatedRect(matchObj.coordinates, pickingBoxSize, matchObj.angle*R2D);
                // checking has picking box collision with any possible object contours.
                vector<Point2f> pickingBouding;
                matchObj.pickingBox.points(pickingBouding);
                for(int boxCounter=0; boxCounter<possibleCollisionContourIndex.size(); boxCounter++) {
                    int collisionBoxIdx = possibleCollisionContourIndex[boxCounter];
                    // ignore object have same index of contours with collision contour and other child contours
                    if((objects[objCounter].conIndex == collisionBoxIdx) ||
                        (objects[objCounter].conIndex == srcHierarchy[collisionBoxIdx][3])) {
                        continue;
                    }

                    // find intersection
                    if(checkPickBoxCollision(pickingBouding, srcContours[collisionBoxIdx])) {
                        hasCollision = true;
                        break;
                    }
                }

                if(!hasCollision) {
                    matchObj.indexOfSample = objCounter;
                    matchingResult.Objects.push_back(matchObj);
                    matchingResult.isFoundMatchObject = true;

                    if(matchingResult.Objects.size() >= ObjectsNum) {
                        break;
                    }
                }
                else {
                    if(!boudingBoxChecking) {
                        drawPickingBox(matchingResult.Image, matchObj.pickingBox, DRAW_COLOR_YELLOW);
                    }
                }
            }
        }

        //// MATCHING DONE -> CALCULATE EXECUTION TIME
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<chrono::milliseconds>(stop - start);
//        std::cout << "Time execution: " << duration.count()  << "ms" << std::endl;

        // Draw picking box
//        int checkIndex = 0;
        for(int i=0;i<matchingResult.Objects.size();i++) {
            GeoModel::drawPcaAxis(matchingResult.Image, matchingResult.Objects[i].coordinates, matchingResult.Objects[i].angle, 20);
            drawPickingBox(matchingResult.Image, matchingResult.Objects[i].pickingBox, DRAW_COLOR_RED);
//            std::cout << "Max scores: " << matchObjList[i].scores << ", Name: " << matchObjList[i].name
//                      << ", Pick position: " << matchObjList[i].coordinates << std::endl;
        }

        // find max model area for check material quantity
        int maxModelSize = 0;
        for(int counter=1;counter<ModelSrc.size();counter++) {
            if(maxModelSize < ModelSrc[counter].getModelContoursSelectedArea()) {
                maxModelSize = ModelSrc[counter].getModelContoursSelectedArea();
            }
        }

        matchingResult.ExecutionTime = duration.count();
        matchingResult.hasObjectsInPlate = objectsInPlate;
        matchingResult.isAreaLessThanLimits = (sumArea < maxModelSize*5) ? true : false;
    }

    void GeoMatch::saveMatchedObjectInfo(MatchedObjects& matched, RotatedObject& objectRotated, GeoModel model, double scores, Point2f pcaCenter) {
        matched.name = model.nameOfModel;
        matched.image = objectRotated.image;
        matched.angle = objectRotated.angle + model.getAngleOfModel(true);
        matched.scores = scores;

        // Transform to find pick position
//        Point modelCenter = model.getModelCenterPCA();
//        int xDist = model.pickPosition.x - modelCenter.x;
//        int yDist = model.pickPosition.y - modelCenter.y;
//        matched.coordinates.x = pcaCenter.x + ((cos(objectRotated.angle)*xDist) - (sin(objectRotated.angle)*yDist));
//        matched.coordinates.y = pcaCenter.y + ((sin(objectRotated.angle)*xDist) + (cos(objectRotated.angle)*yDist));

        // Transform to find pick position with adjust center of max scores
        Point2f modelPickDistance;
        modelPickDistance.x = model.pickPosition.x - model.getPatternOfModel()[0].Center.x;
        modelPickDistance.y = model.pickPosition.y - model.getPatternOfModel()[0].Center.y;

        Point2f srcPatternDistance;
        srcPatternDistance.x = objectRotated.centerMaxScores.x - objectRotated.centerCrop.x;
        srcPatternDistance.y = objectRotated.centerMaxScores.y - objectRotated.centerCrop.y;

        int xDist = srcPatternDistance.x + modelPickDistance.x;
        int yDist = srcPatternDistance.y + modelPickDistance.y;

        matched.coordinates.x = pcaCenter.x + ((cos(objectRotated.angle)*xDist) - (sin(objectRotated.angle)*yDist));
        matched.coordinates.y = pcaCenter.y + ((sin(objectRotated.angle)*xDist) + (cos(objectRotated.angle)*yDist));
    }

    void GeoMatch::getRotatedROI(Mat& matSrc, RotatedObject& object, Point center, RotatedRect minRectArea) {
        int srcCols = matSrc.cols;
        int srcRows = matSrc.rows;
        Mat rotationMatrix;
        rotationMatrix = getRotationMatrix2D(center, object.angle*R2D, 1.0);
        // calculate for resize matrix
        double xVar[4] = { rotationMatrix.at<double>(0, 0),
                          rotationMatrix.at<double>(0, 1),
                          rotationMatrix.at<double>(1, 0),
                          rotationMatrix.at<double>(1, 1) };
        // find image rotated rectangle verticies
        vector<Point> rotatedRect;
        rotatedRect.push_back(Point2i(0, 0));
        rotatedRect.push_back(Point2i(xVar[0] * srcCols, xVar[2] * srcCols));
        rotatedRect.push_back(Point2i((xVar[0] * srcCols + xVar[1] * srcRows), (xVar[2] * srcCols + xVar[3] * srcRows)));
        rotatedRect.push_back(Point2i(xVar[1] * srcRows, xVar[3] * srcRows));
        Rect BoundingRect = boundingRect(rotatedRect);
        // resize matrix to wrap all image
        rotationMatrix.at<double>(0, 2) = -BoundingRect.x;
        rotationMatrix.at<double>(1, 2) = -BoundingRect.y;
        // rotated image
        warpAffine(matSrc, object.image, rotationMatrix, Size(BoundingRect.width, BoundingRect.height));
        // transform vector
        vector<Point2f> transformBoundingBox;
        minRectArea.points(transformBoundingBox);
        transform(transformBoundingBox, transformBoundingBox, rotationMatrix);
        BoundingRect = boundingRect(transformBoundingBox);

        object.image = cropImageWithBorderOffset(object.image, BoundingRect, 2);

        // get center off PCA for transform pickposition
        vector<Point2f> transformCenterPoint;
        transformCenterPoint.push_back(center);
        transform(transformCenterPoint, transformCenterPoint, rotationMatrix);
        Point2f topleft = BoundingRect.tl();
        object.centerCrop = transformCenterPoint[0] - topleft;
    }

    bool GeoMatch::matchingScores(RotatedObject& objectRotated, GeoModel& model, double& lastMaxScores) {

        Mat imgDest;
        Mat gx;
        Mat gy;
        Mat magnitude;
        Mat angle;
        Point2f center;

        const int centerTolerantRow = (int)(objectRotated.image.rows*0.1);
        const int centerTolerantCol = (int)(objectRotated.image.cols*0.1);

        cvtColor(objectRotated.image, imgDest, COLOR_RGB2GRAY);
        Sobel(imgDest, gx, CV_64F, 1, 0, 3);
        Sobel(imgDest, gy, CV_64F, 0, 1, 3);
        cartToPolar(gx, gy, magnitude, angle);

        vector<GeoModel::ModelPattern> modelPattern = model.getPatternOfModel();

        // ncc match search
        long noOfCordinates = modelPattern.size();
        // normalized min score
        double normMinScore = model.minScores / noOfCordinates;
        double normGreediness = ((1 - model.greediness * model.minScores) / (1 - model.greediness)) / noOfCordinates;
        double partialScore = 0;
        double resultScore = 0;

        Point2f offset = model.getDistanceCenterPattern();
        int startRowIdx = objectRotated.centerCrop.y + offset.y - centerTolerantRow;
        int endRowIdx = objectRotated.centerCrop.y + offset.y + centerTolerantRow;
        int startColIdx = objectRotated.centerCrop.x + offset.x - centerTolerantCol;
        int endColIdx = objectRotated.centerCrop.x + offset.x + centerTolerantCol;

        for (int rowIdx = startRowIdx; rowIdx < endRowIdx; rowIdx++)
        {
            for (int colIdx = startColIdx; colIdx < endColIdx; colIdx++)
            {
                double partialSum = 0;
                for (int count = 0; count < noOfCordinates; count++)
                {
                    GeoModel::ModelPattern tempPoint = modelPattern[count];

                    int CoorX = (int)(colIdx + tempPoint.Offset.x);
                    int CoorY = (int)(rowIdx + tempPoint.Offset.y);

                    double iTx = tempPoint.Derivative.x;
                    double iTy = tempPoint.Derivative.y;

                    // ignore invalid pixel
                    if (CoorX < 0 || CoorY < 0 || CoorY >(imgDest.rows - 1) || CoorX >(imgDest.cols - 1)) {
                        continue;
                    }

                    double iSx = gx.at<double>(CoorY, CoorX);
                    double iSy = gy.at<double>(CoorY, CoorX);

                    if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
                    {
                        double mag = magnitude.at<double>(CoorY, CoorX);
                        double matGradMag = (mag == 0) ? 0 : 1 / mag;
                        partialSum += ((iSx * iTx) + (iSy * iTy)) * (tempPoint.Magnitude * matGradMag);
                    }

                    int sumOfCoords = count + 1;

                    partialScore = partialSum / sumOfCoords;

                    double minBreakScores = std::min((model.minScores - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords);
                    if (partialScore < minBreakScores) {
                        break;
                    }

                    if (partialScore < lastMaxScores) {
                        break;
                    }
                }

                if (partialScore > resultScore) {
                    resultScore = partialScore;
                    center.x = colIdx;
                    center.y = rowIdx;
                }
            }
        }

        if( resultScore > model.minScores) {
            if (resultScore > lastMaxScores) {
                lastMaxScores = resultScore;
                objectRotated.centerMaxScores = center;
                return true;
            }
        }
        return false;
    }

    Mat GeoMatch::cropImageWithBorderOffset(Mat sourceImage, Rect boxBounding, int border) {

        Mat outputMat;
        int minOffset_X, maxOffset_X;
        int minOffset_Y, maxOffset_Y;

        Point topLeft = boxBounding.tl();
        Point botRight = boxBounding.br();

        minOffset_X = topLeft.x - border;
        maxOffset_X = botRight.x + border;

        minOffset_Y = topLeft.y - border;
        maxOffset_Y = botRight.y + border;

        if (minOffset_X < 0) {
            minOffset_X = 0;
        }

        if (maxOffset_X > sourceImage.cols) {
            maxOffset_X = sourceImage.cols;
        }

        if (minOffset_Y < 0) {
            minOffset_Y = 0;
        }

        if (maxOffset_Y > sourceImage.rows) {
            maxOffset_Y = sourceImage.rows;
        }

        outputMat = sourceImage(Range(minOffset_Y, maxOffset_Y), Range(minOffset_X, maxOffset_X));

        return outputMat;
    }

    bool GeoMatch::checkPickBoxCollision(std::vector<Point2f> pickingBox, std::vector<Point> contour) {
        for(int idx=0;idx<contour.size();idx++) {
            if(pointPolygonTest(pickingBox, contour.at(idx), false) >= 0) {
                return true;
            }
        }
        return false;
    }

    void GeoMatch::drawPickingBox(Mat& matSrc, RotatedRect rectRot, Scalar color) {
        cv::Point2f vertices2f[4];
        rectRot.points(vertices2f);

        cv::Point vertices[4];
        for(int i = 0; i < 4; ++i){
            vertices[i] = vertices2f[i];
        }

        line(matSrc, vertices[0], vertices[1], color, 1, LineTypes::LINE_AA);
        line(matSrc, vertices[1], vertices[2], color, 1, LineTypes::LINE_AA);
        line(matSrc, vertices[2], vertices[3], color, 1, LineTypes::LINE_AA);
        line(matSrc, vertices[3], vertices[0], color, 1, LineTypes::LINE_AA);
    }
}
