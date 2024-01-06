#ifndef FXWIDGET_H
#define FXWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>

#include "dialog/InputFormDialog.h"

#include "plcdevice.h"
#include "fxremote.h"

namespace Ui {
class FxWidget;
}

using namespace fx;

class FxWidget : public QWidget
{
  Q_OBJECT

public:
  struct PlcDeviceRow
  {
    QTableWidgetItem *item_name;
    QTableWidgetItem *item_type;
    QTableWidgetItem *item_value;

    PlcDeviceRow() {
      item_name = new QTableWidgetItem();
      item_type = new QTableWidgetItem();
      item_value = new QTableWidgetItem();
    }

    PlcDeviceRow(QString name, QString type, QString value) {
      item_name = new QTableWidgetItem(name);
      item_type = new QTableWidgetItem(type);
      item_value = new QTableWidgetItem(value);
    }
  };

  explicit FxWidget(QWidget *parent = nullptr);
  ~FxWidget();

  void FxUpdateViewTable(FxPlc::DeviceMap device_map);
  void FxSetPlcInstance(FxRemote* const& instance);

private:
  void InitAction();
  void InitTable();
  void InitCombobox();

  void OnButtonClick_Plc_Connect();
  void OnButtonClick_Remote_Run();
  void OnButtonClick_Remote_Stop();
  void OnButtonClick_Add_Device();
  void OnButtonClick_Delete_All();
  void OnButtonClick_Delete_Row();
  void OnButtonClick_Bool_On();
  void OnButtonClick_Bool_Off();
  void OnButtonClick_Number_Modify();
  void OnCheckBoxReleased_Add_Multi();
  void OnTableWidgetItemSelectChanged_DeviceList();

  bool UserInputAddress();
  void PlcConnectedHandle();
  void PlcDisconnectedHandle();
  void PlcConnectFailHandle();

  void ReFreshViewList();
  void ReFreshTableValue();
  void ReFreshOperationMode(bool is_run);
  void ReFreshErrorStatus(bool is_error);
  void ClearAllRow();
  void InsertNewRow(int index, QString name, QString type, QString value);
  void DisableSelectWidget();

private:
  Ui::FxWidget *ui;
  FxPlc::DeviceMap device_map_;
  QMap<QString, int> show_data_map_;
  QList<PlcDeviceRow> show_data_;
  FxRemote *plc_ = nullptr;
  QString plc_address_;
  int plc_server_port_;
};

#endif // FXWIDGET_H
