#include "SettingHandler.h"

SettingHandler::SettingHandler()
{

}

void SettingHandler::setFileName(QString name) {
    fileName.setFile(name);
}

void SettingHandler::setFileType(FileType type) {
    fileType = type;
}

bool SettingHandler::load(QString fullPath) {
    fileName.setFile(fullPath);

    QString fileExtension = fileName.suffix();
    if( ((fileType == Json) && (fileExtension != "json")) ||
        ((fileType == Dat) && (fileExtension != "dat"))) {
        return false;
    }

    QFile loadFile(fileName.absoluteFilePath());

    if (!loadFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc((fileType == Json) ? QJsonDocument::fromJson(saveData)
                            : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));


    readValue(loadDoc.object());

    return true;
}

bool SettingHandler::save(QString fullPath) {
    fileName.setFile(fullPath);

    QString fileExtension = fileName.suffix();
    if( ((fileType == Json) && (fileExtension != "json")) ||
        ((fileType == Dat) && (fileExtension != "dat"))) {
        return false;
    }

    QFile saveFile(fileName.absoluteFilePath());

    if (!saveFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject MatchRoi;
    MatchRoi["Matching_ROI"] = matchingROI.ToJson();

    saveFile.write((fileType == Json) ? QJsonDocument(MatchRoi).toJson()
                                      : QCborValue::fromJsonValue(MatchRoi).toCbor());

    return true;
}

void SettingHandler::readValue(QJsonObject json) {
    if (const QJsonValue jsonVal = json["Matching_ROI"]; jsonVal.isObject()) {
        matchingROI.FromJson(jsonVal.toObject());
    }
}

//        qDebug() << "Top left: " << matchingROI.TopLeft.x << " - " << matchingROI.TopLeft.y;
//        qDebug() << "Bottom right: " << matchingROI.BottomRight.x << " - " << matchingROI.BottomRight.y;
