#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initializeRemoteCommandTable();
    initializeCommandTable();

    currentProfile = NULL;
    scenePixmap = NULL;
    signalMapper = NULL;
    commandComboMapper = NULL;

    int width = 300;
    int height = 600;

    scene = new QGraphicsScene(0,0,width,height);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(0,0,width,height);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui->graphicsView->setBackgroundBrush(QColor(230, 200, 167));

    qremoteBoxClient = new QRemoteBoxClient();
    connect(qremoteBoxClient, SIGNAL(remoteCommandReceived(QRemoteBoxClient::RemoteCommand)),
            this, SLOT(remoteCommandReceived(QRemoteBoxClient::RemoteCommand)));
    connect(qremoteBoxClient, SIGNAL(networkConnectedChanged(bool)),
            this, SLOT(networkConnectedChanged(bool)));
    connect(qremoteBoxClient, SIGNAL(serialPortConnectedChanged(bool)),
            this, SLOT(serialPortConnectedChanged(bool)));

    connect(qremoteBoxClient, SIGNAL(commandFinished(QRemoteBoxClient::Command,QVariant)),
            this, SLOT(commandFinished(QRemoteBoxClient::Command,QVariant)));
    connect(qremoteBoxClient, SIGNAL(commandTimedOut(QRemoteBoxClient::Command)),
            this, SLOT(commandTimedOut(QRemoteBoxClient::Command)));
    connect(qremoteBoxClient, SIGNAL(queueFinished()),
            this, SLOT(queueFinished()));
    connect(qremoteBoxClient, SIGNAL(queueStarted()),
            this, SLOT(queueStarted()));
    connect(qremoteBoxClient, SIGNAL(storedCommandsChanged(QStringList)),
            this, SLOT(storedCommandsChanged(QStringList)));

    QSettings tmpConfig(QSettings::IniFormat, QSettings::UserScope, "iremote", "settings");
    settingsDir = QFileInfo(tmpConfig.fileName()).absolutePath() + "/";

    loadSettings();
}

MainWindow::~MainWindow()
{
    unloadProfile();
    saveSettings();

    delete ui;
}

void MainWindow::applicationStarted(QString message)
{
    qDebug() << message;
    QRemoteBoxClient::RemoteCommand command = getRemoteCommand(message);
    qremoteBoxClient->runCommand(command);
}

void MainWindow::loadSettings()
{
    QSettings settings(settingsDir + "settings.ini", QSettings::IniFormat);

    int profileSize = settings.beginReadArray("profile");
    for (int num = 0; num < profileSize; num++)
    {
        settings.setArrayIndex(num);
        profiles.append(Profile());
        profiles.last().profileName = settings.value("profileName").toString();
        profiles.last().pictureFileName = settings.value("pictureFileName").toString();
        currentProfile = &(profiles.last());

        int size = settings.beginReadArray("command");
        for (int i = 0; i < size; i++)
        {
            settings.setArrayIndex(i);
            CommandCombination commandCombination;
            commandCombination.commandName = settings.value("commandName").toString();
            commandCombination.buttonData.name = settings.value("buttonName").toString();
            commandCombination.buttonData.rect = settings.value("buttonRect").toRectF();
            commandCombination.buttonData.pos = settings.value("buttonPos").toPointF();
            profiles.last().commandList.append(commandCombination);
        }
        settings.endArray();
    }
    settings.endArray();

    int size = settings.beginReadArray("remoteCommand");
    for (int i = 0; i < size; i++)
    {
        QByteArray bytes;
        QString name;
        RemoteCommandMapItem mapItem;

        settings.setArrayIndex(i);
        mapItem.isOnDevice = false;
        mapItem.comment = settings.value("comment", "").toString();
        name = settings.value("name").toString();
        bytes = settings.value("command").toByteArray();
        memcpy(&mapItem.remoteCommand, bytes.data(), sizeof(QRemoteBoxClient::RemoteCommand));
        addRemoteCommandMapItem(name, mapItem);
    }
    settings.endArray();

    currentProfile = NULL;
    if (profiles.size() > 0)
    {
        refreshProfiles();
    }

    // GUI settings
    settings.beginGroup("serial");
        ui->serialDeviceCombo->setCurrentIndex(settings.value("device", 0).toInt());
    settings.endGroup();

    settings.beginGroup("network");
        ui->networkAddressEdit->setText(settings.value("address", "192.168.1.5").toString());
        ui->networkPortSpin->setValue(settings.value("port", 2000).toInt());
    settings.endGroup();

    settings.beginGroup("wlan");
        ui->wlanHostnameEdit->setText(settings.value("hostname", "IRemoteBox").toString());
        ui->wlanSsidEdit->setText(settings.value("ssid", "").toString());
        ui->wlanSecurityCombo->setCurrentIndex(settings.value("auth", 0).toInt());
        ui->wlanPassphraseEdit->setText(settings.value("passphrase", "").toString());
    settings.endGroup();

    settings.beginGroup("ip");
        ui->ipAddressEdit->setText(settings.value("address", "169.254.1.1").toString());
        ui->ipMethodCombo->setCurrentIndex(settings.value("method", 0).toInt());
        ui->subnetMaskEdit->setText(settings.value("subnetMask", "255.255.0.0").toString());
        ui->gatewayEdit->setText(settings.value("gateway", "").toString());
    settings.endGroup();

    settings.beginGroup("ir");
        ui->irCountSpin->setValue(settings.value("repeat", 5).toInt());
        ui->irReceiveTimeoutSpin->setValue(settings.value("receiveTimeout", 30).toInt());
        ui->irSendTimeoutSpin->setValue(settings.value("sendTimeout", 100).toInt());
    settings.endGroup();

    ui->profileCombo->setCurrentIndex(settings.value("currentProfile", 0).toInt());
}

void MainWindow::saveSettings()
{
    QSettings settings(settingsDir + "settings.ini", QSettings::IniFormat);

    settings.beginWriteArray("profile");
    for (int num = 0; num < profiles.size(); num++)
    {
        settings.setArrayIndex(num);
        settings.setValue("profileName", profiles.at(num).profileName);
        settings.setValue("pictureFileName", profiles.at(num).pictureFileName);

        settings.beginWriteArray("command");
        for (int i = 0; i < profiles.at(num).commandList.size(); i++)
        {
            settings.setArrayIndex(i);
            settings.setValue("commandName", profiles.at(num).commandList.at(i).commandName);
            settings.setValue("buttonName", profiles.at(num).commandList.at(i).buttonData.name);
            settings.setValue("buttonRect", profiles.at(num).commandList.at(i).buttonData.rect);
            settings.setValue("buttonPos", profiles.at(num).commandList.at(i).buttonData.pos);
        }
        settings.endArray();
    }
    settings.endArray();

    int i = 0;
    settings.beginWriteArray("remoteCommand");
        QMapIterator<QString, RemoteCommandMapItem> iterator(remoteCommandMap);
         while (iterator.hasNext()) {

             QByteArray bytes;
             iterator.next();
             bytes.append((char*)(&(iterator.value().remoteCommand)), sizeof(QRemoteBoxClient::RemoteCommand));

             settings.setArrayIndex(i);
             settings.setValue("name", iterator.key());
             settings.setValue("command", bytes);
             settings.setValue("comment", iterator.value().comment);

             i++;
         }
     settings.endArray();

     // GUI settings
     settings.beginGroup("serial");
        settings.setValue("device", ui->serialDeviceCombo->currentIndex());
     settings.endGroup();

     settings.beginGroup("network");
         settings.setValue("address", ui->networkAddressEdit->text());
         settings.setValue("port", ui->networkPortSpin->value());
     settings.endGroup();

     settings.beginGroup("wlan");
         settings.setValue("hostname", ui->wlanHostnameEdit->text());
         settings.setValue("ssid", ui->wlanSsidEdit->text());
         settings.setValue("auth", ui->wlanSecurityCombo->currentIndex());
         settings.setValue("passphrase", ui->wlanPassphraseEdit->text());
     settings.endGroup();

     settings.beginGroup("ip");
         settings.setValue("address", ui->ipAddressEdit->text());
         settings.setValue("method", ui->ipMethodCombo->currentIndex());
         settings.setValue("subnetMask", ui->subnetMaskEdit->text());
         settings.setValue("gateway", ui->gatewayEdit->text());
     settings.endGroup();

     settings.beginGroup("ir");
         settings.setValue("repeat", ui->irCountSpin->value());
         settings.setValue("receiveTimeout", ui->irReceiveTimeoutSpin->value());
         settings.setValue("sendTimeout", ui->irSendTimeoutSpin->value());
     settings.endGroup();

     settings.setValue("currentProfile", ui->profileCombo->currentIndex());

    settings.sync();
}

void MainWindow::buttonClicked(int id)
{
    QString commandName = currentProfile->commandList.at(id).commandName;

    if (remoteCommandMap.find(commandName) != remoteCommandMap.end())
    {
        qremoteBoxClient->runCommand(remoteCommandMap.value(commandName).remoteCommand);
    }
}

void MainWindow::remoteCommandReceived(QRemoteBoxClient::RemoteCommand remoteCommand)
{
    RemoteCommandMapItem mapItem;
    mapItem.remoteCommand = remoteCommand;
    mapItem.isOnDevice = false;

    QString name = QInputDialog::getText(this, tr("New Command"), tr("Insert the name of the new command"));
    if (!name.isEmpty())
    {
        addRemoteCommandMapItem(name, mapItem);
        refreshProfiles();
    }
}

void MainWindow::loadProfile()
{
    if (signalMapper != NULL)
    {
        signalMapper->deleteLater();
    }
        signalMapper = new QSignalMapper(this);
        connect(signalMapper, SIGNAL(mapped(int)),
                this, SLOT(buttonClicked(int)));

        commandComboMapper = new QSignalMapper(this);
        connect(commandComboMapper, SIGNAL(mapped(int)),
                this, SLOT(commandComboClicked(int)));

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    for (int i = 0; i < currentProfile->commandList.size(); i++)
    {
        GraphicButton *button = createButton(currentProfile->commandList.at(i).buttonData.name,
                                             currentProfile->commandList.at(i).buttonData.rect,
                                             currentProfile->commandList.at(i).buttonData.pos,
                                             i);
        currentProfile->commandList[i].button = button;
        addCommandTableRow(button->buttonName(), currentProfile->commandList.at(i).commandName);
    }

    if (!currentProfile->pictureFileName.isEmpty())
        loadPicture(currentProfile->pictureFileName);
}

void MainWindow::unloadProfile()
{
    for (int i = 0; i < currentProfile->commandList.size(); i++)
    {
        GraphicButton *button = currentProfile->commandList.at(i).button;
        currentProfile->commandList[i].buttonData.name = button->buttonName();
        currentProfile->commandList[i].buttonData.rect = button->rect();
        currentProfile->commandList[i].buttonData.pos = button->pos();
        scene->removeItem(currentProfile->commandList.at(i).button);
        currentProfile->commandList[i].button = NULL;
    }

    unloadPicture();
}

void MainWindow::addProfile(QString name)
{
    Profile profile;
    profile.profileName = name;

    profiles.append(profile);
}

void MainWindow::removeProfile(int id)
{
    if (profiles.size() > id)
    {
        unloadProfile();
        currentProfile = NULL;
        profiles.removeAt(id);
    }
}

void MainWindow::refreshProfiles()
{
    int previousIndex = ui->profileCombo->currentIndex();

    ui->profileCombo->clear();
    for (int i = 0; i < profiles.size(); i++)
    {
        ui->profileCombo->addItem(profiles.at(i).profileName);
    }

    if ((previousIndex > -1) && (profiles.size() > previousIndex))
    {
        ui->profileCombo->setCurrentIndex(previousIndex);
    }
}

void MainWindow::getConfig()
{
    qremoteBoxClient->getIrCount();  // Remove the HELLO message

    // Get IR Config
    qremoteBoxClient->getIrCount();
    qremoteBoxClient->getIrReceiveTimeout();
    qremoteBoxClient->getIrSendTimeout();

    // Get 433MHz Config
    qremoteBoxClient->getRadio433Count();
    qremoteBoxClient->getRadio433ReceiveTimeout();
    qremoteBoxClient->getRadio433SendTimeout();

    // Get 868MHz Config
    qremoteBoxClient->getRadio868Count();
    qremoteBoxClient->getRadio433ReceiveTimeout();
    qremoteBoxClient->getRadio868SendTimeout();

    // Get WLAN Config
    qremoteBoxClient->getWlanSsid();
    qremoteBoxClient->getWlanHostname();
    qremoteBoxClient->getWlanSubnetMask();
    qremoteBoxClient->getWlanGateway();
    qremoteBoxClient->getWlanIpAddress();
    qremoteBoxClient->getWlanAuth();
    qremoteBoxClient->getWlanDhcpMethod();
}

void MainWindow::setConfig()
{
    // Set IR Config
    qremoteBoxClient->setIrCount(ui->irCountSpin->value());
    qremoteBoxClient->setIrReceiveTimeout(ui->irReceiveTimeoutSpin->value());
    qremoteBoxClient->setIrSendTimeout(ui->irSendTimeoutSpin->value());

    // Set 433MHz Config
    qremoteBoxClient->setRadio433Count(ui->radio433CountSpin->value());
    qremoteBoxClient->setRadio433ReceiveTimeout(ui->radio433ReceiveTimeoutSpin->value());
    qremoteBoxClient->setRadio433SendTimeout(ui->radio433SendTimeoutSpin->value());

    // Set 868MHz Config
    qremoteBoxClient->setRadio868Count(ui->radio868CountSpin->value());
    qremoteBoxClient->setRadio868ReceiveTimeout(ui->radio868ReceiveTimeoutSpin->value());
    qremoteBoxClient->setRadio868SendTimeout(ui->radio868SendTimeoutSpin->value());

    // Set WLAN Config
    QRemoteBoxClient::WlanAuthType authType = QRemoteBoxClient::OpenAuthType;

    switch (ui->wlanSecurityCombo->currentIndex())
    {
    case 0: authType = QRemoteBoxClient::OpenAuthType;
        break;
    case 1: authType = QRemoteBoxClient::WEP128AuthType;
        break;
    case 2: authType = QRemoteBoxClient::WPA1AuthType;
        break;
    case 3: authType = QRemoteBoxClient::MixedWPA1AndWPA2PSKAuthType;
        break;
    case 4: authType = QRemoteBoxClient::WPA2PSKAuthType;
        break;
    case 5: authType = QRemoteBoxClient::AdhocAuthType;
        break;
    case 6: authType = QRemoteBoxClient::WPE64AuthType;
        break;
    }
    qremoteBoxClient->setWlanSsid(ui->wlanSsidEdit->text());
    qremoteBoxClient->setWlanAuth(authType);
    qremoteBoxClient->setWlanHostname(ui->wlanHostnameEdit->text());
    if ((authType == QRemoteBoxClient::WEP128AuthType) ||
            (authType == QRemoteBoxClient::WPE64AuthType))
    {
        qremoteBoxClient->setWlanKey(ui->wlanPassphraseEdit->text());
    }
    else if ((authType == QRemoteBoxClient::WPA1AuthType) ||
             (authType == QRemoteBoxClient::MixedWPA1AndWPA2PSKAuthType) ||
             (authType == QRemoteBoxClient::WPA2PSKAuthType))
    {
        qremoteBoxClient->setWlanPhrase(ui->wlanPassphraseEdit->text());
    }

    QRemoteBoxClient::IpDhcpMethod dhcpMethod = QRemoteBoxClient::DhcpOnMethod;

    switch (ui->ipMethodCombo->currentIndex())
    {
    case 0: dhcpMethod = QRemoteBoxClient::DhcpOnMethod;
        break;
    case 1: dhcpMethod = QRemoteBoxClient::DhcpOffMethod;
        break;
    case 2: dhcpMethod = QRemoteBoxClient::AutoIpMethod;
        break;
    }
    qremoteBoxClient->setWlanDhcpMethod(dhcpMethod);

    if (dhcpMethod == QRemoteBoxClient::DhcpOffMethod)
    {
        qremoteBoxClient->setWlanIpAddress(ui->ipAddressEdit->text());
        qremoteBoxClient->setWlanSubnetMask(ui->subnetMaskEdit->text());
        qremoteBoxClient->setWlanGateway(ui->gatewayEdit->text());
    }

    qremoteBoxClient->saveConfig();
}

GraphicButton *MainWindow::createButton(QString name, QRectF rect, QPointF pos, int id)
{
    GraphicButton *item = new GraphicButton;
    item->setPos(pos);
    item->setRect(rect);
    item->setButtonName(name);
    item->setEditable(ui->editableCheck->isChecked());
    scene->addItem(item);

    signalMapper->setMapping(item,id);
    connect(item, SIGNAL(clicked()),
            signalMapper, SLOT(map()));

    return item;
}

void MainWindow::addCommand(QString commandName)
{
    CommandCombination command;
    command.buttonData.name = "";
    command.buttonData.rect = QRect(0,0,50,50);
    command.buttonData.pos = QPointF(10,10);
    command.commandName = commandName;

    unloadProfile();
    currentProfile->commandList.append(command);
    loadProfile();
}

void MainWindow::removeCommand(int id)
{
    ui->tableWidget->removeRow(id);
    scene->removeItem(currentProfile->commandList.at(id).button);
    currentProfile->commandList.at(id).button->deleteLater();
    currentProfile->commandList.removeAt(id);
}

void MainWindow::loadPicture(QString fileName)
{
    QPixmap pixmap;
    pixmap.load(fileName);

    if (!pixmap.isNull())
    {
        pixmap = pixmap.scaled(scene->width(),
                               scene->height(),
                               Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    }

    if (scenePixmap == NULL)
        scenePixmap = scene->addPixmap(pixmap);
    else
        scenePixmap->setPixmap(pixmap);

    scenePixmap->setZValue(-1);
}

void MainWindow::unloadPicture()
{
    if (scenePixmap != NULL)
        scenePixmap->setPixmap(QPixmap());
}

void MainWindow::addCommandTableRow(QString buttonName, QString commandName)
{
    QTableWidgetItem *newItem;
    QComboBox *box;
    int row;
    int column;

    row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row + 1);

    // Column 0 Name
    column = 0;
    newItem = new QTableWidgetItem(buttonName);
    ui->tableWidget->setItem(row, column, newItem);

    // Column 1 combo box
    column = 1;
    box = new QComboBox(this);
    for (int i = 0; i < ui->remoteCommandTable->rowCount(); i++)
    {
        box->addItem(ui->remoteCommandTable->item(i, 0)->text());
    }
    box->setCurrentIndex(box->findText(commandName));
    ui->tableWidget->setCellWidget(row, column, box);

    commandComboMapper->setMapping(box, row);
    connect(box, SIGNAL(currentIndexChanged(int)),
            commandComboMapper, SLOT(map()));

    ui->tableWidget->resizeColumnToContents(1);

}

void MainWindow::initializeCommandTable()
{
    QStringList labels;

    labels << tr("Command") << tr("Button");
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    //ui->tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    //ui->tableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}

void MainWindow::addRemoteCommandMapItem(const QString name, RemoteCommandMapItem mapItem)
{
    remoteCommandMap.insert(name, mapItem);

    if (ui->remoteCommandTable->findItems(name, Qt::MatchCaseSensitive).isEmpty())
    {
        QTableWidgetItem *newItem;
        int row;
        int column;

        row = ui->remoteCommandTable->rowCount();
        ui->remoteCommandTable->setRowCount(row + 1);

        // Column 0 Name
        column = 0;
        newItem = new QTableWidgetItem("");
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->remoteCommandTable->setItem(row, column, newItem);

        // Column 1 Medium
        column = 1;
        newItem = new QTableWidgetItem("");
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->remoteCommandTable->setItem(row, column, newItem);

        // Column 2 Comment
        column = 2;
        newItem = new QTableWidgetItem("");
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->remoteCommandTable->setItem(row, column, newItem);

        // Column 3 On device
        column = 3;
        newItem = new QTableWidgetItem("");
        newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->remoteCommandTable->setItem(row, column, newItem);

        updateTableRow(row, name, mapItem);
    }
}

void MainWindow::updateTableRow(int row, QString name, MainWindow::RemoteCommandMapItem mapItem)
{
    QTableWidgetItem *tableItem;
    QString mediumName;
    QString onDeviceText;
    QColor onDeviceColor;

    // Column 0 Name
    tableItem = ui->remoteCommandTable->item(row, 0);
    tableItem->setText(name);

    // Column 1 Medium
    if (mapItem.remoteCommand.medium == QRemoteBoxClient::RemoteMedium_Ir) {
        mediumName = tr("IR");
    }
    else if (mapItem.remoteCommand.medium == QRemoteBoxClient::RemoteMedium_433MHz) {
        mediumName = tr("433MHz");
    }
    else if (mapItem.remoteCommand.medium == QRemoteBoxClient::RemoteMedium_868MHz) {
        mediumName = tr("868MHz");
    }
    tableItem = ui->remoteCommandTable->item(row, 1);
    tableItem->setText(mediumName);

    // Column 2 Comment
    tableItem = ui->remoteCommandTable->item(row, 2);
    tableItem->setText(mapItem.comment);

    // Column 3 On device
    if (mapItem.isOnDevice)
    {
        onDeviceText = tr("Yes");
        onDeviceColor = QColor(Qt::green);
    }
    else
    {
        onDeviceText = tr("No");
        onDeviceColor = QColor(Qt::transparent);
    }
    tableItem = ui->remoteCommandTable->item(row, 3);
    tableItem->setBackgroundColor(onDeviceColor);
    tableItem->setText(onDeviceText);
}

void MainWindow::removeRemoteCommand(const QString name)
{
    if (!remoteCommandMap.contains(name))
    {
        return;
    }

    remoteCommandMap.remove(name);

    for (int i = ui->remoteCommandTable->rowCount()-1; i >= 0; i--)
    {
        if (ui->remoteCommandTable->item(i, 0)->text() == name)
        {
            ui->remoteCommandTable->removeRow(i);
            return;
        }
    }
}

void MainWindow::copyRemoteCommand(const QString name)
{
    RemoteCommandMapItem mapItem = remoteCommandMap.value(name);

    addRemoteCommandMapItem(name + "_copy", mapItem);
}

void MainWindow::renameRemoteCommand(const QString name, const QString newName)
{
    RemoteCommandMapItem mapItem = remoteCommandMap.value(name);

    removeRemoteCommand(name);
    addRemoteCommandMapItem(newName, mapItem);
}

void MainWindow::changeOnDeviceRemoteCommand(int row, const QString name, bool onDevice, int pos)
{
    RemoteCommandMapItem mapItem = remoteCommandMap.value(name);
    mapItem.isOnDevice = onDevice;
    mapItem.onDevicePos = pos;

    remoteCommandMap.insert(name, mapItem);

    updateTableRow(row, name, mapItem);
}

QRemoteBoxClient::RemoteCommand MainWindow::getRemoteCommand(const QString name)
{
    return remoteCommandMap.value(name).remoteCommand;
}

MainWindow::RemoteCommandMapItem MainWindow::getRemoteCommandMapItem(const QString name)
{
    return remoteCommandMap.value(name);
}

void MainWindow::initializeRemoteCommandTable()
{
    QStringList labels;

    labels << tr("Name") << tr("Medium") << tr("Comment") << tr("On device");
    ui->remoteCommandTable->setColumnCount(4);
    ui->remoteCommandTable->setHorizontalHeaderLabels(labels);
    //ui->remoteCommandTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    //ui->remoteCommandTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    //ui->remoteCommandTable->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    QString cellContent = ui->tableWidget->item(row,column)->text();

    if ((column == 0) && (currentProfile->commandList.size() > row))
    {
        currentProfile->commandList.at(row).button->setButtonName(cellContent);
    }
}

void MainWindow::commandComboClicked(int row)
{
    currentProfile->commandList[row].commandName =  ((QComboBox*)(ui->tableWidget->cellWidget(row,1)))->currentText();
}

void MainWindow::commandFinished(QRemoteBoxClient::Command command, QVariant data)
{
    QRemoteBoxClient::WlanAuthType auth;

    qDebug() << "finished" <<  command << data;
    switch (command)
    {
    case QRemoteBoxClient::GetIrCountCommand: ui->irCountSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::GetIrReceiveTimeoutCommand: ui->irReceiveTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::GetIrSendTimeoutCommand: ui->irSendTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get433CountCommand: ui->radio433CountSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get433ReceiveTimeoutCommand: ui->radio433ReceiveTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get433SendTimeoutCommand: ui->radio433SendTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get868CountCommand: ui->radio868CountSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get868ReceiveTimeoutCommand: ui->radio433ReceiveTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::Get868SendTimeoutCommand: ui->radio868SendTimeoutSpin->setValue(data.toInt()); break;
    case QRemoteBoxClient::GetWlanSsidCommand: ui->wlanSsidEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanHostnameCommand: ui->wlanHostnameEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanMaskCommand: ui->subnetMaskEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanGatewayCommand: ui->gatewayEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanIpCommand: ui->ipAddressEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanAuthCommand:
        auth = data.value<QRemoteBoxClient::WlanAuthType>();
        switch (auth)
        {
        case QRemoteBoxClient::OpenAuthType: ui->wlanSecurityCombo->setCurrentIndex(0);
            break;
        case QRemoteBoxClient::WEP128AuthType: ui->wlanSecurityCombo->setCurrentIndex(1);
            break;
        case QRemoteBoxClient::WPA1AuthType: ui->wlanSecurityCombo->setCurrentIndex(2);
            break;
        case QRemoteBoxClient::MixedWPA1AndWPA2PSKAuthType: ui->wlanSecurityCombo->setCurrentIndex(3);
            break;
        case QRemoteBoxClient::WPA2PSKAuthType: ui->wlanSecurityCombo->setCurrentIndex(4);
            break;
        case QRemoteBoxClient::AdhocAuthType: ui->wlanSecurityCombo->setCurrentIndex(5);
            break;
        case QRemoteBoxClient::WPE64AuthType: ui->wlanSecurityCombo->setCurrentIndex(6);
            break;
        }
        if ((auth == QRemoteBoxClient::WEP128AuthType) ||
            (auth == QRemoteBoxClient::WPE64AuthType))
        {
            qremoteBoxClient->getWlanKey();
        }
        else if ((auth == QRemoteBoxClient::WPA1AuthType) ||
                 (auth == QRemoteBoxClient::MixedWPA1AndWPA2PSKAuthType) ||
                 (auth == QRemoteBoxClient::WPA2PSKAuthType))
        {
            qremoteBoxClient->getWlanPhrase();
        }
        break;
    case QRemoteBoxClient::GetWlanPhraseCommand:
    case QRemoteBoxClient::GetWlanKeyCommand: ui->wlanPassphraseEdit->setText(data.toString()); break;
    case QRemoteBoxClient::GetWlanDhcpCommand:
        switch (data.value<QRemoteBoxClient::IpDhcpMethod>())
        {
        case QRemoteBoxClient::DhcpOnMethod: ui->ipMethodCombo->setCurrentIndex(0);
            break;
        case QRemoteBoxClient::DhcpOffMethod: ui->ipMethodCombo->setCurrentIndex(1);
            break;
        case QRemoteBoxClient::AutoIpMethod: ui->ipMethodCombo->setCurrentIndex(2);
            break;
        case QRemoteBoxClient::DhcpCacheMethod: ui->ipMethodCombo->setCurrentIndex(3);
        }
        break;
    default: /* do nothing */ ;
    }
}

void MainWindow::commandTimedOut(QRemoteBoxClient::Command command)
{
    qDebug() << "timed out" << command;
}

void MainWindow::queueStarted()
{
    ui->statusBar->showMessage(tr("Running command"));
}

void MainWindow::queueFinished()
{
    ui->statusBar->showMessage(tr("Commands finished"));
}

void MainWindow::storedCommandsChanged(QStringList commandList)
{
    QString name;
    bool found;

    qDebug() << "list updated";
    for (int i = 0; i < ui->remoteCommandTable->rowCount(); i++)
    {
        name = ui->remoteCommandTable->item(i, 0)->text();
        found = false;
        foreach(QString command, commandList)
        {
            if (name == command)
            {
                changeOnDeviceRemoteCommand(i, name, true, commandList.indexOf(command));
                found = true;
                break;
            }
        }
        if (!found)
        {
            changeOnDeviceRemoteCommand(i, name, false, 0);
        }
    }
}

void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    for (int i = 0; i < currentProfile->commandList.size(); i++)
    {
        currentProfile->commandList.at(i).button->setFocused((row == i));
    }
}

void MainWindow::on_removeButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    removeCommand(row);
}

void MainWindow::on_addButton_clicked()
{
    if (currentProfile == NULL)
        return;

    if (!ui->editableCheck->isChecked())
        ui->editableCheck->click();

    int id = currentProfile->commandList.size();
    addCommand(QString("%1").arg(id));
}

void MainWindow::on_editableCheck_clicked()
{
    if (currentProfile == NULL)
        return;

    foreach(CommandCombination command, currentProfile->commandList)
    {
        command.button->setEditable(ui->editableCheck->isChecked());
    }
}

void MainWindow::on_imageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));
    if (!fileName.isNull())
    {
        loadPicture(fileName);
        currentProfile->pictureFileName = fileName;
    }
}

void MainWindow::on_networkConnectButton_clicked()
{
    if (!qremoteBoxClient->isNetworkConnected())
        qremoteBoxClient->connectNetwork(ui->networkAddressEdit->text(), ui->networkPortSpin->value());
    else
        qremoteBoxClient->closeNetwork();
}

#ifdef SERIALPORT
void IRemoteWindow::on_serialConnectButton_clicked()
{
    if (!iremote->isSerialPortConnected())
        iremote->connectSerialPort(ui->serialDeviceCombo->currentText());
    else
        iremote->closeSerialPort();
}
#endif

void MainWindow::serialPortConnectedChanged(bool connected)
{
    if (connected)
    {
        ui->serialStatusFrame->setStyleSheet("background-color: green;");
        ui->serialConnectButton->setText(tr("Disconnect"));
    }
    else
    {
        ui->serialStatusFrame->setStyleSheet("background-color: red;");
        ui->serialConnectButton->setText(tr("Connect"));
    }
}

void MainWindow::networkConnectedChanged(bool connected)
{
    if (connected)
    {
        ui->networkStatusFrame->setStyleSheet("background-color: green;");
        ui->networkConnectButton->setText(tr("Disconnect"));

        QTimer::singleShot(150, this, SLOT(getConfig()));
    }
    else
    {
        ui->networkStatusFrame->setStyleSheet("background-color: red;");
        ui->networkConnectButton->setText(tr("Connect"));
    }
}

void MainWindow::on_captureButton_clicked()
{
    qremoteBoxClient->captureIr();
}

void MainWindow::on_capture433Button_clicked()
{
    qremoteBoxClient->captureRadio433MHz();
}

void MainWindow::on_capture868Button_clicked()
{
    qremoteBoxClient->captureRadio868MHz();
}

void MainWindow::on_stopButton_clicked()
{
    qremoteBoxClient->stop();
}

void MainWindow::on_runButton_clicked()
{
    int row = ui->remoteCommandTable->currentRow();
    if (row < 0)
        return;

    QRemoteBoxClient::RemoteCommand command = getRemoteCommand(ui->remoteCommandTable->item(row, 0)->text());
    qremoteBoxClient->runCommand(command);
}

void MainWindow::on_ipMethodCombo_currentIndexChanged(int index)
{
    bool enabled = !((index == 0) || (index == 2));    //DHCP or Auto-Ip

    ui->ipAddressEdit->setEnabled(enabled);
    ui->ipAddressLabel->setEnabled(enabled);
    ui->subnetMaskEdit->setEnabled(enabled);
    ui->subnetMaskLabel->setEnabled(enabled);
    ui->gatewayEdit->setEnabled(enabled);
    ui->gatewayLabel->setEnabled(enabled);
    ui->primaryDnsEdit->setEnabled(enabled);
    ui->primaryDnsLabel->setEnabled(enabled);
    ui->secondaryDnsEdit->setEnabled(enabled);
    ui->secondaryDnsLabel->setEnabled(enabled);
}

void MainWindow::on_profileAddButton_clicked()
{
    QString name = QInputDialog::getText(this, tr("New Profile"), tr("Insert the name of the new profile"));
    if (!name.isEmpty())
    {
        addProfile(name);
        refreshProfiles();
    }
}

void MainWindow::on_profileRemoveButton_clicked()
{
    if (ui->profileCombo->currentIndex() > -1)
    {
        removeProfile(ui->profileCombo->currentIndex());
        refreshProfiles();
    }
}

void MainWindow::on_profileCombo_currentIndexChanged(int index)
{
    if (index == -1)
        return;

    if (currentProfile != NULL)
        unloadProfile();

    currentProfile = &(profiles[index]);
    loadProfile();
}

void MainWindow::on_wlanSecurityCombo_currentIndexChanged(int index)
{
    if (index == -1)
        return;

    bool enabled = (!((index == 0) || (index == 5)));   //Open mode or Adhoc

    ui->wlanPassphraseEdit->setEnabled(enabled);
}

void MainWindow::on_wlanAdhocButton_clicked()
{
    qremoteBoxClient->startWlanAp();
    ui->networkAddressEdit->setText("169.254.1.1");
}

void MainWindow::on_wlanInfrastructureButton_clicked()
{
    qremoteBoxClient->startWlanInfrastructure();
}

void MainWindow::on_removeCommandButton_clicked()
{
    int row = ui->remoteCommandTable->currentRow();
    if (row != -1)
    {
        removeRemoteCommand(ui->remoteCommandTable->item(row, 0)->text());
        refreshProfiles();
    }
}

void MainWindow::on_copyCommandButton_clicked()
{
    int row = ui->remoteCommandTable->currentRow();
    if (row != -1)
    {
        copyRemoteCommand(ui->remoteCommandTable->item(row, 0)->text());
        refreshProfiles();
    }
}

void MainWindow::on_renameCommandButton_clicked()
{
    int row = ui->remoteCommandTable->currentRow();
    if (row != -1)
    {
        QString oldName = ui->remoteCommandTable->item(row, 0)->text();

        QString name = QInputDialog::getText(this, tr("Rename Command"), tr("Insert the new name of the command"), QLineEdit::Normal, oldName);
        if (!name.isEmpty())
        {
            renameRemoteCommand(oldName, name);
            refreshProfiles();
        }
    }
}

void MainWindow::on_openFlashfileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open flash file"),
                                                    QDir::homePath(),
                                                    tr("Bin file (*.bin) (*.bin)"));

    if (!fileName.isNull())
    {
        ui->flashfileNameEdit->setText(fileName);
    }
}

void MainWindow::on_flashButton_clicked()
{
    qremoteBoxClient->flashFirmware(ui->flashfileNameEdit->text());
}

void MainWindow::on_remoteCommandTable_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    int row = ui->remoteCommandTable->currentRow();
    QString commandName = ui->remoteCommandTable->item(row, 0)->text();

    RemoteCommandMapItem mapItem = remoteCommandMap.value(commandName);
    ShowCommandDialog *dialog = new ShowCommandDialog(this);
    dialog->setRemoteCommand(mapItem.remoteCommand);
    dialog->exec();

    mapItem.remoteCommand = dialog->remoteCommand();

    addRemoteCommandMapItem(commandName, mapItem);
}

void MainWindow::on_hideCommandTableButton_clicked()
{
    if (ui->tableWidget->isVisible())
    {
        ui->tableWidget->setVisible(false);
        ui->profileWidget->setVisible(false);
        ui->buttonWidget->setVisible(false);
        ui->hideCommandTableButton->setIcon(QIcon::fromTheme("arrow-left"));
    }
    else
    {
        ui->tableWidget->setVisible(true);
        ui->profileWidget->setVisible(true);
        ui->buttonWidget->setVisible(true);
        ui->hideCommandTableButton->setIcon(QIcon::fromTheme("arrow-right"));
    }
}

void MainWindow::on_saveSettingsButton_clicked()
{
    setConfig();
}

void MainWindow::on_remoteCommandTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    Q_UNUSED(currentColumn)

    RemoteCommandMapItem mapItem;

    if (currentRow != -1)
    {
        ui->runButton->setEnabled(true);
    }
    else
    {
        ui->runButton->setEnabled(false);
    }

    mapItem = getRemoteCommandMapItem(ui->remoteCommandTable->item(currentRow, 0)->text());

    ui->dataAddButton->setEnabled(!mapItem.isOnDevice);
    ui->dataRemoveButton->setEnabled(mapItem.isOnDevice);
}

void MainWindow::on_dataAddButton_clicked()
{
    QString name;
    QRemoteBoxClient::StorageItem storageItem;

    int row = ui->remoteCommandTable->currentRow();
    if (row < 0)
        return;

    name = ui->remoteCommandTable->item(row, 0)->text();
    storageItem.remoteCommand = getRemoteCommand(name);
    storageItem.commandHeader.version = 0u;
    strncpy(storageItem.commandHeader.commandName, name.toLocal8Bit().data(), 50u);

    qremoteBoxClient->dataAdd(storageItem);
}

void MainWindow::on_dataRemoveButton_clicked()
{
    QString name;
    RemoteCommandMapItem mapItem;

    int row = ui->remoteCommandTable->currentRow();
    if (row < 0)
        return;

    name = ui->remoteCommandTable->item(row, 0)->text();
    mapItem =getRemoteCommandMapItem(name);

    qremoteBoxClient->dataRemove(mapItem.onDevicePos);
}

void MainWindow::on_eraseAllButton_clicked()
{
    qremoteBoxClient->dataEraseAll();
}
