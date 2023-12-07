#ifndef SETTINGHANDLER_H
#define SETTINGHANDLER_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCborMap>
#include <QFile>
#include <QFileInfo>

#include <QDebug>

#include <opencv2/core.hpp>

using namespace cv;

class MatchingROI
{
public:
    MatchingROI() {}
    MatchingROI(Point topleft, Point bottomright) {
        TopLeft = topleft;
        BottomRight = bottomright;
    }

    Point TopLeft;
    Point BottomRight;

    QJsonObject ToJson() {
        QJsonObject json;
        json["Top_Left"] = PointToJson(TopLeft);
        json["Bottom_Right"] = PointToJson(BottomRight);
        return json;
    }

    void FromJson(QJsonObject json) {
        if (const QJsonValue jsonVal = json["Top_Left"]; jsonVal.isObject()) {
            JsonToPoint(jsonVal, TopLeft);
        }
        if (const QJsonValue jsonVal = json["Bottom_Right"]; jsonVal.isObject()) {
            JsonToPoint(jsonVal, BottomRight);
        }
    }

    static QJsonObject PointToJson(Point jPoint) {
        QJsonObject json;
        json["X"] = jPoint.x;
        json["Y"] = jPoint.y;
        return json;
    }

    static void JsonToPoint(const QJsonValue &json, Point &point) {
        point.x = json["X"].toInt();
        point.y = json["Y"].toInt();
    }
};


class SettingHandler
{
public:
    enum FileType : int {
        Json,
        Dat
    };

    SettingHandler();

    void setFileName(QString name);
    void setFileType(FileType type);
    bool load(QString fullPath);
    bool save(QString fullPath);

private:
    void readValue(QJsonObject json);

public:
    MatchingROI matchingROI;

private:
    QFileInfo fileName;
    FileType fileType;
};

#endif // SETTINGHANDLER_H
