#include "mainwindow.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPainter>
#include <QTimer>
#include "weathertool.h"
#include <QPainter>

#include "ui_mainwindow.h"

#define INCREMENT 1.2  //温度每升高/降低1°，y坐标的增量
#define POINT_RADIUS 3 //曲线描点的大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    //设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);  // 无边框
    setFixedSize(width(), height());         // 固定窗口大小


    // 定义右键菜单和菜单项
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText(tr("退出"));
    mExitAct->setIcon(QIcon(":/res/close.png"));
    //将菜单项添加到菜单中
    mExitMenu->addAction(mExitAct);


    //点击菜单项后发送信号给当前界面(this)，使用lambda直接执行退出
    connect(mExitAct, &QAction::triggered, this, [=]() { qApp->exit(0); });

    //天气类型
    weatherType();


    // 网络请求
    mNetAccessManager = new QNetworkAccessManager(this);
    // 连接信号槽,finished是应答完成处理时(getWeatherInfo成功后)发出的信号，然后我们调用onReplied函数去处理
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

    getWeatherInfo("濮阳");


    //事件过滤器是接收发送到该对象的所有事件的对象。过滤器可以停止事件或将其转发到此对象（this）。
    //事件过滤器filterObj通过它的eventFilter()函数接收事件然后去做处理。
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow() { delete ui; }


//重写事件，实现单击鼠标右键退出程序
void MainWindow::contextMenuEvent(QContextMenuEvent* event) {
    mExitMenu->exec(QCursor::pos());
    event->accept(); //表示请求已处理，无需向上发送
}


//获取鼠标按下/移动 时的位置(不理解的话画个图就清晰了)
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //globalPos返回的是鼠标点相对于电脑屏幕的坐标
    //this->pos返回的是程序窗口相对于电脑屏幕的坐标
    //globalPos-this->pos就是当前鼠标点到程序窗口的距离
    mOffset = event->globalPos() - this->pos();
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //鼠标点相对于电脑屏幕的坐标 - 当前鼠标点到程序窗口的距离
    this->move(event->globalPos() - mOffset);
}


// 用于请求天气数据
void MainWindow::getWeatherInfo(QString cityName)
{
    QString cityCode =  WeatherTool::getCityCode(cityName);
    if(cityCode.isEmpty())
    {
        QMessageBox::warning(this,"天气","请检查输入是否正确！",QMessageBox::Ok);
        return;
    }

    QUrl url("http://t.weather.itboy.net/api/weather/city/" + cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}


// 解析天气数据and更新UI
void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray,&err); //将byteArray中的json数据传给doc

    //如果出错了就直接退出,不在继续向下执行
    if(err.error != QJsonParseError::NoError)
    {
        return;
    }

    QJsonObject rootObj = doc.object(); //doc.object()方法会返回JSON文档的根对象


    // 1. 解析日期和城市
    mToday.date = rootObj.value("date").toString();
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();

    // 2. 解析yesterday
    QJsonObject objData = rootObj.value("data").toObject();
    QJsonObject objYesterday =  objData.value("yesterday").toObject();

    mDay[0].week = objYesterday.value("week").toString();
    mDay[0].date = objYesterday.value("ymd").toString();
    mDay[0].type = objYesterday.value("type").toString();

    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].high = s.toInt();

    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].low = s.toInt();

    mDay[0].fx = objYesterday.value("fx").toString();
    mDay[0].fl = objYesterday.value("fl").toString();

    mDay[0].aqi = objYesterday.value("aqi").toDouble();

    // 3. 解析forecast中5天的数据
    QJsonArray forecastArr = objData.value("forecast").toArray();

    //mDay[0]表示昨天的数据,已经赋值了,这里循环给未来五天赋值 i+1
    for(int i=0; i<5; i++)
    {
       QJsonObject objforecast = forecastArr[i].toObject();
       mDay[i+1].week = objforecast.value("week").toString();
       mDay[i+1].date = objforecast.value("ymd").toString();

       mDay[i+1].type = objforecast.value("type").toString();

       QString s;
       s = objforecast.value("high").toString().split(" ").at(1);
       s = s.left(s.length()-1);
       mDay[i+1].high = s.toInt();

       s = objforecast.value("low").toString().split(" ").at(1);
       s = s.left(s.length()-1);
       mDay[i+1].low = s.toInt();

       mDay[i+1].fx = objforecast.value("fx").toString();
       mDay[i+1].fl = objforecast.value("fl").toString();

       mDay[i+1].aqi = objforecast.value("aqi").toDouble();

    }

    // 4. 解析今天的数据
    mToday.ganmao = objData.value("ganmao").toString();

    mToday.wendu = objData.value("wendu").toString();
    mToday.shidu = objData.value("shidu").toString();
    mToday.pm25 = objData.value("pm25").toDouble();
    mToday.quality = objData.value("quality").toString();

    // 5. forecast中的第一个数组元素也是今天的数据
    mToday.type = mDay[1].type;
    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;
    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;

    // 6. 更新UI
    updateUI();

    //更新曲线
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();
}


// 更新UI
void MainWindow::updateUI()
{
    // 日期和城市
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+mDay[1].week);
    ui->lblCity->setText(mToday.city);

    // 更新今天的数据
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTemp->setText(mToday.wendu);
    //ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~" +QString::number(mToday.high) + "°C");

    ui->lblGanMao->setText("感冒指数:" + mToday.ganmao);
    ui->lblWindFx->setText(mToday.fx);
    ui->lblWindFl->setText(mToday.fl);

    ui->lblPM25->setText(QString::number(mToday.pm25));

    ui->lblShiDu->setText(mToday.shidu);
    ui->lblQuality->setText(mToday.quality);

    // 更新六天的数据
    for(int i=0;i<6;i++)
    {
       //日期和时间
       mWeekList[i]->setText("周" + mDay[i].week.right(1));
       ui->lblWeek0->setText("昨天");
       ui->lblWeek1->setText("今天");
       ui->lblWeek2->setText("明天");

       QStringList ymdList = mDay[i].date.split("-");
       mDateList[i]->setText(ymdList[1] + "/" +ymdList[2]);

       //更新天气类型
       mTypeList[i]->setText(mDay[i].type);
       mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);

       //更新空气质量
       if(mDay[i].aqi >= 0 && mDay[i].aqi <= 50)
       {
           mAqiList[i]->setText("优");
           mAqiList[i]->setStyleSheet("background-color: rgb(121,184,0);");
       }else if(mDay[i].aqi >= 50 && mDay[i].aqi <= 100)
       {
           mAqiList[i]->setText("良");
           mAqiList[i]->setStyleSheet("background-color: rgb(255,187,23);");
       }else if(mDay[i].aqi >= 100 && mDay[i].aqi <= 150)
       {
           mAqiList[i]->setText("轻度");
           mAqiList[i]->setStyleSheet("background-color: rgb(255,87,97);");
       }else if(mDay[i].aqi >= 150 && mDay[i].aqi <= 200)
       {
           mAqiList[i]->setText("中度");
           mAqiList[i]->setStyleSheet("background-color: rgb(235,17,27);");
       }else if(mDay[i].aqi >= 200 && mDay[i].aqi <= 250)
       {
           mAqiList[i]->setText("重度");
           mAqiList[i]->setStyleSheet("background-color: rgb(170,0,0);");
       }else
       {
           mAqiList[i]->setText("严重");
           mAqiList[i]->setStyleSheet("background-color: rgb(110,0,0);");
       }

       //更新风力、风向
       mFxList[i]->setText(mDay[i].fx);
       mFlList[i]->setText(mDay[i].fl);
    }
}


//重写父类的eventFilter方法
bool MainWindow::eventFilter(QObject *watched, QEvent *ev)
{
    if(watched == ui->lblHighCurve && ev->type() == QEvent::Paint)
    {
       paintHighCurve();
    }
    if(watched == ui->lblLowCurve && ev->type() == QEvent::Paint)
    {
       paintLowCurve();
    }

    return QWidget::eventFilter(watched,ev);
}


//绘制高低温曲线
void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    // 1. 获取x坐标
    int pointX[6] = {0};
    for(int i=0; i<6; i++)
    {
       //每个控件的中心点就是曲线的x坐标
       pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    // 2. 获取y坐标
    int tempSum = 0;
    int tempAverage = 0;
    for(int i=0;i<6;i++)
    {
       tempSum += mDay[i].high;
    }
    tempAverage = tempSum/6;  //六天最高温的平均值
    //计算y坐标
    int pointY[6] = {0};
    int yCenter = ui->lblHighCurve->height()/2;
    for(int i=0; i<6; i++)
    {
       pointY[i] = yCenter - ((mDay[i].high-tempAverage) * INCREMENT);
    }

    // 3. 开始绘制
    // 3.1 初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);      //设置画笔的宽度
    pen.setColor(QColor(255,170,0)); //设置画笔的颜色

    painter.setPen(pen);
    painter.setBrush(QColor(255,170,0));

    // 3.2 画点、写文本
    for(int i=0;i<6;i++)
    {
       //显示圆点
       painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);

       //显示温度文本
       painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].high) + "°");
    }

    // 3.3 画线
    for(int i=0;i<5;i++)
    {
       if(i == 0)
       {
           pen.setStyle(Qt::DotLine);
           painter.setPen(pen);
       }else {
           pen.setStyle(Qt::SolidLine);
           painter.setPen(pen);
       }

       painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }

}
void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    // 1. 获取x坐标
    int pointX[6] = {0};
    for(int i=0; i<6; i++)
    {
       //每个控件的中心点就是曲线的x坐标
       pointX[i] = mWeekList[i]->pos().x() + mWeekList[i]->width()/2;
    }

    // 2. 获取y坐标
    int tempSum = 0;
    int tempAverage = 0;
    for(int i=0;i<6;i++)
    {
       tempSum += mDay[i].low;
    }
    tempAverage = tempSum/6;  //六天最高温的平均值
    //计算y坐标
    int pointY[6] = {0};
    int yCenter = ui->lblLowCurve->height()/2;
    for(int i=0; i<6; i++)
    {
       pointY[i] = yCenter - ((mDay[i].low-tempAverage) * INCREMENT);
    }

    // 3. 开始绘制
    // 3.1 初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);      //设置画笔的宽度
    pen.setColor(QColor(0,255,255)); //设置画笔的颜色

    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));

    // 3.2 画点、写文本
    for(int i=0;i<6;i++)
    {
       //显示圆点
       painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);

       //显示温度文本
       painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].low) + "°");
    }

    // 3.3 画线
    for(int i=0;i<5;i++)
    {
       if(i == 0)
       {
           pen.setStyle(Qt::DotLine);
           painter.setPen(pen);
       }else {
           pen.setStyle(Qt::SolidLine);
           painter.setPen(pen);
       }

       painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }
}

//天气类型
void MainWindow::weatherType()
{
    //将控件添加到控件数组
    //星期和日期
    mWeekList <<ui->lblWeek0<<ui->lblWeek1<<ui->lblWeek2<<
        ui->lblWeek3<<ui->lblWeek4<<ui->lblWeek5;
    mDateList <<ui->lblDate0<<ui->lblDate1<<ui->lblDate2<<
        ui->lblDate3<<ui->lblDate4<<ui->lblDate5;

    //天气和天气图标
    mTypeList<< ui->lblType0<<ui->lblType1<<ui->lblType2<<
        ui->lblType3<<ui->lblType4<<ui->lblType5;
    mTypeIconList<< ui->lblTypeIcon0<<ui->lblTypeIcon1<<ui->lblTypeIcon2<<
        ui->lblTypeIcon3<<ui->lblTypeIcon4<<ui->lblTypeIcon5;

    //天气指数
    mAqiList<< ui->lblQuality0<<ui->lblQuality1<<ui->lblQuality2<<
        ui->lblQuality3<<ui->lblQuality4<<ui->lblQuality5;

    //风向和风力
    mFlList <<ui->lblFl0<<ui->lblFl1<<ui->lblFl2<<ui->lblFl3<<
        ui->lblFl4<<ui->lblFl5;
    mFxList <<ui->lblFx0<<ui->lblFx1<<ui->lblFx2<<ui->lblFx3<<
        ui->lblFx4<<ui->lblFx5;


    //天气对应的图标
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到暴雪",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到大暴雪",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
}


// 用于接收天气数据,reply中就是请求到的数据
void MainWindow::onReplied(QNetworkReply *reply)
{
    //获取到响应码(如果是200就表示成功)
   int status_code =  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(status_code != 200)
   {
       qDebug()<<reply->errorString().toLatin1().data();
       QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
   }
   else
   {
       QByteArray bytearray =  reply->readAll();
       //qDebug()<<"read all:"<<bytearray.data();
       parseJson(bytearray);
   }

   reply->deleteLater(); //释放内存

}


// 搜索按钮
void MainWindow::on_btnSearch_clicked()
{
   QString cityName = ui->leCity->text();
   getWeatherInfo(cityName);
   ui->leCity->clear();
}


// 判断文本框中是否发生回车事件
void MainWindow::on_leCity_returnPressed()
{
   QString cityName = ui->leCity->text();
   getWeatherInfo(cityName);
   ui->leCity->clear();
}

