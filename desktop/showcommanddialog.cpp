#include "showcommanddialog.h"
#include "ui_showcommanddialog.h"

ShowCommandDialog::ShowCommandDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowCommandDialog)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    ui->graphicsView->setScene(scene);

    ui->tableWidget->setVisible(false);
    ui->listAddButton->setVisible(false);
    ui->listRemoveButton->setVisible(false);
    ui->listRefreshButton->setVisible(false);

    ui->graphicsView->installEventFilter(this);
}

ShowCommandDialog::~ShowCommandDialog()
{
    delete ui;
}

bool ShowCommandDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (((event->type() == QEvent::Show)) && (obj == ui->graphicsView)) {
        refreshCommand();
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void ShowCommandDialog::on_tableButton_clicked()
{
    if (ui->tableWidget->isVisible())
    {
        ui->tableWidget->setVisible(false);
        ui->listAddButton->setVisible(false);
        ui->listRemoveButton->setVisible(false);
        ui->listRefreshButton->setVisible(false);
        ui->tableButton->setIcon(QIcon::fromTheme("arrow-left"));
    }
    else
    {
        ui->tableWidget->setVisible(true);
        ui->listAddButton->setVisible(true);
        ui->listRemoveButton->setVisible(true);
        ui->listRefreshButton->setVisible(true);
        ui->tableButton->setIcon(QIcon::fromTheme("arrow-right"));
    }
}

void ShowCommandDialog::on_listAddButton_clicked()
{
    int row = ui->tableWidget->currentRow();

    //if (row != -1)
    //{
    row++;
    ui->tableWidget->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setText(QString::fromUtf8("%1").arg(0));
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    ui->tableWidget->setItem(row,0,item);
    //}
}

void ShowCommandDialog::on_listRemoveButton_clicked()
{
    int row = ui->tableWidget->currentRow();

    if (row != -1)
    {
        ui->tableWidget->removeRow(row);
    }
}

void ShowCommandDialog::on_listRefreshButton_clicked()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        m_remoteCommand.data[i] = ui->tableWidget->item(i, 0)->text().toInt();
    }

    m_remoteCommand.length = ui->tableWidget->rowCount();
    refreshCommand();
    emit remoteCommandChanged(m_remoteCommand);
}

void ShowCommandDialog::refreshCommand()
{
    QVector<double> x;
    QVector<double> y;
    double time = 0;

    ui->tableWidget->clear();

#if 0
    if (plotCurve != NULL) {
        plotCurve->detach();
        delete plotCurve;
    }
#endif
    foreach (QGraphicsLineItem *item, plotLines)
    {
        delete item;
    }
    plotLines.clear();

    x.append(0);
    y.append(1);
    x.append(0);
    y.append(0);

    ui->tableWidget->setRowCount(m_remoteCommand.length);

    for (int i = 0; i < m_remoteCommand.length; i++)
    {
        time += m_remoteCommand.data[i];
        x.append((double)time);
        y.append((i%2));
        x.append((double)time);
        y.append(!(i%2));
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(QString::fromUtf8("%1").arg(m_remoteCommand.data[i]));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
        ui->tableWidget->setItem(i,0,item);
    }

    int width;
    int height;

    width = ui->graphicsView->width();
    height = ui->graphicsView->height();

    scene->setSceneRect(QRectF(0,0,width, height));
    for (int i  = 1; i < x.size(); i++)
    {
        plotLines.append(scene->addLine((x.at(i-1)/time)*width,
                                        y.at(i-1)*height,
                                        (x.at(i)/time)*width,
                                        y.at(i)*height,
                                        QPen(QBrush(Qt::green),2)));
    }
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::IgnoreAspectRatio);

    if (m_remoteCommand.medium == 0) {
        this->setWindowTitle("IR Command");
    } else if (m_remoteCommand.medium == 1) {
        this->setWindowTitle("433MHz Command");
    } else if (m_remoteCommand.medium == 2) {
        this->setWindowTitle("868MHz Command");
    }


    ui->frequencySpin->setValue(m_remoteCommand.frequency);
}
