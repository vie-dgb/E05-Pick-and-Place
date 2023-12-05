#include "ChooseCameraDialog.h"
#include "ui_ChooseCameraDialog.h"

ChooseCameraDialog::ChooseCameraDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseCameraDialog)
{
    ui->setupUi(this);

    // init dialog windows
    setWindowTitle(CHOOSE_CAM_DIALOG_TITLE);

    // init tableWidget_CamList
    ui->tableWidget_CamList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget_CamList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
}

ChooseCameraDialog::~ChooseCameraDialog()
{
    delete ui;
}

// user apply change
void ChooseCameraDialog::on_buttonBox_Accept_accepted()
{
    selectedCameraIndex = ui->tableWidget_CamList->currentRow();
    userSelectedCamera = true;
    selectedCamera = cameraDeviceList[selectedCameraIndex];
    if(isDeviceAccessible(selectedCamera)) {
        emit signal_userAccepted(selectedCamera);
        close();
    }
    else {
        QMessageBox errorMsg;
        errorMsg.critical(this, "Connect error", "Device in use by another application");
    }
}

// user reject change
void ChooseCameraDialog::on_buttonBox_Accept_rejected()
{
    selectedCameraIndex = -1;
    userSelectedCamera = false;
    emit signal_userRejected();
    close();
}

void ChooseCameraDialog::showSelectedCameraDialog() {
    updateCameraTableView();
    setModal(true);
    show();
}

bool ChooseCameraDialog::isUserSelectedCamera() {
    return userSelectedCamera;
}

Pylon::CDeviceInfo ChooseCameraDialog::getSelectCameraInfo() {
    return selectedCamera;
}

void ChooseCameraDialog::on_btn_Refresh_clicked()
{
    updateCameraTableView();
}

void ChooseCameraDialog::updateCameraTableView() {
    clearViewTable();
    Pylon::CTlFactory::GetInstance().EnumerateDevices(cameraDeviceList, false);
    if(cameraDeviceList.size() > 0 ) {
        for(int addCounter = 0; addCounter < cameraDeviceList.size(); addCounter++) {
            cameraViewAddNewRow(cameraDeviceList[addCounter]);
        }
    }
}

void ChooseCameraDialog::clearViewTable() {
    int removeRow = ui->tableWidget_CamList->rowCount();
    for(int removeCounter=0;removeCounter<removeRow;removeCounter++) {
        ui->tableWidget_CamList->removeRow(0);
    }
}

void ChooseCameraDialog::cameraViewAddNewRow(Pylon::CDeviceInfo info) {
    QTableWidgetItem *nameItem = new QTableWidgetItem();
    QTableWidgetItem *userIDItem = new QTableWidgetItem();
    QTableWidgetItem *serialNumberItem = new QTableWidgetItem();
    QTableWidgetItem *macAddrItem = new QTableWidgetItem();
    QTableWidgetItem *statusItem = new QTableWidgetItem();
    QTableWidgetItem *ipConfigItem = new QTableWidgetItem();
    QTableWidgetItem *ipAddrItem = new QTableWidgetItem();
    QTableWidgetItem *subnetMarkItem = new QTableWidgetItem();
    // add new row
    int row = ui->tableWidget_CamList->rowCount();
    ui->tableWidget_CamList->insertRow(row);
    // add item
    // header: Name | Device User ID | Serial Number | MAC Address | Status | IP Configuration | IP Address | Subnet mark

    // Name
    nameItem->setText(QString::fromUtf8(info.GetModelName().c_str()));
    ui->tableWidget_CamList->setItem(row, 0, nameItem);

    // Device User ID
    userIDItem->setText(QString::fromUtf8(info.GetUserDefinedName().c_str()));
    ui->tableWidget_CamList->setItem(row, 1, userIDItem);

    // Serial Number
    serialNumberItem->setText(QString::fromUtf8(info.GetSerialNumber().c_str()));
    ui->tableWidget_CamList->setItem(row, 2, serialNumberItem);

    // MAC Address
    macAddrItem->setText(QString::fromUtf8(info.GetMacAddress().c_str()));
    ui->tableWidget_CamList->setItem(row, 3, macAddrItem);

    // Status, check device can accessible or not
    if(isDeviceAccessible(info)) {
        statusItem->setText("Ok");
    }
    else {
        statusItem->setText("In use");
    }
    ui->tableWidget_CamList->setItem(row, 4, statusItem);

    // IP Configuration
    ipConfigItem->setText(QString::fromUtf8(info.GetIpConfigCurrent().c_str()));
    ui->tableWidget_CamList->setItem(row, 5, ipConfigItem);

    // IP Address
    ipAddrItem->setText(QString::fromUtf8(info.GetIpAddress().c_str()));
    ui->tableWidget_CamList->setItem(row, 6, ipAddrItem);

    // Subnet mark
    subnetMarkItem->setText(QString::fromUtf8(info.GetSubnetMask().c_str()));
    ui->tableWidget_CamList->setItem(row, 7, subnetMarkItem);
}

bool ChooseCameraDialog::isDeviceAccessible(Pylon::CDeviceInfo info) {
    return Pylon::CTlFactory::GetInstance().IsDeviceAccessible(info);
}
