#ifndef IREMOTEWINDOW_H
#define IREMOTEWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSettings>
#include <QFileDialog>
#include <QSignalMapper>
#include "graphicbutton.h"
#include "qremoteboxclient.h"
#include "showcommanddialog.h"

typedef struct {
    QString name;
    QRectF rect;
    QPointF pos;
} Button;

typedef struct {
    GraphicButton *button;
    Button buttonData;
    QString commandName;
} CommandCombination;

typedef struct {
    QList<CommandCombination> commandList;
    QString profileName;
    QString pictureFileName;
} Profile;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:

    typedef struct {
        QRemoteBoxClient::RemoteCommand remoteCommand;
        QString comment;
        bool isOnDevice;
        int onDevicePos;
    } RemoteCommandMapItem;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void applicationStarted(QString message);
    
private slots:
    void buttonClicked(int id);
    void remoteCommandReceived(QRemoteBoxClient::RemoteCommand remoteCommand);

    void serialPortConnectedChanged(bool connected);
    void networkConnectedChanged(bool connected);

    void commandComboClicked(int row);

    void commandFinished(QRemoteBoxClient::Command command, QVariant data);
    void commandTimedOut(QRemoteBoxClient::Command command);
    void queueStarted();
    void queueFinished();
    void storedCommandsChanged(QStringList commandList);

    void getConfig();
    void setConfig();

    void on_addButton_clicked();
    void on_editableCheck_clicked();
    void on_imageButton_clicked();
    void on_tableWidget_cellChanged(int row, int column);
    void on_tableWidget_cellClicked(int row, int column);
    void on_removeButton_clicked();
    void on_networkConnectButton_clicked();
#ifdef SERIALPORT
    void on_serialConnectButton_clicked();
#endif
    void on_captureButton_clicked();
    void on_runButton_clicked();
    void on_ipMethodCombo_currentIndexChanged(int index);
    void on_profileAddButton_clicked();
    void on_profileRemoveButton_clicked();
    void on_profileCombo_currentIndexChanged(int index);
    void on_wlanSecurityCombo_currentIndexChanged(int index);
    void on_wlanAdhocButton_clicked();
    void on_removeCommandButton_clicked();
    void on_openFlashfileButton_clicked();
    void on_flashButton_clicked();
    void on_wlanInfrastructureButton_clicked();
    void on_capture433Button_clicked();
    void on_copyCommandButton_clicked();
    void on_renameCommandButton_clicked();
    void on_capture868Button_clicked();
    void on_remoteCommandTable_doubleClicked(const QModelIndex &index);
    void on_hideCommandTableButton_clicked();
    void on_saveSettingsButton_clicked();
    void on_stopButton_clicked();

    void on_remoteCommandTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_dataAddButton_clicked();

    void on_eraseAllButton_clicked();

    void on_dataRemoveButton_clicked();

private:
    Ui::MainWindow *ui;

    QGraphicsScene *scene;
    QGraphicsPixmapItem *scenePixmap;
    QRemoteBoxClient *qremoteBoxClient;

    QList<Profile> profiles;

    Profile *currentProfile;

    QMap<QString,RemoteCommandMapItem> remoteCommandMap;

    QSignalMapper *signalMapper;
    QSignalMapper *commandComboMapper;

    QString settingsDir;

    GraphicButton *createButton(QString name, QRectF rect, QPointF pos, int id);
    void addCommand(QString commandName);
    void removeCommand(int id);
    void loadPicture(QString fileName);
    void unloadPicture();
    void addCommandTableRow(QString buttonName, QString commandName);
    void initializeCommandTable();

    void addRemoteCommandMapItem(const QString name, RemoteCommandMapItem mapItem);
    void updateTableRow(int row, QString name, MainWindow::RemoteCommandMapItem mapItem);
    void removeRemoteCommand(const QString name);
    void copyRemoteCommand(const QString name);
    void renameRemoteCommand(const QString name, const QString newName);
    void changeOnDeviceRemoteCommand(int row, const QString name, bool onDevice, int pos);
    QRemoteBoxClient::RemoteCommand getRemoteCommand(const QString name);
    MainWindow::RemoteCommandMapItem getRemoteCommandMapItem(const QString name);
    void initializeRemoteCommandTable();

    void loadSettings();
    void saveSettings();

    void loadProfile();
    void unloadProfile();

    void addProfile(QString name);
    void removeProfile(int id);
    void refreshProfiles();
};

#endif // IREMOTEWINDOW_H
