#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // initialize ui language
    ui_Language_Init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // check user really want to close application or not
    // show message box with "Yes" and "Cancel" button, focus on Cancel button
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, this->windowTitle(),
                                                               tr("Are you sure?\n"),
                                                               QMessageBox::Yes | QMessageBox::No,
                                                               QMessageBox::No);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
        return;
    }

    // accept event (accept close application)
    event->accept();
}

void MainWindow::ui_Language_Init() {
    // normal UI language is English
    currentLanguage = LanguagesUI::EN;

    connect(ui->actionEnglish, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::EN);
    });
    connect(ui->actionJapanese, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::JP);
    });
    connect(ui->actionVietnamese, &QAction::triggered, this, [this]() {
        ui_Language_Load(LanguagesUI::VN);
    });

    this->setWindowTitle("Han's E05 pick and place control application");
}

void MainWindow::ui_Language_Load(const LanguagesUI lang) {
    if(currentLanguage == lang) {
        return;
    }

    QString translatorPath = QApplication::applicationDirPath();
    bool loadState = false;

    switch (lang) {
    case LanguagesUI::VN:
        translatorPath.append("/E05-PnP_vi_VN.qm");
        loadState = uiTranslator.load(translatorPath);
        qApp->installTranslator(&uiTranslator);
        break;
    case LanguagesUI::JP:
        translatorPath.append("/E05-PnP_ja_JP.qm");
        loadState = uiTranslator.load(translatorPath);
        qApp->installTranslator(&uiTranslator);
        break;
    case LanguagesUI::EN:
        loadState = qApp->removeTranslator(&uiTranslator);
        break;
    }

    if(loadState) {
        ui->retranslateUi(this);
        currentLanguage = lang;
    }
}

