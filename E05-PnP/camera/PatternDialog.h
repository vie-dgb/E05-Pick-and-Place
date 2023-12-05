#ifndef PATTERNDIALOG_H
#define PATTERNDIALOG_H

#include <QDialog>
#include <QLabel>
#include "camera/GeoModel.h"

namespace Ui {
class PatternDialog;
}

class PatternDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatternDialog(QWidget *parent = nullptr);
    ~PatternDialog();

    void showPatternEdit(ImageMatch::GeoModel _model);

signals:
    void patternModelEditFinhished(ImageMatch::GeoModel editedModel);

private slots:
    void changePickPosition(QMouseEvent *event);

    void on_btn_ApplyModel_clicked();
    void on_lineEdit_ModelName_editingFinished();
    void on_spinBox_MinScores_editingFinished();
    void on_slider_EdgeRange_sliderMoved(int position);
    void on_spinBox_EdgeRange_valueChanged(int arg1);
    void on_slider_Contrast_sliderMoved(int position);
    void on_spinBox_Contrast_valueChanged(double arg1);
    void on_spinBox_PickPositionX_valueChanged(int arg1);
    void on_spinBox_PickPositionY_valueChanged(int arg1);
    void on_spinBox_TemplateAngle_valueChanged(double arg1);

private:
    void initWindowWidgetsValue();
    void displayImageToLabel(QLabel *label, cv::Mat image);
    void setWidgetsCannyRangeValue(int value);
    void edgeRangeValueChange(int value);
    void setWidgetsCannyContrastValue(double value);
    void contrastValueChange(double value);
    void updatePickPositionSpinBox();

    Ui::PatternDialog *ui;
    ImageMatch::GeoModel modelTemp;
};

#endif // PATTERNDIALOG_H
