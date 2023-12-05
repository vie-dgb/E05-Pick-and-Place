#include "PatternDialog.h"
#include "widget/ClickLabel.h"
#include "ui_PatternDialog.h"

PatternDialog::PatternDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PatternDialog)
{
    ui->setupUi(this);

    // init windows widgets limit
    setWindowTitle("Edit model source");
    ui->spinBox_EdgeRange->setMinimum(CANNY_RANGE_MIN);
    ui->spinBox_EdgeRange->setMaximum(CANNY_RANGE_MAX);
    ui->slider_EdgeRange->setMinimum(CANNY_RANGE_MIN);
    ui->slider_EdgeRange->setMaximum(CANNY_RANGE_MAX);
    ui->spinBox_Contrast->setMinimum(CANNY_CONTRAST_MIN);
    ui->spinBox_Contrast->setMaximum(CANNY_CONTRAST_MAX);
    ui->slider_Contrast->setMinimum(CANNY_CONTRAST_MIN*100.0);
    ui->slider_Contrast->setMaximum(CANNY_CONTRAST_MAX*100.0);
    ui->spinBox_TemplateAngle->setMinimum(OBJECT_ANGLE_MIN);
    ui->spinBox_TemplateAngle->setMaximum(OBJECT_ANGLE_MAX);
    ui->spinBox_MinScores->setMinimum(MATCH_SCORES_MIN);
    ui->spinBox_MinScores->setMaximum(MATCH_SCORES_MAX);
    ui->spinBox_TemplateAngle->setSingleStep(0.1);

    // connect slot to get pick position
    connect(ui->label_Original, SIGNAL(signalEventMousePress(QMouseEvent*)),
                this, SLOT(changePickPosition(QMouseEvent*)));

}

PatternDialog::~PatternDialog()
{
    delete ui;
}

/* PRIVATE SLOTS */

void PatternDialog::changePickPosition(QMouseEvent *event) {
    modelTemp.pickPosition.x = event->position().x();
    modelTemp.pickPosition.y = event->position().y();
    updatePickPositionSpinBox();
    displayImageToLabel(ui->label_Original, modelTemp.getImageOfModelWithPickPosition());
}

void PatternDialog::on_btn_ApplyModel_clicked()
{
    // model edited, send edited model
    modelTemp.modelLearnPattern();
    emit patternModelEditFinhished(modelTemp);
    close();
}

void PatternDialog::showPatternEdit(ImageMatch::GeoModel _model) {
    // get model infor
    modelTemp = _model;
    initWindowWidgetsValue();
    updatePickPositionSpinBox();
    // display image
    displayImageToLabel(ui->label_Original, modelTemp.getImageOfModelWithPickPosition());
    displayImageToLabel(ui->label_edgeCanny, modelTemp.getImageOfModelCannyThreshold());
    // fixed dialog window size
    setFixedSize(0,0);
    // popup windows dialog, block the others windows when edit model
    setModal(true);
    show();
}

void PatternDialog::on_lineEdit_ModelName_editingFinished()
{
    if(ui->lineEdit_ModelName->text().isEmpty()) {
        ui->lineEdit_ModelName->setText(QString::fromStdString(modelTemp.nameOfModel));
    }
    else {
        modelTemp.nameOfModel = ui->lineEdit_ModelName->text().toStdString();
    }
}


void PatternDialog::on_spinBox_MinScores_editingFinished()
{
    modelTemp.minScores = ui->spinBox_MinScores->value();
}

void PatternDialog::on_slider_EdgeRange_sliderMoved(int position)
{

    edgeRangeValueChange(position);
}


void PatternDialog::on_spinBox_EdgeRange_valueChanged(int arg1)
{
    edgeRangeValueChange(arg1);
}

void PatternDialog::on_slider_Contrast_sliderMoved(int position)
{
    double value = (double)position/100.0;
    contrastValueChange(value);
}

void PatternDialog::on_spinBox_Contrast_valueChanged(double arg1)
{
    contrastValueChange(arg1);
}

void PatternDialog::on_spinBox_PickPositionX_valueChanged(int arg1)
{
    modelTemp.pickPosition.x = arg1;
//    modelTemp.pickPosition.y = ui->spinBox_PickPositionY->value();
    displayImageToLabel(ui->label_Original, modelTemp.getImageOfModelWithPickPosition());
}

void PatternDialog::on_spinBox_PickPositionY_valueChanged(int arg1)
{
//    modelTemp.pickPosition.x = ui->spinBox_PickPositionX->value();
    modelTemp.pickPosition.y = arg1;
    displayImageToLabel(ui->label_Original, modelTemp.getImageOfModelWithPickPosition());
}

void PatternDialog::on_spinBox_TemplateAngle_valueChanged(double arg1)
{
    modelTemp.setAngleOfModel(arg1, false);
    displayImageToLabel(ui->label_Original, modelTemp.getImageOfModelWithPickPosition());
}

/* PRIVATE FUNCTIONS */

void PatternDialog::initWindowWidgetsValue() {
//    qDebug() << "Min scores: " << modelTemp.minScores;
    ui->lineEdit_ModelName->setText(QString::fromStdString(modelTemp.nameOfModel));
    ui->spinBox_MinScores->setValue(modelTemp.minScores);
    ui->spinBox_EdgeRange->setValue(modelTemp.cannyRange);
    ui->slider_EdgeRange->setValue(modelTemp.cannyRange);
    ui->spinBox_Contrast->setValue(modelTemp.cannyContrast);
    ui->slider_Contrast->setValue((int)(modelTemp.cannyContrast*100.0));
    ui->spinBox_TemplateAngle->setValue(modelTemp.getAngleOfModel());

    ui->spinBox_PickPositionX->setMaximum(modelTemp.getImageOfModelCols());
    ui->spinBox_PickPositionY->setMaximum(modelTemp.getImageOfModelRows());
}

void PatternDialog::displayImageToLabel(QLabel *label, cv::Mat image) {
    cv::Mat displayImg = image.clone();
    cv::cvtColor(displayImg, displayImg, cv::COLOR_BGR2RGB);
    QImage qDisplayFrame = QImage((uchar*)displayImg.data, displayImg.cols, displayImg.rows, displayImg.step, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(qDisplayFrame));
}

inline void PatternDialog::setWidgetsCannyRangeValue(int value) {
    ui->spinBox_EdgeRange->setValue(value);
    ui->slider_EdgeRange->setValue(value);
    modelTemp.cannyRange = value;
}

void PatternDialog::edgeRangeValueChange(int value) {
    setWidgetsCannyRangeValue(value);

    if(modelTemp.isImageEmpty()) {
        return;
    }

    displayImageToLabel(ui->label_edgeCanny, modelTemp.getImageOfModelCannyThreshold());
}

inline void PatternDialog::setWidgetsCannyContrastValue(double value) {
    ui->spinBox_Contrast->setValue(value);
    ui->slider_Contrast->setValue(value*100.0);
    modelTemp.cannyContrast = value;
}

void PatternDialog::contrastValueChange(double value) {
    setWidgetsCannyContrastValue(value);

    if(modelTemp.isImageEmpty()) {
        return;
    }

    displayImageToLabel(ui->label_edgeCanny, modelTemp.getImageOfModelCannyThreshold());
}

void PatternDialog::updatePickPositionSpinBox() {
    cv::Point position = modelTemp.pickPosition;
    ui->spinBox_PickPositionX->setValue(position.x);
    ui->spinBox_PickPositionY->setValue(position.y);
}
