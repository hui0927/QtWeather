# QtWeather
桌面天气预报(基于Qt5,代码结构清晰并含有详细注释)

## :star:项目描述
基于Qt5.15.2/C++的一个桌面版天气预报,实现了全国各地城市六天的天气预警(温湿度、污染指数、空气质量、风向),并打包完成 可以下载Hui_weather.exe查看完整效果。
![image](https://github.com/hui0927/QtWeather/assets/117810372/fde6cf58-5b96-4d7d-8ce7-f61741c60dd4)


## :airplane:项目技术点
* 重写鼠标事件,实现窗口跟随鼠标移动、单击右键退出
* 自行绘制温度曲线,使温度趋势更加清晰明了
* 使用事件过滤器拦截并修改事件处理逻辑
* 使用QNetwork发送HTTP请求 获得天气数据
* 使用QJsonDocument解析JSON、QFile读写JSON数据
* 使用信号槽实现对象间的通信和交互,实现用户搜索后显示数据到界面

## :computer:QT项目发布打包参考
https://subingwen.cn/qt/qt-setup/
