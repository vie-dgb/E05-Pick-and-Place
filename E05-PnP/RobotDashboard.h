#ifndef ROBOTDASHBOARD_H
#define ROBOTDASHBOARD_H

#include <QWidget>

#include "robot/HansClient.h"

namespace Ui {
class RobotDashboard;
}

class RobotDashboard : public QWidget
{
    Q_OBJECT
public:
    explicit RobotDashboard(QWidget *parent = nullptr, rb::HansClient* const& hans = nullptr);
    ~RobotDashboard();

private:
    void InitDashboardAction();

private:
    Ui::RobotDashboard *ui;

    rb::HansClient *robot;
};

#endif // ROBOTDASHBOARD_H
