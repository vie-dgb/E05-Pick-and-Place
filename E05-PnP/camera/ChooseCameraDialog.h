#ifndef CHOOSECAMERADIALOG_H
#define CHOOSECAMERADIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <pylon/PylonIncludes.h>
#include <QMessageBox>

#define CHOOSE_CAM_DIALOG_TITLE     "Choose GigE Camera"

namespace Ui {
class ChooseCameraDialog;
}

class ChooseCameraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseCameraDialog(QWidget *parent = nullptr);
    ~ChooseCameraDialog();

    void showSelectedCameraDialog();
    bool isUserSelectedCamera();
    Pylon::CDeviceInfo getSelectCameraInfo();

signals:
    void signal_userAccepted(Pylon::CDeviceInfo device);
    void signal_userRejected();

private slots:
    void on_buttonBox_Accept_accepted();
    void on_buttonBox_Accept_rejected();
    void on_btn_Refresh_clicked();

private:
    void updateCameraTableView();
    void clearViewTable();
    void cameraViewAddNewRow(Pylon::CDeviceInfo info);
    bool isDeviceAccessible(Pylon::CDeviceInfo info);

    Ui::ChooseCameraDialog *ui;
    QStandardItemModel *model;

    bool userSelectedCamera;
    Pylon::DeviceInfoList cameraDeviceList;
    Pylon::CDeviceInfo selectedCamera;
    int selectedCameraIndex;
};

#endif // CHOOSECAMERADIALOG_H
