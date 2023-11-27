#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTranslator>
#include <QMessageBox>

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
    // MAINWINDOW EVENT HANDLE FUNCTIONS
    void closeEvent(QCloseEvent *event) override;

    // UI LANGUAGE HANDLE FUNCTIONS
    void ui_Language_Init();
    void ui_Language_Load(const LanguagesUI lang);

private:
    Ui::MainWindow *ui;
    // LANGUAGE
    QTranslator uiTranslator;
    LanguagesUI currentLanguage;

    const QString string_mainTitle = "Han's E05 pick and place control application";
};
#endif // MAINWINDOW_H
