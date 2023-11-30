#include "RobotDashboard.h"
#include "ui_RobotDashboard.h"

RobotDashboard::RobotDashboard(QWidget *parent, rb::HansClient* const& hans) :
    QWidget(parent),
    ui(new Ui::RobotDashboard)
{
    ui->setupUi(this);
    robot = hans;
}

RobotDashboard::~RobotDashboard()
{
    delete ui;
}

void RobotDashboard::InitDashboardAction() {

}
