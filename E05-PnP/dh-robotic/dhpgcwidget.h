#ifndef DHPGCWIDGET_H
#define DHPGCWIDGET_H

#include <QWidget>
#include "dhr_define.h"
#include "dh_pgc.h"

namespace Ui {
class DhPgcWidget;
}

using namespace dhr;

class DhPgcWidget : public QWidget
{
  Q_OBJECT

public:
  explicit DhPgcWidget(QWidget *parent = nullptr);
  ~DhPgcWidget();

  void ShowPgcDeviceSetParameters(PGCData::PGCFeedback feedback_data);
  void ShowPgcDeviceInfo(PGCData::PGCFeedback feedback_data);
  bool IsAutoInit();
  int GetSlaveAddress();
  void SetSlaveEditBoxEnable(bool is_enable);

private:
  void InitActions();

signals:
  void SignalsRgiInitialize();
  void SignalsGripper_PositionEdited(int position);
  void SignalsGripper_ForceEdited(int force);
  void SignalsGripper_SpeedEdited(int speed);

private:
  Ui::DhPgcWidget *ui;
};

#endif // DHPGCWIDGET_H
