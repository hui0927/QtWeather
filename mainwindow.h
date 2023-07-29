#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "weatherdata.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    //重写事件，实现单击鼠标右键退出程序
    void contextMenuEvent(QContextMenuEvent* event);
    //获取鼠标按下/移动 时的位置
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    //用于请求天气数据
    void getWeatherInfo(QString cityCode);

    //解析天气数据
    void parseJson(QByteArray& byteArray);

    //更新UI
    void updateUI();

    //重写父类的eventFilter方法
    bool eventFilter(QObject *watched, QEvent *ev);

    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();

    //天气类型
    void weatherType();

private slots:
    void on_btnSearch_clicked();

    void on_leCity_returnPressed();

private:
    //处理天气数据
    void onReplied(QNetworkReply* reply);


private:
    Ui::MainWindow* ui;

    QMenu* mExitMenu;   // 右键退出的菜单
    QAction* mExitAct;  // 退出的行为
    QPoint mOffset;     // 窗口移动时，鼠标与窗口左上角的偏移
    QNetworkAccessManager* mNetAccessManager; //用于网络请求

    Today mToday; //当天的天气数据
    Day mDay[6];  //未来六天的天气数据

    //控件数组,用于更新UI
    //星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;
    //天气和天气图标
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;
    //天气污染指数
    QList<QLabel*> mAqiList;
    //风力和风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;


    QMap<QString,QString> mTypeMap;

};
#endif  // MAINWINDOW_H



