#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QtMath>
#include <QFile>
#include <QTextStream>
#include <QEventLoop>
#include <QShowEvent>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "xdotool.h"
#include "picker.h"

// Why does “extern const int n;” not work as expected?
// https://stackoverflow.com/questions/14894698/why-does-extern-const-int-n-not-work-as-expected

extern const int Direction_Up;
extern const int Direction_Down;
const int Direction_Up = 0;
const int Direction_Down = 1;
extern QString TranslateText(QString word);
extern QString OCRTranslate();

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(configTool.MainWindowWidth, configTool.MainWindowHeight);

    view = new QWebEngineView(this->centralWidget());
    view->setZoomFactor(1.2);
    view->setGeometry(10,10, width() - 30 , height() - 30);

    // 当页面加载完成后，获取html页面高度调整自身高度
    connect(view, &QWebEngineView::loadFinished, this, [=]{
        view->page()->runJavaScript("document.body.offsetHeight;",[=](QVariant result){
            int newHeight = int(result.toInt() * 1.2 + 10);
            view->setFixedSize(view->width(),newHeight);
            this->setFixedHeight(newHeight + 30);
            emit gotHeight();
        });
    });

    // 读取html模板

    QFile file(QCoreApplication::applicationDirPath() + "/interpret_js_1.html");
    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "fail to open";
    QTextStream in(&file);
    this->html1 = in.readAll();
    file.close();

    file.setFileName(QCoreApplication::applicationDirPath() + "/interpret_js_2.html");
    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "fail to open";
    in.setDevice(&file);
    this->html2 = in.readAll();
    file.close();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
    if (Direction == Direction_Up)
    {
        // 三角形占用了上面的区域
        this->setFixedHeight(this->height() + TriangleHeight);
    }
    e->ignore();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QColor greyColor(192, 192, 192);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen;
    pen.setColor(greyColor);
    pen.setWidth(3);
    painter.setPen(pen);

    QBrush brush;
    brush.setColor(Qt::white);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    QPolygon polygon;
    QPainterPath path;
    if (showTriangle == false)
    {
        centralWidget()->move(0, 0);
        path.addRoundedRect(5, 5, this->width() -5 -5, this->height()-5-5 , 15, 15);
        showTriangle = true;
    }
    else if (Direction == Direction_Down)
    {
        centralWidget()->move(0, 0);
        path.moveTo(0, 0);
        path.arcMoveTo(5,5,30,30,90);
        path.arcTo(5,5,30,30,90, 90);
        path.lineTo(5, this->height() - TriangleHeight - 15);
        path.arcTo(5,this->height() - TriangleHeight - 30,30,30,180, 90);
        path.lineTo(this->width() / 2 - TriangleWidth + TriangleOffset, this->height() - TriangleHeight);
        path.lineTo(this->width() / 2 + TriangleOffset, this->height());
        path.lineTo(this->width() / 2 + TriangleWidth + TriangleOffset, this->height() - TriangleHeight);
        path.lineTo(this->width() -5 -15, this->height() - TriangleHeight);
        path.arcTo(this->width() -5 -30 ,this->height() - TriangleHeight - 30,30,30,270, 90);
        path.lineTo(this->width() -5 , 5+15);
        path.arcTo(this->width() -5 -30 ,5,30,30,0, 90);
        path.closeSubpath();

    }
    else if (Direction == Direction_Up)
    {
        centralWidget()->move(centralWidget()->x(), TriangleHeight);
        path.moveTo(0, 0);
        path.arcMoveTo(5,TriangleHeight,30,30,90);
        path.arcTo(5,TriangleHeight,30,30,90, 90);
        path.lineTo(5, this->height() - 5 -15 );
        path.arcTo(5,this->height() - 5 - 30,30,30,180, 90);
        path.lineTo(this->width() -5 -15, this->height() - 5);
        path.arcTo(this->width() -5 -30 ,this->height() - 5 - 30,30,30,270, 90);
        path.lineTo(this->width() -5 , TriangleHeight+15);
        path.arcTo(this->width() -5 -30 ,TriangleHeight,30,30,0, 90);
        path.lineTo(this->width() / 2 + TriangleWidth + TriangleOffset, TriangleHeight);
        path.lineTo(this->width() / 2 + TriangleOffset, 0);
        path.lineTo(this->width() / 2 - TriangleWidth + TriangleOffset, TriangleHeight);
        path.closeSubpath();
    }
    painter.drawPath(path);
}

void MainWindow::onMouseButtonPressed(int x, int y)
{
    if (!this->isHidden() && (x < this->x() || x > this->x() + width() || y < this->y() || y > this->y() + height()))
        hide();

}

void MainWindow::onFloatButtonPressed(QPoint mousePressPosition, QPoint mouseReleasedPosition)
{
    QEventLoop qel;
    connect(this, &MainWindow::gotHeight, &qel, &QEventLoop::quit);
    // 获取翻译
    QString json = TranslateText(picker->getSelectedText());
    qDebug() << json;
    if (json.startsWith("{"))
    {
        QString html = this->html2;
        this->view->setHtml(html.replace("\"{0}\"", json));
    }
    else
    {
        QString html = this->html1;
        this->view->setHtml(html.replace("\"{0}\"", json));
    }
    // 等待页面加载完成
    qel.exec();
    // 获取默认方向向 重置三角形偏移量
    int direction = configTool.Direction;
    TriangleOffset = 0;

    QPoint mid(0, 0);
    mid.rx() = (mousePressPosition.x() + mouseReleasedPosition.x() - width()) / 2;

    if (direction == Direction_Up)
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    else
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;
    // 判断是否超出屏幕上边界
    if (direction == Direction_Down && mid.y() < 0)
    {
        direction = Direction_Up;
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    }
    // 判断是否超出屏幕下边界
    if (direction == Direction_Up && mid.y() + this->height() > xdotool.screenHeight)
    {
        direction = Direction_Down;
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;
    }
    Direction = direction;
    // 判断是否超出屏幕左边界
    if (mid.x() < configTool.Edge)
    {
        TriangleOffset = configTool.Edge - mid.x();
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = configTool.Edge;
        TriangleOffset = -TriangleOffset;
    }
    // 判断是否超出屏幕右边界
    if (mid.x() + this->width() > xdotool.screenWidth - configTool.Edge)
    {
        TriangleOffset = mid.x() + this->width() - (xdotool.screenWidth - configTool.Edge);
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = xdotool.screenWidth - configTool.Edge - this->width();
    }
    move(mid);
    this->show();

}

void MainWindow::onOCRShortCutPressed()
{
    QString res = OCRTranslate();
    QPoint mousePressPosition = xdotool.eventMonitor.mousePressPosition;
    QPoint mouseReleasedPosition = xdotool.eventMonitor.mouseReleasedPosition;
    QEventLoop qel;
    connect(this, &MainWindow::gotHeight, &qel, &QEventLoop::quit);
    qDebug() << res;
    if (res.startsWith("{"))
    {
        QString html = this->html2;
        this->view->setHtml(html.replace("\"{0}\"", res));
    }
    else if(res.isEmpty())
    {
        return;
    }
    else
    {
        QString html = this->html1;
        this->view->setHtml(html.replace("\"{0}\"", res));
    }
    // 等待页面加载完成
    qel.exec();

    // 获取默认方向向 重置三角形偏移量
    int direction = configTool.Direction;
    TriangleOffset = 0;

    QPoint mid(0, 0);
    mid.rx() = (mousePressPosition.x() + mouseReleasedPosition.x() - width()) / 2;

    if (direction == Direction_Up)
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    else
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;
    // 判断是否超出屏幕上边界
    if (direction == Direction_Down && mid.y() < 0)
    {
        direction = Direction_Up;
        mid.ry() = std::max(mousePressPosition.y(), mouseReleasedPosition.y()) + 15;
    }
    // 判断是否超出屏幕下边界
    if (direction == Direction_Up && mid.y() + this->height() > xdotool.screenHeight)
    {
        direction = Direction_Down;
        mid.ry() = std::min(mousePressPosition.y(), mouseReleasedPosition.y()) - this->height() - 15;
    }
    Direction = direction;
    // 判断是否超出屏幕左边界
    if (mid.x() < configTool.Edge)
    {
        TriangleOffset = configTool.Edge - mid.x();
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = configTool.Edge;
        TriangleOffset = -TriangleOffset;
    }
    // 判断是否超出屏幕右边界
    if (mid.x() + this->width() > xdotool.screenWidth - configTool.Edge)
    {
        TriangleOffset = mid.x() + this->width() - (xdotool.screenWidth - configTool.Edge);
        if (TriangleOffset > this->width() / 2 - TriangleWidth * 2)
            TriangleOffset = this->width() / 2 - TriangleWidth * 2;
        mid.rx() = xdotool.screenWidth - configTool.Edge - this->width();
    }
    move(mid);
    this->show();
    this->activateWindow();
}

void MainWindow::onSearchBarReturned(QPoint pos, QPoint size, QString res)
{
    QEventLoop qel;
    connect(this, &MainWindow::gotHeight, &qel, &QEventLoop::quit);
    res = TranslateText(res);
    if (res.startsWith("{"))
    {
        QString html = this->html2;
        this->view->setHtml(html.replace("\"{0}\"", res));
    }
    else
    {
        QString html = this->html1;
        this->view->setHtml(html.replace("\"{0}\"", res));
    }
    // 等待页面加载完成
    qel.exec();

    this->showTriangle = false;
    QPoint mid;
    mid.ry() = pos.y() + size.y();
    mid.rx() = pos.x() + size.x() / 2 - this->width() / 2;
    // 判断是否超出屏幕下边界
    if (mid.y() + this->height() > xdotool.screenHeight)
    {
        mid.ry() = pos.y() - this->height();
    }
    move(mid);
    this->show();
}
