#include "fxwidget.h"
#include "ui_fxwidget.h"

FxWidget::FxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FxWidget)
{
  ui->setupUi(this);

  InitTable();
  InitCombobox();
  InitAction();
}

FxWidget::~FxWidget()
{
  delete ui;
}

void FxWidget::FxUpdateViewTable(FxPlc::DeviceMap device_map) {
  device_map_ = device_map;
  ReFreshTableValue();
}

void FxWidget::FxSetPlcInstance(FxRemote* const& instance) {
  if (instance == nullptr) {
    return;
  }

  plc_ = instance;

  connect(plc_, &FxRemote::FxSignal_Connected,
          this, &FxWidget::PlcConnectedHandle);
  connect(plc_, &FxRemote::FxSignal_Disconnected,
          this, &FxWidget::PlcDisconnectedHandle);
  connect(plc_, &FxRemote::FxSignal_ConnectFail,
          this, &FxWidget::PlcDisconnectedHandle);
  connect(plc_, &FxRemote::FxSignal_PollingQueryStriggered,
          this, &FxWidget::FxUpdateViewTable);
}

void FxWidget::InitAction() {
  // button
  connect(ui->btn_plc_connect, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Plc_Connect);
  connect(ui->btn_remote_run, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Remote_Run);
  connect(ui->btn_remote_stop, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Remote_Stop);
  connect(ui->btn_add_device, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Add_Device);
  connect(ui->btn_delete_all, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Delete_All);
  connect(ui->btn_delete_row, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Delete_Row);
  connect(ui->btn_bool_off, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Bool_Off);
  connect(ui->btn_bool_on, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Bool_On);
  connect(ui->btn_number_set, &QPushButton::clicked,
          this, &FxWidget::OnButtonClick_Number_Modify);
  // check box
  connect(ui->checkBox_add_multi, &QCheckBox::released,
          this, &FxWidget::OnCheckBoxReleased_Add_Multi);
  // select table items
  connect(ui->tableWidget, &QTableWidget::itemSelectionChanged,
          this, &FxWidget::OnTableWidgetItemSelectChanged_DeviceList);
  // disable initial widget
  DisableSelectWidget();
  OnCheckBoxReleased_Add_Multi();
}

void FxWidget::InitTable() {
  QStringList header_list;
  header_list.push_back("Device name");
  header_list.push_back("Type");
  header_list.push_back("Value");
  ui->tableWidget->setColumnCount(header_list.size());
  ui->tableWidget->setHorizontalHeaderLabels(header_list);
  ui->tableWidget->setColumnWidth(0, 180);
  int minimum_height = ui->tableWidget->columnWidth(0);
  minimum_height += ui->tableWidget->columnWidth(1);
  minimum_height += ui->tableWidget->columnWidth(2);
  minimum_height += 20;
  ui->tableWidget->setMinimumWidth(minimum_height);
}

void FxWidget::InitCombobox() {
  QStringList device_name_list;
//  device_name_list.push_back("X");
  device_name_list.push_back("Y");
  device_name_list.push_back("M");
  device_name_list.push_back("D");
  ui->comboBox_device_name->addItems(device_name_list);
}

void FxWidget::OnButtonClick_Add_Device() {
  FxPlc::DeviceName device_name = FxPlc::QStringToEnum(
      ui->comboBox_device_name->currentText());

  if (ui->checkBox_add_multi->isChecked()) {
    for (int index=0;index<ui->spinBox_multi->value();index++) {
      device_map_.Add(device_name, ui->spinBox_start_number->value() + index);
    }
  } else {
    device_map_.Add(device_name, ui->spinBox_start_number->value());
  }

  ReFreshViewList();
}

void FxWidget::OnButtonClick_Plc_Connect() {
  if (plc_->FxIsConnected()) {
    plc_->FxDisconnectFromPlc();
  } else {
    if (UserInputAddress())
    plc_->FxConnectToPlc(plc_address_, plc_server_port_);
  }
}

void FxWidget::OnButtonClick_Remote_Run() {
  plc_->FxSetPlcRunMode();
}

void FxWidget::OnButtonClick_Remote_Stop() {
  plc_->FxSetPlcStopMode();
}

void FxWidget::OnButtonClick_Delete_All() {
  device_map_.ClearAll();
  ClearAllRow();
  ReFreshViewList();
}

void FxWidget::OnButtonClick_Delete_Row() {  
  int delete_index = ui->tableWidget->currentRow();
  if (delete_index < 0) {
    return;
  }
  QString device_name = ui->tableWidget->item(delete_index, 0)->text();
  fx::FxPlc::DeviceAddress deivce;
  deivce.FromString(device_name);
  device_map_.Remove(deivce.name, deivce.number);
  ReFreshViewList();
  DisableSelectWidget();
}

void FxWidget::OnButtonClick_Bool_On() {
  if (plc_ == nullptr) {
    return;
  }

  int current_index = ui->tableWidget->currentRow();
  if (current_index < 0) {
    DisableSelectWidget();
    return;
  }
  QString device_str = ui->tableWidget->item(current_index, 0)->text();
  fx::FxPlc::DeviceAddress deivce;
  deivce.FromString(device_str);
  plc_->FxWriteBit(deivce.name, deivce.number, true);
}

void FxWidget::OnButtonClick_Bool_Off() {
  if (plc_ == nullptr) {
    return;
  }

  int current_index = ui->tableWidget->currentRow();
  if (current_index < 0) {
    DisableSelectWidget();
    return;
  }
  QString device_str = ui->tableWidget->item(current_index, 0)->text();
  fx::FxPlc::DeviceAddress deivce;
  deivce.FromString(device_str);
  plc_->FxWriteBit(deivce.name, deivce.number, false);
}

void FxWidget::OnButtonClick_Number_Modify() {
  if (plc_ == nullptr) {
    return;
  }

  int current_index = ui->tableWidget->currentRow();
  if (current_index < 0) {
    DisableSelectWidget();
    return;
  }
  QString device_str = ui->tableWidget->item(current_index, 0)->text();
  fx::FxPlc::DeviceAddress deivce;
  deivce.FromString(device_str);
  plc_->FxWriteWord(deivce.name, deivce.number, ui->spinBox_number_modify->value());
}

void FxWidget::OnCheckBoxReleased_Add_Multi() {
  bool enable_state = ui->checkBox_add_multi->isChecked();
  ui->label_device_number->setEnabled(enable_state);
  ui->spinBox_multi->setEnabled(enable_state);
}

void FxWidget::OnTableWidgetItemSelectChanged_DeviceList() {
  if (!ui->btn_delete_row->isEnabled()) {
    ui->btn_delete_row->setEnabled(true);
  }

  QString var_type = ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text();
  if (var_type == "Bool") {
    ui->btn_bool_off->setEnabled(true);
    ui->btn_bool_on->setEnabled(true);
    ui->btn_number_set->setEnabled(false);
    ui->spinBox_number_modify->setEnabled(false);
  } else if (var_type == "Number") {
    ui->btn_bool_off->setEnabled(false);
    ui->btn_bool_on->setEnabled(false);
    ui->btn_number_set->setEnabled(true);
    ui->spinBox_number_modify->setEnabled(true);
  }
}

bool FxWidget::UserInputAddress() {
  // setup input dialog form
  InputFormDialog::FormData data;
  data["PLC Address"] = "192.168.1.250";
  data["Server port"] = 10000;
  // limits server port number
  InputFormDialog::FormOptions options;
  options.numericMin = 0;
  options.numericMax = 65535;

  while(true) {
    if(InputFormDialog::getInput("Enter PLC Address", data, options)) {
      QHostAddress h_address(data.at<QString>("PLC Address"));
      if(h_address.protocol() == QAbstractSocket::IPv4Protocol) {
        plc_address_ = data.at<QString>("PLC Address");
        plc_server_port_ = data.at<int>("Server port");
        return true;
      }
    } else {
      return false;
    }
  }
}

void FxWidget::PlcConnectedHandle() {
  ui->btn_plc_connect->setText("Disconnect");
  ui->label_plc_connect_status->setText("PLC connected");
  ui->label_plc_ip_address->setText(plc_address_);
  ui->label_plc_server_port->setText(QString::number(plc_server_port_,10));
//  ui->label_plc_model_name->setText(plc_->FxPlcModelName());
}

void FxWidget::PlcDisconnectedHandle() {
  ui->btn_plc_connect->setText("Connect");
  ui->label_plc_connect_status->setText("No connection");
  ui->label_plc_ip_address->setText("*");
  ui->label_plc_server_port->setText("*");
  ui->label_plc_model_name->setText("");
}

void FxWidget::PlcConnectFailHandle() {
  QMessageBox::information(this, "PLC connect fail",
                           "PLC Connect fail", QMessageBox::Ok);
}

void FxWidget::ReFreshViewList() {
  ClearAllRow();
  show_data_map_.clear();
  show_data_.clear();
  int new_row_index = 0;
  if (!device_map_.y_map.isEmpty()) {
    for (int index=0;index<device_map_.y_map.count();index++) {
      QList<int> num_list = device_map_.y_map.keys();
      QString name = "Y" + QString::number(num_list[index]);
      QString value = (device_map_.y_map[num_list[index]]) ? "True" : "False";
      show_data_map_[name] = new_row_index++;
      PlcDeviceRow new_row(name, "Bool", value);
      show_data_.push_back(new_row);
    }
  }

  if (!device_map_.m_map.isEmpty()) {
    for (int index=0;index<device_map_.m_map.count();index++) {
      QList<int> num_list = device_map_.m_map.keys();
      QString name = "M" + QString::number(num_list[index]);
      QString value = (device_map_.m_map[num_list[index]]) ? "True" : "False";
      show_data_map_[name] = new_row_index++;
      PlcDeviceRow new_row(name, "Bool", value);
      show_data_.push_back(new_row);
    }
  }


  if (!device_map_.d_map.isEmpty()) {
    for (int index=0;index<device_map_.d_map.count();index++) {
      QList<int> num_list = device_map_.d_map.keys();
      QString name = "D" + QString::number(num_list[index]);
      int16_t number = static_cast<int16_t>(device_map_.d_map[num_list[index]]);
      QString value = QString::number(number);
      show_data_map_[name] = new_row_index++;
      PlcDeviceRow new_row(name, "Number", value);
      show_data_.push_back(new_row);
    }
  }

  for (int index=0;index<show_data_.count();index++) {
    ui->tableWidget->insertRow(index);
    PlcDeviceRow insert_row = show_data_[index];
    ui->tableWidget->setItem(index, 0, insert_row.item_name);
    ui->tableWidget->setItem(index, 1, insert_row.item_type);
    ui->tableWidget->setItem(index, 2, insert_row.item_value);
  }

  plc_->FxSetDeviceMap(device_map_);
}

void FxWidget::ReFreshTableValue() {
  if (plc_ == nullptr) {
    return;
  }

  if (ui->label_plc_model_name->text().isEmpty()) {
    ui->label_plc_model_name->setText(plc_->FxPlcModelName());
  }

  ReFreshOperationMode(plc_->FxIsPlcRunMode());
  ReFreshErrorStatus(plc_->FxIsPlcErrorOccurred());

  if (!device_map_.y_map.isEmpty()) {
    for (int index=0;index<device_map_.y_map.count();index++) {
      QList<int> num_list = device_map_.y_map.keys();
      QString name = "Y" + QString::number(num_list[index]);
      QString value = (device_map_.y_map[num_list[index]]) ? "True" : "False";
      int index_in_map = show_data_map_[name];
      show_data_[index_in_map].item_value->setText(value);
    }
  }

  if (!device_map_.m_map.isEmpty()) {
    for (int index=0;index<device_map_.m_map.count();index++) {
      QList<int> num_list = device_map_.m_map.keys();
      QString name = "M" + QString::number(num_list[index]);
      QString value = (device_map_.m_map[num_list[index]]) ? "True" : "False";
      int index_in_map = show_data_map_[name];
      show_data_[index_in_map].item_value->setText(value);
    }
  }


  if (!device_map_.d_map.isEmpty()) {
    for (int index=0;index<device_map_.d_map.count();index++) {
      QList<int> num_list = device_map_.d_map.keys();
      QString name = "D" + QString::number(num_list[index]);
      int16_t number = static_cast<int16_t>(device_map_.d_map[num_list[index]]);
      QString value = QString::number(number);
      int index_in_map = show_data_map_[name];
      show_data_[index_in_map].item_value->setText(value);
    }
  }
}

void FxWidget::ReFreshOperationMode(bool is_run) {
  QString status = "RUN";
  if (!is_run) {
    status = "STOP";
  }
  ui->label_plc_operation->setText(status);
}

void FxWidget::ReFreshErrorStatus(bool is_error) {
  QString status = "NO ERROR";
  if (is_error) {
    status = "ERROR OCCURRED";
  }
  ui->label_error_occurred->setText(status);
}

void FxWidget::ClearAllRow() {
  ui->tableWidget->reset();
  ui->tableWidget->setRowCount(0);
}

void FxWidget::InsertNewRow(int index, QString name, QString type, QString value) {
  QTableWidgetItem *item_name = new QTableWidgetItem(name);
  QTableWidgetItem *item_type = new QTableWidgetItem(type);
  QTableWidgetItem *item_value = new QTableWidgetItem(value);
  ui->tableWidget->insertRow(index);
  ui->tableWidget->setItem(index, 0, item_name);
  ui->tableWidget->setItem(index, 1, item_type);
  ui->tableWidget->setItem(index, 2, item_value);
}

void FxWidget::DisableSelectWidget() {
  ui->btn_bool_off->setEnabled(false);
  ui->btn_bool_on->setEnabled(false);
  ui->btn_number_set->setEnabled(false);
  ui->spinBox_number_modify->setEnabled(false);
}
