#include "CalibCamera.h"
#include "ui_CalibCamera.h"

CalibCamera::CalibCamera(QWidget *parent, ImageCropper* const& cropper) :
    QDialog(parent),
    ui(new Ui::CalibCamera)
{
    ui->setupUi(this);
    FrameCropper = cropper;
    dialogInit();
}

CalibCamera::~CalibCamera()
{
    delete ui;
}

void CalibCamera::show_CalibDialog() {
    userPressDone = false;
    frameReceiveCounter = 0;
    setModal(true);
    showFullScreen();
}

void CalibCamera::updateNewFrame(cv::Mat frame) {
    frame.copyTo(calibFrame);
    frameReceiveCounter++;
    ui->label_FrameCounter->setText("Frame received counter: " + QString::number(frameReceiveCounter));
    displayFrame();
}

void CalibCamera::dialogInit() {
    connect(ui->label_DisplayFrame, &ClickLabel::signalEventMouseMove, this, &CalibCamera::label_View_MouseMove);
    connect(ui->label_DisplayFrame, &ClickLabel::signalEventMousePress, this, &CalibCamera::label_View_MousePress);
    connect(ui->label_DisplayFrame, &ClickLabel::signalEventMouseRelease, this, &CalibCamera::label_View_MouseRelease);

    connect(ui->btn_Done, &QPushButton::clicked, this, &CalibCamera::btn_Clicked_Done);
    connect(this, &CalibCamera::finished, this, &CalibCamera::user_FormFinished);
}

void CalibCamera::label_View_MouseMove(QMouseEvent* event) {
    cv::Point mouseCoordinates;
    mouseCoordinates.x = event->x();
    mouseCoordinates.y = event->y();

    FrameCropper->mouseMoveCropRuntimeImg(mouseCoordinates);
    displayFrame();
}

void CalibCamera::label_View_MousePress(QMouseEvent* event) {
    cv::Point mouseCoordinates;
    mouseCoordinates.x = event->x();
    mouseCoordinates.y = event->y();
    FrameCropper->mousePressCropRuntimeImg(mouseCoordinates);
}

void CalibCamera::label_View_MouseRelease(QMouseEvent* event) {
    FrameCropper->mouseReleaseCropRuntimeImg();
    displayFrame();
}

void CalibCamera::btn_Clicked_Done() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Test", "Quit?",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        userPressDone = true;
        close();
    }
}

void CalibCamera::user_FormFinished() {
    emit calibCameraDone(userPressDone);
}

void CalibCamera::displayFrame() {
    ui->label_DisplayFrame->setMaximumHeight(calibFrame.rows);
    ui->label_DisplayFrame->setMaximumWidth(calibFrame.cols);

    cv::Mat cvRGBFrame = FrameCropper->drawCropRuntimeImageRect(calibFrame);
//    double scaleFactor = (double)ui->label_DisplayFrame->maximumHeight() / (double)calibFrame.rows;
//    cv::resize(calibFrame, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    QImage qDisplayFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->label_DisplayFrame->setPixmap(QPixmap::fromImage(qDisplayFrame));
}
