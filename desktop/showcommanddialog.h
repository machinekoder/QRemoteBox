#ifndef SHOWCOMMANDDIALOG_H
#define SHOWCOMMANDDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include "qremoteboxclient.h"

namespace Ui {
class ShowCommandDialog;
}

class ShowCommandDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRemoteBoxClient::RemoteCommand remoteCommand READ remoteCommand WRITE setRemoteCommand NOTIFY remoteCommandChanged)
    
public:
    explicit ShowCommandDialog(QWidget *parent = 0);

    ~ShowCommandDialog();

    QRemoteBoxClient::RemoteCommand remoteCommand() const
    {
        return m_remoteCommand;
    }

public slots:
    void setRemoteCommand(const QRemoteBoxClient::RemoteCommand &arg)
    {
        m_remoteCommand = arg;
        refreshCommand();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void remoteCommandChanged(QRemoteBoxClient::RemoteCommand arg);

private slots:
    void on_tableButton_clicked();

    void on_listAddButton_clicked();

    void on_listRemoveButton_clicked();

    void on_listRefreshButton_clicked();

private:
    Ui::ShowCommandDialog *ui;

    QRemoteBoxClient::RemoteCommand m_remoteCommand;
    QGraphicsScene *scene;
    QList<QGraphicsLineItem *> plotLines;

    void refreshCommand();
};

#endif // SHOWCOMMANDDIALOG_H
