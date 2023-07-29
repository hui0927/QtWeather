#ifndef WEATHERDATA_H
#define WEATHERDATA_H
#include <QString>

// 存放今天的天气数据
class Today
{
public:
    Today()
    {
        date = "2023-7-28";
        city = "河南";
        ganmao = "感冒指数";
        wendu =  " ";
        shidu = "0%";
        pm25 = 0;
        quality = "无数据";
        type = "多云";
        fl = "2级";
        fx = "南风";
        high = 30;
        low = 18;

    }

    QString date;
    QString city;
    QString ganmao;
    QString wendu;
    QString shidu;
    int pm25;
    QString quality;
    QString type;
    QString fl;
    QString fx;
    int high;
    int low;
};

// 存放未来六天的天气数据
class Day
{
public:
    Day()
    {
        date = "2023-7-28";
        week = "周五";
        type = "多云";
        high = 0;
        low = 0;
        fx = "南风";
        fl = "2级";
        aqi = 0;
    }

    QString date;
    QString week;
    QString type;
    int high;
    int low;
    QString fx;
    QString fl;
    int aqi;
};




#endif // WEATHERDATA_H
