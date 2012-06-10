#include "qgcautoquad.h"
#include "ui_qgcautoquad.h"
#include "LinkManager.h"
#include "UASManager.h"
#include <SerialLinkInterface.h>
#include <SerialLink.h>
#include <qstringlist.h>
#include <configuration.h>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QTextBrowser>
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QSvgGenerator>

QGCAutoquad::QGCAutoquad(QWidget *parent) :
    QWidget(parent),
    currLink(NULL),
    plot(new IncrementalPlot()),
    uas(NULL),
    paramaq(NULL),
    esc32(NULL),
    ui(new Ui::QGCAutoquad)
{
    ui->setupUi(this);
    esc32 = NULL;
    model = NULL;
    QHBoxLayout* layout = new QHBoxLayout(ui->plotFrame);
    layout->addWidget(plot);
    ui->plotFrame->setLayout(layout);

    //setup ListView curves
    //SetupListView();

	//GUI slots
	connect(ui->SelectFirmwareButton, SIGNAL(clicked()), this, SLOT(selectFWToFlash()));
	connect(ui->portName, SIGNAL(editTextChanged(QString)), this, SLOT(setPortName(QString)));
    connect(ui->portName, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortName(QString)));

    connect(ui->comboBox_port_esc32, SIGNAL(editTextChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->comboBox_port_esc32, SIGNAL(currentIndexChanged(QString)), this, SLOT(setPortNameEsc32(QString)));
    connect(ui->pushButton_connect_to_esc32, SIGNAL(clicked()), this, SLOT(btnConnectEsc32()));
    connect(ui->pushButton_read_config, SIGNAL(clicked()),this, SLOT(btnReadConfigEsc32()));
    connect(ui->pushButton_send_to_esc32, SIGNAL(clicked()),this,SLOT(btnSaveToEsc32()));
    connect(ui->pushButton_esc32_read_arm_disarm, SIGNAL(clicked()),this,SLOT(btnArmEsc32()));
    connect(ui->pushButton_esc32_read_start_stop, SIGNAL(clicked()),this,SLOT(btnStartStopEsc32()));
    connect(ui->pushButton_send_rpm, SIGNAL(clicked()),this,SLOT(btnSetRPM()));
    connect(ui->horizontalSlider_rpm, SIGNAL(valueChanged(int)),this,SLOT(Esc32RpmSlider(int)));

    //pushButton_send_rpm

	connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(flashFW()));
    connect(ui->pushButton_Add_Static, SIGNAL(clicked()),this,SLOT(addStatic()));
    connect(ui->pushButton_Remov_Static, SIGNAL(clicked()),this,SLOT(delStatic()));
    connect(ui->pushButton_Add_Dynamic, SIGNAL(clicked()),this,SLOT(addDynamic()));
    connect(ui->pushButton_Remove_Dynamic, SIGNAL(clicked()),this,SLOT(delDynamic()));
    connect(ui->pushButton_save_to_aq_radio, SIGNAL(clicked()),this,SLOT(setRadio()));
    connect(ui->pushButton_save_to_aq_frame, SIGNAL(clicked()),this,SLOT(setFrame()));
    connect(ui->pushButton_Sel_params_file_user, SIGNAL(clicked()),this,SLOT(setUsersParams()));
    connect(ui->pushButton_Create_params_file_user, SIGNAL(clicked()),this,SLOT(CreateUsersParams()));
    connect(ui->pushButton_save, SIGNAL(clicked()),this,SLOT(WriteUsersParams()));
    connect(ui->pushButton_Load_frame_from_file, SIGNAL(clicked()),this,SLOT(LoadFrameFromFile()));
    connect(ui->pushButton_Save_farem_to_file, SIGNAL(clicked()),this,SLOT(SaveFrameToFile()));
    connect(ui->pushButton_Calculate, SIGNAL(clicked()),this,SLOT(CalculatDeclination()));
    connect(ui->pushButton_Open_Log_file, SIGNAL(clicked()),this,SLOT(OpenLogFile()));
    connect(ui->pushButton_set_marker, SIGNAL(clicked()),this,SLOT(startSetMarker()));

    connect(ui->pushButton_save_to_aq_pid1, SIGNAL(clicked()),this,SLOT(save_PID_toAQ1()));
    connect(ui->pushButton_save_to_aq_pid2, SIGNAL(clicked()),this,SLOT(save_PID_toAQ2()));
    connect(ui->pushButton_save_to_aq_pid3, SIGNAL(clicked()),this,SLOT(save_PID_toAQ3()));
    connect(ui->pushButton_save_image_plot, SIGNAL(clicked()),this,SLOT(save_plot_image()));
    connect(ui->pushButtonshow_cahnnels, SIGNAL(clicked()),this,SLOT(showChannels()));


    //pushButton_send_to_esc32
    //Slots for Calibration
    connect(ui->pushButton_start_cal1, SIGNAL(clicked()),this,SLOT(startcal1()));
    connect(ui->pushButton_start_cal2, SIGNAL(clicked()),this,SLOT(startcal2()));
    connect(ui->pushButton_start_cal3, SIGNAL(clicked()),this,SLOT(startcal3()));
    connect(ui->pushButton_start_sim1, SIGNAL(clicked()),this,SLOT(startsim1()));
    connect(ui->pushButton_start_sim1_2, SIGNAL(clicked()),this,SLOT(startsim1b()));
    connect(ui->pushButton_start_sim2, SIGNAL(clicked()),this,SLOT(startsim2()));
    connect(ui->pushButton_start_sim3, SIGNAL(clicked()),this,SLOT(startsim3()));
    connect(ui->pushButton_abort_cal1, SIGNAL(clicked()),this,SLOT(abortcal1()));
    connect(ui->pushButton_abort_cal2, SIGNAL(clicked()),this,SLOT(abortcal2()));
    connect(ui->pushButton_abort_cal3, SIGNAL(clicked()),this,SLOT(abortcal3()));
    connect(ui->pushButton_abort_sim1, SIGNAL(clicked()),this,SLOT(abortsim1()));
    connect(ui->pushButton_abort_sim1_2, SIGNAL(clicked()),this,SLOT(abortsim1b()));
    connect(ui->pushButton_abort_sim2, SIGNAL(clicked()),this,SLOT(abortsim2()));
    connect(ui->pushButton_abort_sim3, SIGNAL(clicked()),this,SLOT(abortsim3()));
    connect(ui->checkBox_sim3_4_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_4_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_5_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_5_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_sim3_6_var, SIGNAL(clicked()),this,SLOT(check_var()));
    connect(ui->checkBox_sim3_6_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
    connect(ui->checkBox_raw_value, SIGNAL(clicked()),this,SLOT(raw_transmitter_view()));



	//Process Slots
    ps_master.setProcessChannelMode(QProcess::MergedChannels);
    connect(&ps_master, SIGNAL(finished(int)), this, SLOT(prtstexit(int)));
    connect(&ps_master, SIGNAL(readyReadStandardOutput()), this, SLOT(prtstdout()));
    connect(&ps_master, SIGNAL(readyReadStandardError()), this, SLOT(prtstderr()));


	// UAS slots
	connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(addUAS(UASInterface*)), Qt::UniqueConnection);
    QList<UASInterface*> mavs = UASManager::instance()->getUASList();
    foreach (UASInterface* currMav, mavs) {
        addUAS(currMav);
    }
    setActiveUAS(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(addUAS(UASInterface*)), Qt::UniqueConnection);
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)), Qt::UniqueConnection);

    VisibleWidget = 0;
    ui->comboBox_Radio_Type->addItem("Spektrum 11Bit", 0);
    ui->comboBox_Radio_Type->addItem("Spektrum 10Bit", 1);
    ui->comboBox_Radio_Type->addItem("Futaba", 2);
    ui->comboBox_Radio_Type->addItem("PWM", 3);

    ui->comboBox_marker->addItem("Start & End 1s", 0);
    ui->comboBox_marker->addItem("Start & End 2s", 1);
    ui->comboBox_marker->addItem("Start & End 3s", 2);
    ui->comboBox_marker->addItem("Start & End 5s", 3);
    ui->comboBox_marker->addItem("Start & End 10s", 4);
    ui->comboBox_marker->addItem("Start & End 15s", 5);
    ui->comboBox_marker->addItem("manual", 6);

    setupPortList();
    loadSettings();
}

QGCAutoquad::~QGCAutoquad()
{
    writeSettings();

    delete ui;
}

void QGCAutoquad::SetupListView()
{
    model = new QStandardItemModel(this); //listView_curves
    for ( int i=0; i<parser.LogChannelsStruct.count(); i++ ) {
        QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
        QStandardItem *item = new QStandardItem(val_pair.second.fieldName);
        item->setCheckable(true);
        model->appendRow(item);
    }
    ui->listView_Curves->setModel(model);
    connect(model, SIGNAL(itemChanged(QStandardItem*)), this,SLOT(CurveItemChanged(QStandardItem*)));
}

void QGCAutoquad::OpenLogFile()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ log file (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        QFile file(fileNames.first());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read Log file. Permission denied");
            msgBox.exec();
        }
        LogFile = QDir::toNativeSeparators(file.fileName());
        LastFilePath = LogFile;
        DecodeLogFile(LogFile);
    }
}

void QGCAutoquad::CurveItemChanged(QStandardItem *item)
{

    if ( item->checkState() )
    {
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 1;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }
    else
    {
        for ( int i = 0; i<parser.LogChannelsStruct.count(); i++) {
            QPair<QString,loggerFieldsAndActive_t> val_pair = parser.LogChannelsStruct.at(i);
            if ( val_pair.first == item->text()) {
                val_pair.second.fieldActive = 0;
                parser.LogChannelsStruct.replace(i,val_pair);
                break;
            }
        }
    }

}

void QGCAutoquad::DecodeLogFile(QString fileName)
{
    plot->removeData();
    plot->setStyleText("lines");
    if ( model)
        disconnect(model, SIGNAL(itemChanged(QStandardItem*)), this,SLOT(CurveItemChanged(QStandardItem*)));
    ui->listView_Curves->reset();
    if ( parser.ParseLogHeader(fileName) == 0)
        SetupListView();

}

void QGCAutoquad::showChannels() {
    parser.ShowCurves();
    plot->removeData();
    if (!QFile::exists(LogFile))
        return;

    for (int i = 0; i < parser.yValues.count(); i++) {
        plot->appendData(parser.yValues.keys().at(i), parser.xValues.values().at(0)->data(), parser.yValues.values().at(i)->data(), parser.xValues.values().at(0)->count());
    }
    plot->setStyleText("lines");
    plot->updateScale();
}

void QGCAutoquad::loadSettings()
{
    // Load defaults from settings
    QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

    if (settings.contains("STATIC_FILE_COUNT"))
    {
        qint32 FileStaticCount = settings.value("STATIC_FILE_COUNT").toInt();
        StaticFiles.clear();
        for ( int i =0; i<FileStaticCount; i++) {
            StaticFiles.append(settings.value("STATIC_FILE" + QString::number(i)).toString());
            ui->listWidgetStatic->addItem(settings.value("STATIC_FILE" + QString::number(i)).toString());
        }
    }
    if (settings.contains("DYNAMIC_FILE_COUNT"))
    {
        qint32 FileDynamicCount = settings.value("DYNAMIC_FILE_COUNT").toInt();
        DynamicFiles.clear();
        for ( int i =0; i<FileDynamicCount; i++) {
            DynamicFiles.append(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
            ui->listWidgetDynamic->addItem(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
        }
    }

	ui->lineEdit_insert_declination->setText(settings.value("DECLINATION_SOURCE").toString());
	ui->lineEdit_cal_declination->setText(settings.value("DECLINATION_CALC").toString());

    if (settings.contains("AUTOQUAD_FW_FILE"))
        ui->fileLabel->setText(settings.value("AUTOQUAD_FW_FILE").toString());
    if (settings.contains("USERS_PARAMS_FILE")) {
        UsersParamsFile = settings.value("USERS_PARAMS_FILE").toString();
        if (QFile::exists(UsersParamsFile))
            ShowUsersParams(QDir::toNativeSeparators(UsersParamsFile));
    }

    ui->lineEdit_variance->setText(settings.value("AUTOQUAD_VARIANCE1").toString());
    ui->lineEdit_variance_2->setText(settings.value("AUTOQUAD_VARIANCE2").toString());
    ui->lineEdit_variance_3->setText(settings.value("AUTOQUAD_VARIANCE3").toString());

    ui->lineEdit_stop->setText(settings.value("AUTOQUAD_STOP1").toString());
    ui->lineEdit_stop_2->setText(settings.value("AUTOQUAD_STOP2").toString());
    ui->lineEdit_stop_3->setText(settings.value("AUTOQUAD_STOP3").toString());

    LastFilePath = settings.value("AUTOQUAD_LAST_PATH").toString();
    settings.endGroup();
    settings.sync();
}

void QGCAutoquad::writeSettings()
{
    QSettings settings("Aq.ini", QSettings::IniFormat);
    settings.beginGroup("AUTOQUAD_SETTINGS");

    settings.setValue("STATIC_FILE_COUNT", QString::number(StaticFiles.count()));
    for ( int i = 0; i<StaticFiles.count(); i++) {
        settings.setValue("STATIC_FILE" + QString::number(i), StaticFiles.at(i));
    }
    settings.setValue("DYNAMIC_FILE_COUNT", QString::number(DynamicFiles.count()));
    for ( int i = 0; i<DynamicFiles.count(); i++) {
        settings.setValue("DYNAMIC_FILE" + QString::number(i), DynamicFiles.at(i));
    }

    settings.setValue("DECLINATION_SOURCE", ui->lineEdit_insert_declination->text());
    settings.setValue("DECLINATION_CALC", ui->lineEdit_cal_declination->text());
    settings.setValue("USERS_PARAMS_FILE", UsersParamsFile);

    settings.setValue("AUTOQUAD_FW_FILE", ui->fileLabel->text());

    settings.setValue("AUTOQUAD_VARIANCE1", ui->lineEdit_variance->text());
    settings.setValue("AUTOQUAD_VARIANCE2", ui->lineEdit_variance_2->text());
    settings.setValue("AUTOQUAD_VARIANCE3", ui->lineEdit_variance_3->text());
    settings.setValue("AUTOQUAD_VARIANCE4", ui->lineEdit_variance_4->text());

    settings.setValue("AUTOQUAD_STOP1", ui->lineEdit_stop->text());
    settings.setValue("AUTOQUAD_STOP2", ui->lineEdit_stop_2->text());
    settings.setValue("AUTOQUAD_STOP3", ui->lineEdit_stop_3->text());
    settings.setValue("AUTOQUAD_STOP3", ui->lineEdit_stop_4->text());

    settings.setValue("AUTOQUAD_LAST_PATH", LastFilePath);

    settings.sync();
    settings.endGroup();
}

void QGCAutoquad::addStatic()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Log (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        for ( int i=0; i<fileNames.size(); i++) {
            QString fileNameLocale = QDir::toNativeSeparators(fileNames.at(i));
            ui->listWidgetStatic->addItem(fileNameLocale);
            StaticFiles.append(fileNameLocale);
            LastFilePath = fileNameLocale;
        }
    }
}

void QGCAutoquad::delStatic()
{
    int currIndex = ui->listWidgetStatic->row(ui->listWidgetStatic->currentItem());
    if ( currIndex >= 0) {
        QString SelStaticFile =  ui->listWidgetStatic->item(currIndex)->text();
        StaticFiles.removeAt(StaticFiles.indexOf(SelStaticFile));
        ui->listWidgetStatic->takeItem(currIndex);
    }
}

void QGCAutoquad::addDynamic()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Log (*.LOG)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        for ( int i=0; i<fileNames.size(); i++) {
            QString fileNameLocale = QDir::toNativeSeparators(fileNames.at(i));
            ui->listWidgetDynamic->addItem(fileNameLocale);
            DynamicFiles.append(fileNameLocale);
            LastFilePath = fileNameLocale;
        }
    }

}

void QGCAutoquad::delDynamic()
{
    int currIndex = ui->listWidgetDynamic->row(ui->listWidgetDynamic->currentItem());
    if ( currIndex >= 0) {
        QString SelDynamicFile =  ui->listWidgetDynamic->item(currIndex)->text();
        DynamicFiles.removeAt(DynamicFiles.indexOf(SelDynamicFile));
        ui->listWidgetDynamic->takeItem(currIndex);
    }
}

void QGCAutoquad::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QGCAutoquad::setPortName(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
	portName = port;
	ui->ComPortLabel->setText(portName);
}

void QGCAutoquad::setPortNameEsc32(QString port)
{
#ifdef Q_OS_WIN
    port = port.split("-").first();
#endif
    port = port.remove(" ");
    portNameEsc32 = port;
    ui->label_portName_esc32->setText(portNameEsc32);
}

void QGCAutoquad::btnConnectEsc32()
{
    QString port = ui->label_portName_esc32->text();
    if ( ui->pushButton_connect_to_esc32->text() == "connect esc32") {
        if (esc32 == NULL)
            esc32 = new AQEsc32();
        connect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        connect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        connect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        connect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        connect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        connect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        ui->pushButton_connect_to_esc32->setText("disconnect");
        esc32->Connect(port);
    }
    else {
        disconnect(esc32,SIGNAL(ShowConfig(QString)),this,SLOT(showConfigEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32ParaWritten(QString)),this,SLOT(ParaWrittenEsc32(QString)));
        disconnect(esc32, SIGNAL(Esc32CommandWritten(int,QVariant,QVariant)),this,SLOT(CommandWrittenEsc32(int,QVariant,QVariant)));
        disconnect(esc32, SIGNAL(Esc32Connected()),this,SLOT(Esc32Connected()));
        disconnect(esc32, SIGNAL(ESc32Disconnected()),this,SLOT(ESc32Disconnected()));
        ui->pushButton_connect_to_esc32->setText("connect esc32");
        esc32->Disconnect();
        esc32 = NULL;
    }
}

void QGCAutoquad::showConfigEsc32(QString Config)
{
    paramEsc32.clear();
    QString ConfigStr = Config.remove("\n");
    QStringList RowList = ConfigStr.split("\r");
    for ( int j = 0; j< RowList.length(); j++) {
        QStringList ParaList = RowList.at(j).split(" ", QString::SkipEmptyParts);
        if ( ParaList.length() >= 3)
            paramEsc32.insert(ParaList.at(0),ParaList.at(2));
    }
    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
    for ( int i = 0; i<edtList.count(); i++) {
        edtList.at(i)->setText("");
        QString ParaName = edtList.at(i)->objectName();
        if ( paramEsc32.contains(ParaName) )
        {
            QString value = paramEsc32.value(ParaName);
            edtList.at(i)->setText(value);
        }
    }
}

void QGCAutoquad::btnSaveToEsc32() {

    bool oneWritten = false;
    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
    for ( int i = 0; i<edtList.count(); i++) {
        QString ParaName = edtList.at(i)->objectName();
        if ( paramEsc32.contains(ParaName) )
        {
            QString valueEsc32 = paramEsc32.value(ParaName);
            QString valueText = edtList.at(i)->text();
            if ( valueEsc32 != valueText) {
                esc32->SavePara(ParaName,valueText);
                WaitForParaWriten = 1;
                ParaNameWritten = ParaName;
                oneWritten = true;
                while(WaitForParaWriten >0) {
                    QCoreApplication::processEvents();
                }
            }
        }
    }
    if ( oneWritten )
        saveEEpromEsc32();
}

void QGCAutoquad::btnReadConfigEsc32() {

}

void QGCAutoquad::btnArmEsc32()
{
    if ( !esc32)
        return;
    if ( ui->pushButton_esc32_read_arm_disarm->text() == "arm")
        esc32->sendCommand(BINARY_COMMAND_ARM,0.0f, 0.0f, 0);
    if ( ui->pushButton_esc32_read_arm_disarm->text() == "disarm")
        esc32->sendCommand(BINARY_COMMAND_DISARM,0.0f, 0.0f, 0);

}

void QGCAutoquad::btnStartStopEsc32()
{
    if ( !esc32)
        return;
    if ( ui->pushButton_esc32_read_start_stop->text() == "start")
        esc32->sendCommand(BINARY_COMMAND_START,0.0f, 0.0f, 0);
    if ( ui->pushButton_esc32_read_start_stop->text() == "stop")
        esc32->sendCommand(BINARY_COMMAND_STOP,0.0f, 0.0f, 0);
}

void QGCAutoquad::ParaWrittenEsc32(QString ParaName) {
    if ( ParaNameWritten == ParaName) {
        WaitForParaWriten = 0;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tab_aq_esc32 );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaNamEedt = edtList.at(i)->objectName();
            if ( ParaNamEedt == ParaName )
            {
                paramEsc32.remove(ParaName);
                paramEsc32.insert(ParaName,edtList.at(i)->text());
                break;
            }
        }
    }
}

void QGCAutoquad::CommandWrittenEsc32(int CommandName, QVariant V1, QVariant V2) {
    if ( CommandName == BINARY_COMMAND_ARM) {
        ui->pushButton_esc32_read_arm_disarm->setText("disarm");
    }
    if ( CommandName == BINARY_COMMAND_DISARM) {
        ui->pushButton_esc32_read_arm_disarm->setText("arm");
    }
    if ( CommandName == BINARY_COMMAND_START) {
        ui->pushButton_esc32_read_start_stop->setText("stop");
    }
    if ( CommandName == BINARY_COMMAND_STOP) {
        ui->pushButton_esc32_read_start_stop->setText("start");
    }
    if ( CommandName == BINARY_COMMAND_RPM) {
        ui->label_rpm->setText(V1.toString());
    }
}

void QGCAutoquad::btnSetRPM()
{
    if (( ui->pushButton_esc32_read_start_stop->text() == "stop") &&( ui->pushButton_esc32_read_arm_disarm->text() == "disarm")) {
        float rpm = (float)ui->horizontalSlider_rpm->value();
        ui->label_rpm->setText(QString::number(ui->horizontalSlider_rpm->value()));
        esc32->sendCommand(BINARY_COMMAND_RPM,rpm, 0.0f, 1);
    }

}

void QGCAutoquad::Esc32RpmSlider(int rpm) {
    ui->label_rpm->setText(QString::number(rpm));
}

void QGCAutoquad::saveEEpromEsc32()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values are transmitted to Esc32! Do you want to store the para into ROM?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            esc32->sendCommand(BINARY_COMMAND_CONFIG,1.0f, 0.0f, 1);
        }
        break;
        case QMessageBox::No:
        break;
        default:
        // should never be reached
        break;
    }
}

void QGCAutoquad::Esc32Connected(){
    esc32->ReadConfigEsc32();
}

void QGCAutoquad::ESc32Disconnected() {
}

void QGCAutoquad::setupPortList()
{
    ui->portName->clear();
    ui->portName->clearEditText();

    ui->comboBox_port_esc32->clear();
    ui->comboBox_port_esc32->clearEditText();
    // Get the ports available on this system
	seriallink = new SerialLink();
    QVector<QString>* ports = seriallink->getCurrentPorts();

    // Add the ports in reverse order, because we prepend them to the list
    for (int i = ports->size() - 1; i >= 0; --i)
    {
        // Prepend newly found port to the list
        if (ui->portName->findText(ports->at(i)) == -1)
        {
            ui->portName->insertItem(0, ports->at(i));
        }
        if (ui->comboBox_port_esc32->findText(ports->at(i)) == -1)
        {
            ui->comboBox_port_esc32->insertItem(0, ports->at(i));
        }
    }
    ui->portName->setEditText(seriallink->getPortName());
    ui->comboBox_port_esc32->setEditText(seriallink->getPortName());
}

void QGCAutoquad::selectFWToFlash()
{
    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ hex (*.hex)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }
	
    if (fileNames.size() > 0)
    {
        QString fileNameLocale = QDir::toNativeSeparators(fileNames.first());
        QFile file(fileNameLocale );
        ui->fileLabel->setText(fileNameLocale );
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read hex file. Permission denied");
            msgBox.exec();
        }
        fileToFlash = file.fileName();
        LastFilePath = fileToFlash;
        file.close();
    }
}

void QGCAutoquad::flashFW()
{
	QString AppPath = "";
	#ifdef Q_OS_WIN
		AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "stm32flash.exe");
        #else
         AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "stm32flash");
	#endif


     QMessageBox msgBox;
     msgBox.setText("Flashing firmware, takes some seconds. Please wait 20sec. bevor you retry!");
     msgBox.exec();


	QStringList Arguments;
	Arguments.append("-b 57600");
    Arguments.append("-w" );
    Arguments.append(QDir::toNativeSeparators(ui->fileLabel->text()));
	Arguments.append("-v");
	Arguments.append(portName);
    active_cal_mode = 0;
    ui->textFlashOutput->clear();
    ps_master.start(AppPath , Arguments, QProcess::Unbuffered | QProcess::ReadWrite);
}

void QGCAutoquad::prtstexit(int) {
    if ( active_cal_mode == 0 ) {
        ui->flashButton->setEnabled(true);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 1 ) {
        ui->pushButton_start_cal1->setEnabled(true);
        ui->pushButton_abort_cal1->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 2 ) {
        ui->pushButton_start_cal2->setEnabled(true);
        ui->pushButton_abort_cal2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 3 ) {
        ui->pushButton_start_cal3->setEnabled(true);
        ui->pushButton_abort_cal3->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 4 ) {
        ui->pushButton_start_sim1->setEnabled(true);
        ui->pushButton_abort_sim1->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 41 ) {
        ui->pushButton_start_sim1_2->setEnabled(true);
        ui->pushButton_abort_sim1_2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 5 ) {
        ui->pushButton_start_sim2->setEnabled(true);
        ui->pushButton_abort_sim2->setEnabled(false);
        active_cal_mode = 0;
    }
    if ( active_cal_mode == 6 ) {
        ui->pushButton_start_sim3->setEnabled(true);
        ui->pushButton_abort_sim3->setEnabled(false);
        active_cal_mode = 0;
    }
}

void QGCAutoquad::prtstdout() {
        if ( active_cal_mode == 0 ) {
            output = ps_master.readAllStandardOutput();
            if ( output.contains("[uWrote")) {
                output = output.right(output.length()-3);
                ui->textFlashOutput->clear();
            }
            ui->textFlashOutput->append(output);
        }
        if ( active_cal_mode == 1 ) {
            output_cal1 = ps_master.readAllStandardOutput();
            if ( output_cal1.contains("[H") )
                    ui->textOutput_cal1->clear();

            ui->textOutput_cal1->append(output_cal1);
        }
        if ( active_cal_mode == 2 ) {
            output_cal2 = ps_master.readAllStandardOutput();
            if ( output_cal2.contains("[H") )
                    ui->textOutput_cal2->clear();

            ui->textOutput_cal2->append(output_cal2);
        }
        if ( active_cal_mode == 3 ) {
            output_cal3 = ps_master.readAllStandardOutput();
            if ( output_cal3.contains("[H") )
                    ui->textOutput_cal3->clear();

            ui->textOutput_cal3->append(output_cal3);
        }
        if ( active_cal_mode == 4 ) {
            output_sim1 = ps_master.readAllStandardOutput();
            if ( output_sim1.contains("[H") )
                    ui->textOutput_sim1->clear();

            ui->textOutput_sim1->append(output_sim1);
        }
        if ( active_cal_mode == 41 ) {
            output_sim1b = ps_master.readAllStandardOutput();
            if ( output_sim1b.contains("[H") )
                    ui->textOutput_sim1_2->clear();

            ui->textOutput_sim1_2->append(output_sim1b);
        }
        if ( active_cal_mode == 5 ) {
            output_sim2 = ps_master.readAllStandardOutput();
            if ( output_sim2.contains("[H") )
                    ui->textOutput_sim2->clear();

            ui->textOutput_sim2->append(output_sim2);
        }
        if ( active_cal_mode == 6 ) {
            output_sim3 = ps_master.readAllStandardOutput();
            if ( output_sim2.contains("[H") )
                    ui->textOutput_sim3->clear();

            ui->textOutput_sim3->append(output_sim3);
        }
}

void QGCAutoquad::prtstderr() {
    if ( active_cal_mode == 0 ) {
        output = ps_master.readAllStandardError();
	ui->textFlashOutput->append(output);
    }
    if ( active_cal_mode == 1 ) {
            output_cal1 = ps_master.readAllStandardError();
            if ( output_cal1.contains("[") )
                    ui->textOutput_cal1->clear();
            ui->textOutput_cal1->append(output_cal1);
    }
    if ( active_cal_mode == 2 ) {
            output_cal2 = ps_master.readAllStandardError();
            if ( output_cal2.contains("[") )
                    ui->textOutput_cal2->clear();
            ui->textOutput_cal2->append(output_cal2);
    }
    if ( active_cal_mode == 3 ) {
            output_cal3 = ps_master.readAllStandardError();
            if ( output_cal3.contains("[") )
                    ui->textOutput_cal3->clear();
            ui->textOutput_cal3->append(output_cal3);
    }
    if ( active_cal_mode == 4 ) {
            output_sim1 = ps_master.readAllStandardError();
            if ( output_sim1.contains("[") )
                    ui->textOutput_sim1->clear();
            ui->textOutput_sim1->append(output_sim1);
    }
    if ( active_cal_mode == 41 ) {
            output_sim1b = ps_master.readAllStandardError();
            if ( output_sim1b.contains("[") )
                    ui->textOutput_sim1_2->clear();
            ui->textOutput_sim1_2->append(output_sim1b);
    }
    if ( active_cal_mode == 5 ) {
            output_sim2 = ps_master.readAllStandardError();
            if ( output_sim2.contains("[") )
                    ui->textOutput_sim2->clear();
            ui->textOutput_sim2->append(output_sim2);
    }
    if ( active_cal_mode == 6 ) {
            output_sim3 = ps_master.readAllStandardError();
            if ( output_sim3.contains("[") )
                    ui->textOutput_sim3->clear();
            ui->textOutput_sim3->append(output_sim3);
    }
}

void QGCAutoquad::handleConnectButton()
{
    if (currLink) {
        if (currLink->isConnected()) {
            currLink->disconnect();
        } else {
            currLink->connect();
        }
    }
}

UASInterface* QGCAutoquad::getUAS()
{
    return uas;
}

void QGCAutoquad::addUAS(UASInterface* uas_ext)
{
    QString uasColor = uas_ext->getColor().name().remove(0, 1);

}

void QGCAutoquad::setActiveUAS(UASInterface* uas_ext)
{
    if (uas_ext)
    {
        uas = uas_ext;
        disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
        disconnect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(getGUIpara()));
        //if ( VisibleWidget == 1) {
            if ( !paramaq ) {
                paramaq = new QGCAQParamWidget(uas, this);
                ui->gridLayout_paramAQ->addWidget(paramaq);
                connect(paramaq, SIGNAL(requestParameterRefreshed()), this, SLOT(getGUIpara()));
                if ( LastFilePath == "")
                    paramaq->setFilePath(QCoreApplication::applicationDirPath());
                else
                    paramaq->setFilePath(LastFilePath);
            }
            paramaq->loadParaAQ();
            //getGUIpara();

            VisibleWidget = 2;
        //}
        connect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
    }
}

void QGCAutoquad::raw_transmitter_view() {
    if ( ui->checkBox_raw_value->checkState() ){
         disconnect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));

         ui->progressBar_Throttle->setMaximum(2500);
         ui->progressBar_Throttle->setMinimum(500);

         ui->progressBar_Roll->setMaximum(2500);
         ui->progressBar_Roll->setMinimum(500);

         ui->progressBar_Pitch->setMaximum(2500);
         ui->progressBar_Pitch->setMinimum(500);

         ui->progressBar_Rudd->setMaximum(2500);
         ui->progressBar_Rudd->setMinimum(500);

         ui->progressBar_Gear->setMaximum(2500);
         ui->progressBar_Gear->setMinimum(500);

         ui->progressBar_Flaps->setMaximum(2500);
         ui->progressBar_Flaps->setMinimum(500);

         ui->progressBar_Aux2->setMaximum(2500);
         ui->progressBar_Aux2->setMinimum(500);

         ui->progressBar_Aux3->setMaximum(2500);
         ui->progressBar_Aux3->setMinimum(500);

         connect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));

    }
    else
    {
        disconnect(uas, SIGNAL(remoteControlChannelRawChanged(int,float)), this, SLOT(setChannelRaw(int,float)));

        ui->progressBar_Throttle->setMaximum(-500);
        ui->progressBar_Throttle->setMinimum(1500);

        ui->progressBar_Roll->setMaximum(-1500);
        ui->progressBar_Roll->setMinimum(1500);

        ui->progressBar_Pitch->setMaximum(-1500);
        ui->progressBar_Pitch->setMinimum(1500);

        ui->progressBar_Rudd->setMaximum(-1500);
        ui->progressBar_Rudd->setMinimum(1500);

        ui->progressBar_Gear->setMaximum(-1500);
        ui->progressBar_Gear->setMinimum(1500);

        ui->progressBar_Flaps->setMaximum(-1500);
        ui->progressBar_Flaps->setMinimum(1500);

        ui->progressBar_Aux2->setMaximum(-1500);
        ui->progressBar_Aux2->setMinimum(1500);

        ui->progressBar_Aux3->setMaximum(-1500);
        ui->progressBar_Aux3->setMinimum(1500);

        connect(uas, SIGNAL(remoteControlChannelScaledChanged(int,float)), this, SLOT(setChannelScaled(int,float)));
    }
}

void QGCAutoquad::hideEvent(QHideEvent* event)
{
    if ( VisibleWidget <= 1)
        VisibleWidget = 0;
    QWidget::hideEvent(event);
    emit visibilityChanged(false);
}

void QGCAutoquad::showEvent(QShowEvent* event)
{
    // React only to internal (pre-display)
    // events
    if ( VisibleWidget <= 1)
        VisibleWidget = 1;

    if ( VisibleWidget == 1) {
        if ( uas != NULL)
        {
            paramaq = new QGCAQParamWidget(uas, this);
            ui->gridLayout_paramAQ->addWidget(paramaq);
            VisibleWidget = 2;
        }
    }
    QWidget::showEvent(event);
    emit visibilityChanged(true);
}

void QGCAutoquad::setChannelScaled(int channelId, float normalized)
{
    if (channelId == 0 )
    {
        qint32 val = (qint32)((normalized*10000.0f)/13)+750;
        ui->progressBar_Throttle->setValue(val);
        ui->label_chan_1_M->setText(QString::number(val));
    }
    if (channelId == 1 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Roll->setValue(val);
        ui->label_chan_2_M->setText(QString::number(val));
    }
    if (channelId == 2 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Pitch->setValue(val);
        ui->label_chan_3_M->setText(QString::number(val));
    }
    if (channelId == 3 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Rudd->setValue(val);
        ui->label_chan_4_M->setText(QString::number(val));
    }
    if (channelId == 4 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Gear->setValue(val);
        ui->label_chan_5_M->setText(QString::number(val));
    }
    if (channelId == 5 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Flaps->setValue(val);
        ui->label_chan_6_M->setText(QString::number(val) + " " + "Pos Hold");
    }
    if (channelId == 6 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux2->setValue(val);
        ui->label_chan_7_M->setText(QString::number(val));
    }
    if (channelId == 7 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux3->setValue(val);
        ui->label_chan_8_M->setText(QString::number(val));
    }

}

void QGCAutoquad::setChannelRaw(int channelId, float normalized)
{
    if (channelId == 0 )
    {
        qint32 val = (qint32)((normalized));
        ui->progressBar_Throttle->setValue(val);
        ui->label_chan_1_M->setText(QString::number(val));
    }
    if (channelId == 1 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Roll->setValue(val);
        ui->label_chan_2_M->setText(QString::number(val));
    }
    if (channelId == 2 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Pitch->setValue(val);
        ui->label_chan_3_M->setText(QString::number(val));
    }
    if (channelId == 3 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Rudd->setValue(val);
        ui->label_chan_4_M->setText(QString::number(val));
    }
    if (channelId == 4 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Gear->setValue(val);
        ui->label_chan_5_M->setText(QString::number(val));
    }
    if (channelId == 5 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Flaps->setValue(val);
        ui->label_chan_6_M->setText(QString::number(val) + " " + "Pos Hold");
    }
    if (channelId == 6 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux2->setValue(val);
        ui->label_chan_7_M->setText(QString::number(val));
    }
    if (channelId == 7 )
    {
        int val = (int)((normalized*10000.0f)/13);
        ui->progressBar_Aux3->setValue(val);
        ui->label_chan_8_M->setText(QString::number(val));
    }

}

void QGCAutoquad::check_var()
{
    if ( ui->checkBox_sim3_4_var->checkState()) {
        ui->lineEdit_variance->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_var->checkState()) {
        ui->lineEdit_variance->setEnabled(false);
    }

    if ( ui->checkBox_sim3_4_var_2->checkState()) {
        ui->lineEdit_variance_4->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_var_2->checkState()) {
        ui->lineEdit_variance_4->setEnabled(false);
    }

    if ( ui->checkBox_sim3_5_var->checkState()) {
        ui->lineEdit_variance_2->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_5_var->checkState()) {
        ui->lineEdit_variance_2->setEnabled(false);
    }


    if ( ui->checkBox_sim3_6_var->checkState()) {
        ui->lineEdit_variance_3->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_6_var->checkState()) {
        ui->lineEdit_variance_3->setEnabled(false);
    }

}

void QGCAutoquad::check_stop()
{
    if ( ui->checkBox_sim3_4_stop->checkState()) {
        ui->lineEdit_stop->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_stop->checkState()) {
        ui->lineEdit_stop->setEnabled(false);
    }

    if ( ui->checkBox_sim3_4_stop_2->checkState()) {
        ui->lineEdit_stop_4->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_4_stop_2->checkState()) {
        ui->lineEdit_stop_4->setEnabled(false);
    }

    if ( ui->checkBox_sim3_5_stop->checkState()) {
        ui->lineEdit_stop_2->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_5_stop->checkState()) {
        ui->lineEdit_stop_2->setEnabled(false);
    }

    if ( ui->checkBox_sim3_6_stop->checkState()) {
        ui->lineEdit_stop_3->setEnabled(true);
    }
    else if  ( !ui->checkBox_sim3_6_stop->checkState()) {
        ui->lineEdit_stop_3->setEnabled(false);
    }


}

void QGCAutoquad::startcal1(){
    QString AppPath = "";
    #ifdef Q_OS_WIN
        AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
        ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    #else
        AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
        ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
    #endif

        QStringList Arguments;

        Arguments.append("--rate");
        for ( int i = 0; i<StaticFiles.count(); i++) {
            Arguments.append(StaticFiles.at(i));
        }
        Arguments.append(":");

        active_cal_mode = 1;
        ui->textOutput_cal1->clear();
        ui->pushButton_start_cal1->setEnabled(false);
        ui->pushButton_abort_cal1->setEnabled(true);
        ui->textOutput_cal1->append(AppPath);
        for ( int i = 0; i<Arguments.count(); i++) {
            ui->textOutput_cal1->append(Arguments.at(i));
        }
        ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startcal2(){
    QString AppPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
#endif

    QStringList Arguments;

    Arguments.append("--acc");
    for ( int i = 0; i<StaticFiles.count(); i++) {
        Arguments.append(StaticFiles.at(i));
    }
    Arguments.append(":");

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 2;
    ui->textOutput_cal2->clear();
    ui->pushButton_start_cal2->setEnabled(false);
    ui->pushButton_abort_cal2->setEnabled(true);
    ui->textOutput_cal2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_cal2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startcal3(){
    QString AppPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "cal.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "cal");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_unix"));
#endif

    QStringList Arguments;

    Arguments.append("--mag");

    for ( int i = 0; i<StaticFiles.count(); i++) {
        Arguments.append(StaticFiles.at(i));
    }
    Arguments.append(":");

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    active_cal_mode = 3;
    ui->textOutput_cal3->clear();
    ui->pushButton_start_cal3->setEnabled(false);
    ui->pushButton_abort_cal3->setEnabled(true);
    ui->textOutput_cal3->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_cal3->append(Arguments.at(i));
    }

    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim1(){
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

    QStringList Arguments;

    Arguments.append("--gyo");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_4_var->checkState() ) {
        Arguments.append("--var=" + ui->lineEdit_variance->text());
    }
    if ( ui->checkBox_sim3_4_stop->checkState() ) {
        Arguments.append("--stop=" + ui->lineEdit_stop->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 4;
    ui->textOutput_sim1->clear();
    ui->pushButton_start_sim1->setEnabled(false);
    ui->pushButton_abort_sim1->setEnabled(true);
    ui->textOutput_sim1->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim1->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim1b(){
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

    QStringList Arguments;

    Arguments.append("--acc");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_4_var_2->checkState() ) {
        Arguments.append("--var=" + ui->lineEdit_variance_2->text());
    }
    if ( ui->checkBox_sim3_4_stop_2->checkState() ) {
        Arguments.append("--stop=" + ui->lineEdit_stop_2->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 41;
    ui->textOutput_sim1_2->clear();
    ui->pushButton_start_sim1_2->setEnabled(false);
    ui->pushButton_abort_sim1_2->setEnabled(true);
    ui->textOutput_sim1_2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim1_2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim2(){
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

    QStringList Arguments;

    Arguments.append("--acc");
    Arguments.append("--gyo");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append( QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_5_var->checkState() ) {
        Arguments.append("--var=" + ui->lineEdit_variance_2->text());
    }
    if ( ui->checkBox_sim3_5_stop->checkState() ) {
        Arguments.append("--stop=" + ui->lineEdit_stop_2->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 5;
    ui->textOutput_sim2->clear();
    ui->pushButton_start_sim2->setEnabled(false);
    ui->pushButton_abort_sim2->setEnabled(true);
    ui->textOutput_sim2->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim2->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::startsim3(){
    QString AppPath = "";
    QString Sim3ParaPath = "";
#ifdef Q_OS_WIN
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.exe");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\" + "aq_win" + "\\" + "sim3.params");
#else
    AppPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/" + "sim3");
    ps_master.setWorkingDirectory(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix"));
    Sim3ParaPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + "aq_unix" + "/sim3.params");
#endif

    QStringList Arguments;

    Arguments.append("--mag");
    Arguments.append("--incl");
    Arguments.append("-p");
    Arguments.append(Sim3ParaPath);
    Arguments.append("-p");
    Arguments.append(QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()));

    if ( ui->checkBox_sim3_6_var->checkState() ) {
        Arguments.append("--var=" + ui->lineEdit_variance_3->text());
    }
    if ( ui->checkBox_sim3_6_stop->checkState() ) {
        Arguments.append("--stop=" + ui->lineEdit_stop_3->text());
    }

    for ( int i = 0; i<DynamicFiles.count(); i++) {
        Arguments.append(DynamicFiles.at(i));
    }

    active_cal_mode = 6;
    ui->textOutput_sim3->clear();
    ui->pushButton_start_sim3->setEnabled(false);
    ui->pushButton_abort_sim3->setEnabled(true);
    ui->textOutput_sim3->append(AppPath);
    for ( int i = 0; i<Arguments.count(); i++) {
        ui->textOutput_sim3->append(Arguments.at(i));
    }
    ps_master.start(AppPath , Arguments, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void QGCAutoquad::abortcal1(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortcal2(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortcal3(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim1(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim1b(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim2(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::abortsim3(){
    if ( ps_master.Running)
        ps_master.close();
}

void QGCAutoquad::getGUIpara() {
	if ( !paramaq)
		return;

	QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( this );
	for ( int i = 0; i<edtList.count(); i++) {
		QString ParaName = edtList.at(i)->objectName();
		if ( ParaName.startsWith("CTRL",Qt::CaseSensitive) || ParaName.startsWith("NAV",Qt::CaseSensitive) || ParaName.startsWith("GMBL",Qt::CaseSensitive) || ParaName.startsWith("SPVR",Qt::CaseSensitive) ) {
			edtList.at(i)->setText(paramaq->getParaAQ(ParaName).toString());
		}
		else if ( ParaName.startsWith("MOT_",Qt::CaseSensitive)) {
			edtList.at(i)->setText(paramaq->getParaAQ(ParaName).toString());
		}
	}

    int radio_type = paramaq->getParaAQ("RADIO_TYPE").toInt();
    ui->comboBox_Radio_Type->setCurrentIndex(radio_type);
    ui->IMU_ROT->setText(paramaq->getParaAQ("IMU_ROT").toString());
    ui->IMU_MAG_DECL->setText(paramaq->getParaAQ("IMU_MAG_DECL").toString());

 }

void QGCAutoquad::setRadio() {

    if ( !paramaq)
            return;
    paramaq->setParameter(190,"RADIO_TYPE",ui->comboBox_Radio_Type->currentIndex());
    QuestionForROM();

}

void QGCAutoquad::setFrame() {

    if ( !paramaq)
            return;

    paramaq->setParameter(190,"MOT_PWRD_01_T",ui->MOT_PWRD_01_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_T",ui->MOT_PWRD_02_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_T",ui->MOT_PWRD_03_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_T",ui->MOT_PWRD_04_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_T",ui->MOT_PWRD_05_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_T",ui->MOT_PWRD_06_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_T",ui->MOT_PWRD_07_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_T",ui->MOT_PWRD_08_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_T",ui->MOT_PWRD_09_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_T",ui->MOT_PWRD_10_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_T",ui->MOT_PWRD_11_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_T",ui->MOT_PWRD_12_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_T",ui->MOT_PWRD_13_T->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_T",ui->MOT_PWRD_14_T->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_P",ui->MOT_PWRD_01_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_P",ui->MOT_PWRD_02_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_P",ui->MOT_PWRD_03_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_P",ui->MOT_PWRD_04_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_P",ui->MOT_PWRD_05_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_P",ui->MOT_PWRD_06_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_P",ui->MOT_PWRD_07_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_P",ui->MOT_PWRD_08_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_P",ui->MOT_PWRD_09_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_P",ui->MOT_PWRD_10_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_P",ui->MOT_PWRD_11_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_P",ui->MOT_PWRD_12_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_P",ui->MOT_PWRD_13_P->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_P",ui->MOT_PWRD_14_P->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_R",ui->MOT_PWRD_01_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_R",ui->MOT_PWRD_02_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_R",ui->MOT_PWRD_03_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_R",ui->MOT_PWRD_04_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_R",ui->MOT_PWRD_05_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_R",ui->MOT_PWRD_06_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_R",ui->MOT_PWRD_07_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_R",ui->MOT_PWRD_08_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_R",ui->MOT_PWRD_09_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_R",ui->MOT_PWRD_10_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_R",ui->MOT_PWRD_11_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_R",ui->MOT_PWRD_12_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_R",ui->MOT_PWRD_13_R->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_14_R",ui->MOT_PWRD_14_R->text().toFloat());

    paramaq->setParameter(190,"MOT_PWRD_01_Y",ui->MOT_PWRD_01_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_02_Y",ui->MOT_PWRD_02_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_03_Y",ui->MOT_PWRD_03_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_04_Y",ui->MOT_PWRD_04_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_05_Y",ui->MOT_PWRD_05_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_06_Y",ui->MOT_PWRD_06_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_07_Y",ui->MOT_PWRD_07_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_08_Y",ui->MOT_PWRD_08_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_09_Y",ui->MOT_PWRD_09_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_10_Y",ui->MOT_PWRD_10_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_11_Y",ui->MOT_PWRD_11_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_12_Y",ui->MOT_PWRD_12_Y->text().toFloat());
    paramaq->setParameter(190,"MOT_PWRD_13_Y",ui->MOT_PWRD_13_Y->text().toFloat());
	paramaq->setParameter(190,"MOT_PWRD_14_Y",ui->MOT_PWRD_14_Y->text().toFloat());

    paramaq->setParameter(190,"MOT_FRAME","0");

    QuestionForROM();
}

void QGCAutoquad::setUsersParams() {
    QString dirPath = QDir::toNativeSeparators(UsersParamsFile);
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Parameters (*.params)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        ShowUsersParams(QDir::toNativeSeparators(fileNames.at(0)));
    }
}

void QGCAutoquad::ShowUsersParams(QString fileName) {
    QFile file(fileName);
    UsersParamsFile = file.fileName();
    ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    ui->textOutput_Users_Params->setText(file.readAll());
    file.close();
}

void QGCAutoquad::CreateUsersParams() {
    QString dirPath = UsersParamsFile ;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Parameter-File (*.params)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }

    if (fileNames.size() > 0)
    {
        UsersParamsFile = fileNames.at(0);
    }
    if (!UsersParamsFile.endsWith(".params") )
        UsersParamsFile += ".params";

    UsersParamsFile = QDir::toNativeSeparators(UsersParamsFile);
    QFile file( UsersParamsFile );
    if ( file.exists())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("file already exists, Overwrite?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                file.close();
                QFile::remove(UsersParamsFile );
            }
            break;
            case QMessageBox::No:
            // ok was clicked
            break;
            default:
            // should never be reached
            break;
        }
    }

    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      return;
    }
    QDataStream stream( &file );
    stream << "";
    file.close();
    ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
}

void QGCAutoquad::WriteUsersParams() {
    QString message = ui->textOutput_Users_Params->toPlainText();
    QFile file(UsersParamsFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    QTextStream out(&file);
    out << message;
    file.close();
}

void QGCAutoquad::LoadFrameFromFile() {

    QString dirPath;
    if ( LastFilePath == "")
        dirPath = QCoreApplication::applicationDirPath();
    else
        dirPath = LastFilePath;
    QFileInfo dir(dirPath);
    QFileDialog dialog;
    dialog.setDirectory(dir.absoluteDir());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setFilter(tr("AQ Mixing Table (*.mix)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
    }
	
    if (fileNames.size() > 0)
    {
        QString fileName = fileNames.first();
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox msgBox;
            msgBox.setText("Could not read mixing file. Permission denied");
            msgBox.exec();
			return;
        }
		file.close();

		QSettings settings(fileName, QSettings::IniFormat);

		settings.beginGroup("Throttle");
		ui->MOT_PWRD_01_T->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_T->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_T->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_T->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_T->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_T->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_T->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_T->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_T->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_T->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_T->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_T->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_T->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_T->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Pitch");
		ui->MOT_PWRD_01_P->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_P->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_P->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_P->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_P->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_P->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_P->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_P->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_P->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_P->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_P->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_P->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_P->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_P->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Roll");
		ui->MOT_PWRD_01_R->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_R->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_R->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_R->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_R->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_R->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_R->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_R->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_R->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_R->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_R->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_R->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_R->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_R->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.beginGroup("Yaw");
		ui->MOT_PWRD_01_Y->setText(settings.value("Motor1").toString());
		ui->MOT_PWRD_02_Y->setText(settings.value("Motor2").toString());
		ui->MOT_PWRD_03_Y->setText(settings.value("Motor3").toString());
		ui->MOT_PWRD_04_Y->setText(settings.value("Motor4").toString());
		ui->MOT_PWRD_05_Y->setText(settings.value("Motor5").toString());
		ui->MOT_PWRD_06_Y->setText(settings.value("Motor6").toString());
		ui->MOT_PWRD_07_Y->setText(settings.value("Motor7").toString());
		ui->MOT_PWRD_08_Y->setText(settings.value("Motor8").toString());
		ui->MOT_PWRD_09_Y->setText(settings.value("Motor9").toString());
		ui->MOT_PWRD_10_Y->setText(settings.value("Motor10").toString());
		ui->MOT_PWRD_11_Y->setText(settings.value("Motor11").toString());
		ui->MOT_PWRD_12_Y->setText(settings.value("Motor12").toString());
		ui->MOT_PWRD_13_Y->setText(settings.value("Motor13").toString());
		ui->MOT_PWRD_14_Y->setText(settings.value("Motor14").toString());
		settings.endGroup();
		settings.sync();
	}

}

void QGCAutoquad::SaveFrameToFile() {

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "./motorMixing.mix", tr("AQ Mixing Table (*.mix)"));
	if ( !fileName.endsWith(".mix"))
		fileName += ".mix";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
	file.close();

	QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Throttle");
	settings.setValue("Motor1", ui->MOT_PWRD_01_T->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_T->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_T->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_T->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_T->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_T->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_T->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_T->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_T->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_T->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_T->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_T->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_T->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_T->text());
    settings.endGroup();
    settings.beginGroup("Pitch");
	settings.setValue("Motor1", ui->MOT_PWRD_01_P->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_P->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_P->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_P->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_P->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_P->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_P->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_P->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_P->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_P->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_P->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_P->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_P->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_P->text());
    settings.endGroup();
    settings.beginGroup("Roll");
	settings.setValue("Motor1", ui->MOT_PWRD_01_R->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_R->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_R->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_R->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_R->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_R->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_R->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_R->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_R->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_R->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_R->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_R->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_R->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_R->text());
    settings.endGroup();
    settings.beginGroup("Yaw");
	settings.setValue("Motor1", ui->MOT_PWRD_01_Y->text());
	settings.setValue("Motor2", ui->MOT_PWRD_02_Y->text());
	settings.setValue("Motor3", ui->MOT_PWRD_03_Y->text());
	settings.setValue("Motor4", ui->MOT_PWRD_04_Y->text());
	settings.setValue("Motor5", ui->MOT_PWRD_05_Y->text());
	settings.setValue("Motor6", ui->MOT_PWRD_06_Y->text());
	settings.setValue("Motor7", ui->MOT_PWRD_07_Y->text());
	settings.setValue("Motor8", ui->MOT_PWRD_08_Y->text());
	settings.setValue("Motor9", ui->MOT_PWRD_09_Y->text());
	settings.setValue("Motor10", ui->MOT_PWRD_10_Y->text());
	settings.setValue("Motor11", ui->MOT_PWRD_11_Y->text());
	settings.setValue("Motor12", ui->MOT_PWRD_12_Y->text());
	settings.setValue("Motor13", ui->MOT_PWRD_13_Y->text());
	settings.setValue("Motor14", ui->MOT_PWRD_14_Y->text());
    settings.endGroup();
    settings.sync();

}

void QGCAutoquad::CalculatDeclination() {

	QString dec_source = ui->lineEdit_insert_declination->text();
	if ( !dec_source.contains(".")) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
	}
	if ( !dec_source.startsWith("-")) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
	}
	if ( dec_source.length() != 6 ) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
	}
	QStringList HoursMinutes = dec_source.mid(1,(dec_source.length()-1)).split(".");

    if ( HoursMinutes.count() != 2 ) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
        return;
	}
	qint32 secounds = HoursMinutes.at(1).toInt();
    float secounds_calc = (100.0f/60.0f) * secounds;
	// Set together
    QString recalculated;
    recalculated.append("#define");
    recalculated.append(' ');
    recalculated.append("IMU_MAG_INCL");
    recalculated.append(' ');
    recalculated.append("-");
    recalculated.append(HoursMinutes.at(0));
    recalculated.append(".");
    recalculated.append( QString::number(secounds_calc,'f',0));

    ui->lineEdit_cal_declination->setText(recalculated);

}

void QGCAutoquad::QuestionForROM()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setInformativeText("The values are transmitted to AutoQuad! Do you want to store the para into ROM?");
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
        {
            uas->writeParametersToStorageAQ();
        }
        break;
        case QMessageBox::No:
        break;
        default:
        // should never be reached
        break;
    }
}

void QGCAutoquad::save_PID_toAQ1()
{
QVariant val_uas;
QVariant val_local;
bool changed;

    if ( !paramaq)
        return;

    QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tabWidget_para_edit_1 );
    for ( int i = 0; i<edtList.count(); i++) {
        QString ParaName = edtList.at(i)->objectName();
        // Hier alle NAV von PID Page 1
        if ( ParaName.startsWith("CTRL_",Qt::CaseSensitive)) {
            if ( paramaq->getParameterValue(190,ParaName,val_uas) )
            {
                val_local = edtList.at(i)->text().toFloat();
                if ( val_uas != val_local) {
                    paramaq->setParameter(190,ParaName,val_local);
                    changed = true;
                }
            }
        }
    }
    if ( changed )
        QuestionForROM();

}

void QGCAutoquad::save_PID_toAQ2()
{
    QVariant val_uas;
    QVariant val_local;
    bool changed;

        if ( !paramaq)
            return;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tabWidget_para_edit_2 );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            // Hier alle CTRL von PID Page 2
            if ( ParaName.startsWith("NAV_",Qt::CaseSensitive)) {
                if ( paramaq->getParameterValue(190,ParaName,val_uas) )
                {
                    val_local = edtList.at(i)->text().toFloat();
                    if ( val_uas != val_local) {
                        paramaq->setParameter(190,ParaName,val_local);
                        changed = true;
                    }
                }
            }
        }
        if ( changed )
            QuestionForROM();

}

void QGCAutoquad::save_PID_toAQ3()
{
    QVariant val_uas;
    QVariant val_local;
    bool changed;

        if ( !paramaq)
            return;

        QList<QLineEdit*> edtList = qFindChildren<QLineEdit*> ( ui->tabWidget_para_edit_3 );
        for ( int i = 0; i<edtList.count(); i++) {
            QString ParaName = edtList.at(i)->objectName();
            // Hier alle CTRL von PID Page 3
            if ( paramaq->getParameterValue(190,ParaName,val_uas) )
            {
                val_local = edtList.at(i)->text().toFloat();
                if ( val_uas != val_local) {
                    paramaq->setParameter(190,ParaName,val_local);
                    changed = true;
                }
            }
        }


        if ( changed )
            QuestionForROM();

}

void QGCAutoquad::save_plot_image(){
    QString fileName = "plot.svg";
    fileName = QFileDialog::getSaveFileName(
                   this, "Export File Name", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
                   "SVG Images (*.svg);;PDF Documents (*.pdf)");

    if (!fileName.contains(".")) {
        // .svg is default extension
        fileName.append(".svg");
    }

    while(!(fileName.endsWith(".svg") || fileName.endsWith(".pdf"))) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Unsuitable file extension for PDF or SVG");
        msgBox.setInformativeText("Please choose .pdf or .svg as file extension. Click OK to change the file extension, cancel to not save the file.");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        // Abort if cancelled
        if(msgBox.exec() == QMessageBox::Cancel) return;
        fileName = QFileDialog::getSaveFileName(
                       this, "Export File Name", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation),
                       "PDF Documents (*.pdf);;SVG Images (*.svg)");
    }

    if (fileName.endsWith(".svg")) {
        exportSVG(fileName);
    } else if (fileName.endsWith(".pdf")) {
        exportPDF(fileName);
    }}

void QGCAutoquad::exportPDF(QString fileName)
{
    /*
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    //printer.setFullPage(true);
    printer.setPageMargins(10.0, 10.0, 10.0, 10.0, QPrinter::Millimeter);
    printer.setPageSize(QPrinter::A4);

    QString docName = plot->title().text();
    if ( !docName.isEmpty() ) {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("QGroundControl");
    printer.setOrientation(QPrinter::Landscape);

    plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
    //        plot->setCanvasBackground(Qt::white);
    //        QwtPlotPrintFilter filter;
    //        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
    //        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
    //        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);
    //        if ( printer.colorMode() == QPrinter::GrayScale )
    //        {
    //            int options = QwtPlotPrintFilter::PrintAll;
    //            options &= ~QwtPlotPrintFilter::PrintBackground;
    //            options |= QwtPlotPrintFilter::PrintFrameWithScales;
    //            filter.setOptions(options);
    //        }
    plot->print(printer);//, filter);
    plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
    //plot->setCanvasBackground(QColor(5, 5, 8));

    */
}

void QGCAutoquad::exportSVG(QString fileName)
{
    if ( !fileName.isEmpty() ) {
        plot->setStyleSheet("QWidget { background-color: #FFFFFF; color: #000000; background-clip: border; font-size: 10pt;}");
        //plot->setCanvasBackground(Qt::white);
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(QSize(800, 600));

        QwtPlotPrintFilter filter;
        filter.color(Qt::white, QwtPlotPrintFilter::CanvasBackground);
        filter.color(Qt::black, QwtPlotPrintFilter::AxisScale);
        filter.color(Qt::black, QwtPlotPrintFilter::AxisTitle);
        filter.color(Qt::black, QwtPlotPrintFilter::MajorGrid);
        filter.color(Qt::black, QwtPlotPrintFilter::MinorGrid);

        plot->print(generator, filter);
        plot->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; font-size: 11pt;}");
    }
}


void QGCAutoquad::startSetMarker() {
    if ( picker == NULL ) {
        if ( ui->comboBox_marker->currentIndex() == 6) {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }
            QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
            picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPicker::PointSelection,
                         QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOff,
                         plot->canvas());
            picker->setRubberBand(QwtPicker::CrossRubberBand);
            connect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
            StepCuttingPlot = 0;
        }
        else {
            if ( MarkerCut1 != NULL) {
                MarkerCut1->setVisible(false);
                MarkerCut1->detach();
                MarkerCut1 = NULL;
            }
            if ( MarkerCut2 != NULL) {
                MarkerCut2->setVisible(false);
                MarkerCut2->detach();
                MarkerCut2 = NULL;
            }
            if ( MarkerCut3 != NULL) {
                MarkerCut3->setVisible(false);
                MarkerCut3->detach();
                MarkerCut3 = NULL;
            }
            if ( MarkerCut4 != NULL) {
                MarkerCut4->setVisible(false);
                MarkerCut4->detach();
                MarkerCut4 = NULL;
            }

            double x1,x2,y1,y2 = 0;
            int time_count = 0;
            //200 Hz
            if ( ui->comboBox_marker->currentIndex() == 0)
                time_count = 200 * 1;
            if ( ui->comboBox_marker->currentIndex() == 1)
                time_count = 200 * 2;
            if ( ui->comboBox_marker->currentIndex() == 2)
                time_count = 200 * 3;
            if ( ui->comboBox_marker->currentIndex() == 3)
                time_count = 200 * 5;
            if ( ui->comboBox_marker->currentIndex() == 4)
                time_count = 200 * 10;
            if ( ui->comboBox_marker->currentIndex() == 5)
                time_count = 200 * 15;

            MarkerCut1 = new QwtPlotMarker();
            MarkerCut1->setLabel(QString::fromLatin1("sp1"));
            MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
            MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
            x1 = parser.xValues.value("XVALUES")->value(0);
            y1 = parser.yValues.values().at(0)->value(0);;
            MarkerCut1->setValue(x1,y1);
            MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut1->setVisible(true);
            MarkerCut1->attach(plot);

            MarkerCut2 = new QwtPlotMarker();
            MarkerCut2->setLabel(QString::fromLatin1("ep1"));
            MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
            MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
            x2 = parser.xValues.value("XVALUES")->value(time_count);
            y2 = parser.yValues.values().at(0)->value(time_count);;
            MarkerCut2->setValue(x2,y2);
            MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
            MarkerCut2->setVisible(true);
            MarkerCut2->attach(plot);

            MarkerCut3 = new QwtPlotMarker();
            MarkerCut3->setLabel(QString::fromLatin1("sp2"));
            MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
            MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
            x1 = (parser.xValues.values().at(0)->count()-1);
            y1 = parser.yValues.values().at(0)->value(parser.xValues.values().at(0)->count()-1);
            MarkerCut3->setValue(x1,y1);
            MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut3->setVisible(true);
            MarkerCut3->attach(plot);

            MarkerCut4 = new QwtPlotMarker();
            MarkerCut4->setLabel(QString::fromLatin1("ep2"));
            MarkerCut4->setLabelAlignment(Qt::AlignLeft|Qt::AlignBottom);
            MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
            x2 = (parser.xValues.values().at(0)->count()-1) - time_count;
            y2 = parser.yValues.values().at(0)->value((parser.xValues.values().at(0)->count()-1) - time_count);
            MarkerCut4->setValue(x2,y2);
            MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
            MarkerCut4->setVisible(true);
            MarkerCut4->attach(plot);
            plot->replot();
        }
    }
    else {
        disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
        picker->setEnabled(false);
        picker = NULL;
    }

}


void QGCAutoquad::setPoint1(const QwtDoublePoint &pos) {

    if ( StepCuttingPlot == 0) {
        MarkerCut1 = new QwtPlotMarker();
        MarkerCut1->setLabel(QString::fromLatin1("sp1"));
        MarkerCut1->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut1->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut1->setValue((int)pos.x(),pos.y());
        MarkerCut1->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut1->setVisible(true);
        MarkerCut1->attach(plot);
        StepCuttingPlot = 1;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
    }
    else if ( StepCuttingPlot == 1 ) {
        MarkerCut2 = new QwtPlotMarker();
        MarkerCut2->setLabel(QString::fromLatin1("ep1"));
        MarkerCut2->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut2->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut2->setValue((int)pos.x(),pos.y());
        MarkerCut2->setLinePen(QPen(QColor(QString("red"))));
        MarkerCut2->setVisible(true);
        MarkerCut2->attach(plot);
        StepCuttingPlot = 2;
        plot->replot();


        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("Select one more cutting area?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                StepCuttingPlot = 3;
                QMessageBox::information(this, "Information", "Please select the start point of the frame!",QMessageBox::Ok, 0 );
            }
            break;
            case QMessageBox::No:
            {
                disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
                picker->setEnabled(false);
                picker = NULL;

                QMessageBox msgBox;
                msgBox.setWindowTitle("Question");
                msgBox.setInformativeText("Delete the selected frames from the file?");
                msgBox.setWindowModality(Qt::ApplicationModal);
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                int ret = msgBox.exec();
                switch (ret) {
                    case QMessageBox::Yes:
                    {
                        parser.ReWriteFile("",MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
                    }
                    break;
                    case QMessageBox::No:
                        if ( MarkerCut1 != NULL) {
                            MarkerCut1->setVisible(false);
                            MarkerCut1->detach();
                            MarkerCut1 = NULL;
                        }
                        if ( MarkerCut2 != NULL) {
                            MarkerCut2->setVisible(false);
                            MarkerCut2->detach();
                            MarkerCut2 = NULL;
                        }
                        if ( MarkerCut3 != NULL) {
                            MarkerCut3->setVisible(false);
                            MarkerCut3->detach();
                            MarkerCut3 = NULL;
                        }
                        if ( MarkerCut4 != NULL) {
                            MarkerCut4->setVisible(false);
                            MarkerCut4->detach();
                            MarkerCut4 = NULL;
                        }
                    break;

                    default:
                    break;
                }
            }
            break;

            default:
            break;
        }

        /*
        disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
        picker->setEnabled(false);
        picker = NULL;
        plot->replot();
        */

    }
    else if ( StepCuttingPlot == 3 ) {
        MarkerCut3 = new QwtPlotMarker();
        MarkerCut3->setLabel(QString::fromLatin1("sp2"));
        MarkerCut3->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
        MarkerCut3->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut3->setValue((int)pos.x(),pos.y());
        MarkerCut3->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut3->setVisible(true);
        MarkerCut3->attach(plot);
        StepCuttingPlot = 4;
        plot->replot();
        QMessageBox::information(this, "Information", "Please select the end point of the frame!",QMessageBox::Ok, 0 );
    }
    else if ( StepCuttingPlot == 4 ) {
        MarkerCut4 = new QwtPlotMarker();
        MarkerCut4->setLabel(QString::fromLatin1("ep2"));
        MarkerCut4->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
        MarkerCut4->setLineStyle(QwtPlotMarker::VLine);
        MarkerCut4->setValue((int)pos.x(),pos.y());
        MarkerCut4->setLinePen(QPen(QColor(QString("blue"))));
        MarkerCut4->setVisible(true);
        MarkerCut4->attach(plot);
        StepCuttingPlot = 5;
        plot->replot();

        disconnect(picker, SIGNAL (selected(const QwtDoublePoint &)),  this, SLOT (setPoint1(const QwtDoublePoint &)) );
        picker->setEnabled(false);
        picker = NULL;

        QMessageBox msgBox;
        msgBox.setWindowTitle("Question");
        msgBox.setInformativeText("Delete the selected frames from the file?");
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Yes:
            {
                parser.ReWriteFile("",MarkerCut1->xValue(),MarkerCut2->xValue(),MarkerCut3->xValue(),MarkerCut4->xValue());
            }
            break;
            case QMessageBox::No:
                if ( MarkerCut1 != NULL) {
                    MarkerCut1->setVisible(false);
                    MarkerCut1->detach();
                    MarkerCut1 = NULL;
                }
                if ( MarkerCut2 != NULL) {
                    MarkerCut2->setVisible(false);
                    MarkerCut2->detach();
                    MarkerCut2 = NULL;
                }
                if ( MarkerCut3 != NULL) {
                    MarkerCut3->setVisible(false);
                    MarkerCut3->detach();
                    MarkerCut3 = NULL;
                }
                if ( MarkerCut4 != NULL) {
                    MarkerCut4->setVisible(false);
                    MarkerCut4->detach();
                    MarkerCut4 = NULL;
                }
            break;

            default:
            break;
        }
    }
}
