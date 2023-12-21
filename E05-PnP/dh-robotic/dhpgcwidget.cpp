#include "dhpgcwidget.h"
#include "ui_dhpgcwidget.h"

DhPgcWidget::DhPgcWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DhPgcWidget)
{
  ui->setupUi(this);
  InitActions();
}

DhPgcWidget::~DhPgcWidget()
{
  delete ui;
}

void DhPgcWidget::ShowPgcDeviceSetParameters(PGCData::PGCFeedback feedback_data) {
  ui->doubleSpinBox_gipper_position->setValue(feedback_data.gripper_position/10.0);
  ui->spinBox_gripper_force->setValue(100);
  ui->spinBox_gripper_speed->setValue(100);
}

void DhPgcWidget::ShowPgcDeviceInfo(PGCData::PGCFeedback feedback_data) {
  // gripper info
  ui->label_gripper_init_state->setText(
      "Initialize state: " + dhr::EnumConvert(feedback_data.gripper_init_state));
  ui->label_gripper_status->setText(
      "Status: " + dhr::EnumConvert(feedback_data.gripper_status));
  ui->label_gripper_position->setText(
      "Current position: " + QString::number(feedback_data.gripper_position / 10.0));
}

bool DhPgcWidget::IsAutoInit() {
  return ui->checkBox_auto_initialize->isChecked();
}

int DhPgcWidget::GetSlaveAddress() {
  return ui->spinBox_rgi_address->value();
}

void DhPgcWidget::SetSlaveEditBoxEnable(bool is_enable) {
  ui->spinBox_rgi_address->setEnabled(is_enable);
}

void DhPgcWidget::InitActions() {
  connect(ui->btn_initialize, &QPushButton::clicked, this, [this] () {
    emit SignalsRgiInitialize();
  });

  connect(ui->doubleSpinBox_gipper_position, &QDoubleSpinBox::editingFinished,
          this, [this] () {
    int value = ui->doubleSpinBox_gipper_position->value() * 10;
    emit SignalsGripper_PositionEdited(value);
  });

  connect(ui->btn_gripper_test_1, &QPushButton::clicked, this, [this] () {
    int value = ui->doubleSpinBox_gipper_test_1->value() * 10;
    emit SignalsGripper_PositionEdited(value);
  });

  connect(ui->btn_gripper_test_2, &QPushButton::clicked, this, [this] () {
    int value = ui->doubleSpinBox_gipper_test_2->value() * 10;
    emit SignalsGripper_PositionEdited(value);
  });

  connect(ui->btn_gripper_test_3, &QPushButton::clicked, this, [this] () {
    int value = ui->doubleSpinBox_gipper_test_3->value() * 10;
    emit SignalsGripper_PositionEdited(value);
  });

  connect(ui->spinBox_gripper_force, &QSpinBox::editingFinished,
          this, [this] () {
    emit SignalsGripper_ForceEdited(ui->spinBox_gripper_force->value());
  });

  connect(ui->spinBox_gripper_speed, &QSpinBox::editingFinished,
          this, [this] () {
    emit SignalsGripper_SpeedEdited(ui->spinBox_gripper_speed->value());
  });


}
