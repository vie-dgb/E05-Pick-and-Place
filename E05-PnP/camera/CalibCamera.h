#ifndef CALIBCAMERA_H
#define CALIBCAMERA_H

#include <QDialog>
#include <QMessageBox>
#include <widget/ClickLabel.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "camera/ImageCropper.h"

namespace Ui {
class CalibCamera;
}

class CalibCamera : public QDialog
{
    Q_OBJECT

public:
    explicit CalibCamera(QWidget *parent = nullptr, ImageCropper* const& cropper = nullptr);
    ~CalibCamera();

    void show_CalibDialog();
    void updateNewFrame(cv::Mat frame);
    void displayFrame();

private:
    void dialogInit();

    void label_View_MouseMove(QMouseEvent* event);
    void label_View_MousePress(QMouseEvent* event);
    void label_View_MouseRelease(QMouseEvent* event);

    void btn_Clicked_Done();
    void user_FormFinished();

signals:
    void calibCameraDone(bool b_Done);

private:
    Ui::CalibCamera *ui;
    ImageCropper *FrameCropper;
    cv::Mat calibFrame;
    bool userPressDone;
    int frameReceiveCounter;
};

#endif // CALIBCAMERA_H
