#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTranslator>
#include <QMessageBox>

#include "dialog/InputFormDialog.h"
#include "robot/HansClient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum LanguagesUI {
        VN = 1,
        EN,
        JP
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    /// MAINWINDOW EVENT HANDLE FUNCTIONS
    void closeEvent(QCloseEvent *event) override;

    /// UI LANGUAGE HANDLE FUNCTIONS
    void ui_Language_Init();
    void ui_Language_Load(const LanguagesUI lang);

    /// ROBOT UI FUNTIONS
    void robot_UiInitialize();
    bool robot_UserInputAddress();

    /// BUTTON ACTIONS
    void on_click_robot_connect();
    void on_click_robot_control();

private:
    Ui::MainWindow *ui;
    // LANGUAGE
    QTranslator uiTranslator;
    LanguagesUI currentLanguage;

    rb::HansClient *hansRobot;
    QString robotAddress;

    // UI lable text
    const QString lb_robot_Connect = "Connect";
    const QString lb_robot_Disconnect = "Disconnect";
    const QString lb_robot_Connected = "E05 Connected";
    const QString lb_robot_Disconnected = "E05 No Connection";
    const QString lb_robot_WaitConnect = "E05 Connecting";
    const QString lb_robot_dialog_Address = "Ip address";
    const QString lb_robot_dialog_ConnectFail = "E05 connect fail. \nPlease check robot address.";
    const QString lb_robot_dialog_LostConnect = "E05 lost connect. \nPlease restart the robot's power.";
    const QString lb_robot_dialog_ConnectInfo = "E05 connection";
};
#endif // MAINWINDOW_H
