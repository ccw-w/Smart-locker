import * as echarts from '../../ec-canvas/echarts';
const util = require('../../utils/util.js');
Page({
  data: {
    alertShown: false,
    tempThreshold: 30, // 默认温度阈值
    humiThreshold: 90, // 默认湿度阈值
    temp: 'N/A',
    humi: 'N/A',
    flameDetected: false,
    lastFlameTime: 'N/A',
    temperatureHumidityIntervalId: null,
    ec: {
      onInit: function (canvas, width, height, dpr) {
        const chart = echarts.init(canvas, null, {
          width: width,
          height: height,
          devicePixelRatio: dpr
        });
        canvas.setChart(chart);

        const option = {
          title: {
            text: '温湿度变化'
          },
          tooltip: {
            trigger: 'axis'
          },
          legend: {
            data: ['温度', '湿度']
          },
          xAxis: {
            type: 'category',
            boundaryGap: false,
            data: []
          },
          yAxis: {
            type: 'value'
          },
          series: [
            {
              name: '温度',
              type: 'line',
              data: []
            },
            {
              name: '湿度',
              type: 'line',
              data: []
            }
          ]
        };

        chart.setOption(option);
        return chart;
      }
    },
    tempData: [],
    humiData: [],
    times: [],
    date: new Date().toISOString().slice(0, 10),
    time: new Date().toLocaleTimeString(),
    weatherStatus: '加载中...',
    currentLocation: '加载中...'
  },

  onLoad: function () {
    console.log('页面加载');
    this.get_info();
    this.startTemperatureHumidityUpdate();
    this.getWeatherInfo();
    this.getCurrentTime();
  },

  onUnload: function () {
    this.stopTemperatureHumidityUpdate();
  },

  startTemperatureHumidityUpdate() {
    this.temperatureHumidityIntervalId = setInterval(() => {
      this.get_info();
    }, 3500);
  },

  stopTemperatureHumidityUpdate() {
    if (this.temperatureHumidityIntervalId) {
      clearInterval(this.temperatureHumidityIntervalId);
    }
  },

  restartTemperatureHumidityUpdate() {
    this.stopTemperatureHumidityUpdate();
    this.startTemperatureHumidityUpdate();
  },

  get_info() {
  console.log('获取设备信息');
  wx.request({
    url: "https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=oay0gSchgn&device_name=cabinet",
    header: {
      "authorization": "version=2018-10-31&res=products%2Foay0gSchgn%2Fdevices%2Fcabinet&et=1916285142&method=md5&sign=iE9rAZ8KU8zpHjSLQDvPUg%3D%3D"
    },
    method: "GET",
    success: res => {
    this.getCurrentTime();
      console.log('请求成功', res);
      const dataMap = res.data.data.reduce((acc, item) => {
        acc[item.identifier] = item.value;
        return acc;
      }, {});

      // 更新温湿度，保留一位小数并补零
      const temp = this.formatTemperature(dataMap.temp);
      const humi = this.formatTemperature(dataMap.humi);
      const flameDetected = dataMap.fire === 'true';
      // 检查是否有火焰，或者温度湿度是否超过新的阈值
    if (parseFloat(temp) > this.data.tempThreshold || parseFloat(humi) > this.data.humiThreshold) {
        if (!this.data.alertShown) {
          //一秒发送一次
          for (let i = 0; i < 4; i++) {
            setTimeout(() => {
              this.sendCommandToDevice('fan', true); // 发送开启风扇命令
            }, i * 1000);
          }
          this.showAlert('警告', `当前环境不安全！已自动打开风扇。稍后请手动关闭！温度：${temp}°C，湿度：${humi}%`);
          this.setData({ alertShown: true });
        }
      } else {
        // 温湿度恢复正常，重置警报状态
        this.setData({ alertShown: false });
      }
       // 检查是否有火焰，或者温度湿度是否超过新的阈值
    if (flameDetected) {
          this.showAlert('警告', '检测到火焰！');
        }

      // 记录上一次检测到火焰的时间
      if (flameDetected) {
        this.setData({
          lastFlameTimestamp: new Date().getTime()
        });
      }
      // 计算从上次检测到火焰到现在的时间差
      let lastFlameTime = '无火焰';
      if (this.data.lastFlameTimestamp) {
        const currentTime = new Date().getTime();
        const timeDiff = currentTime - this.data.lastFlameTimestamp;
        lastFlameTime = this.formatDuration(timeDiff);
      }

      this.setData({
        temp: temp,
        humi: humi,
        flameDetected: flameDetected,
        lastFlameTime: lastFlameTime
      });

      // 更新图表数据
      this.updateChartData(temp, humi);
    },
    fail: err => {
      console.error('请求失败', err);
    }
  });
},

formatTemperature(value) {
    if (value === undefined || value === null) return '无火焰';
    const num = parseFloat(value);
    if (isNaN(num)) return '无火焰',this.setData({ alertShown: false });;
    return num.toFixed(1);
  },

  formatDuration(milliseconds) {
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);

    const remainingSeconds = seconds % 60;
    const remainingMinutes = minutes % 60;
    const remainingHours = hours % 24;

    if (days > 0) {
      return `${days}天${remainingHours}小时${remainingMinutes}分钟`;
    } else if (hours > 0) {
      return `${hours}小时${remainingMinutes}分钟`;
    } else if (minutes > 0) {
      return `${minutes}分钟${remainingSeconds}秒`;
    } else {
      return `${seconds}秒`;
    }
  },
  onTempThresholdChange(e) {
    this.setData({
      tempThreshold: e.detail.value
    });
  },

  onHumiThresholdChange(e) {
    this.setData({
      humiThreshold: e.detail.value
    });
  },

  updateChartData(temp, humi) {
    const time = new Date().toLocaleTimeString();
    const tempData = this.data.tempData.slice();
    const humiData = this.data.humiData.slice();
    const times = this.data.times.slice();

    // 添加新数据
    tempData.push(temp);
    humiData.push(humi);
    times.push(time);

    // 如果数据过多，移除最早的数据点
    if (tempData.length > 20) {
      tempData.shift();
      humiData.shift();
      times.shift();
    }

    // 更新数据
    this.setData({
      tempData: tempData,
      humiData: humiData,
      times: times
    });

    // 更新图表
    const chart = this.selectComponent('#mychart-dom-bar').chart;
    if (chart) {
      chart.setOption({
        xAxis: {
          data: times
        },
        series: [
          {
            name: '温度',
            data: tempData
          },
          {
            name: '湿度',
            data: humiData
          }
        ]
      });
    } else {
      console.warn('Chart component not initialized yet. Retrying...');
      setTimeout(() => {
        this.updateChartData(temp, humi);
      }, 500);
    }
  },

  bindDateChange(e) {
    this.setData({
      date: e.detail.value
    });
  },

  bindTimeChange(e) {
    this.setData({
      time: e.detail.value
    });
  },
  showAlert(title, content) {
    wx.showModal({
      title: title,
      content: content,
      showCancel: false, // 不显示取消按钮
      confirmText: '确定',
      success: function(res) {
        if (res.confirm) {
          console.log('用户点击了确定');
        }
      }
    });
  },

  getWeatherInfo(retries = 3, delay = 10000) {
    wx.request({
      url: "https://cn.apihz.cn/api/tianqi/tqyb.php?id=88888888&key=88888888&sheng=江苏&place=苏州",
      method: "GET",
      success: res => {
        console.log('天气信息获取成功', res);
        const weatherData = res.data;
        if (weatherData && weatherData.place) {
          const currentLocation = weatherData.place.split(',')[2]; // 只取城市名
          this.setData({
            weatherStatus: weatherData.windDirection,
            currentLocation: currentLocation
          });
        } else {
          console.error('天气数据格式错误', weatherData);
          if (retries > 0) {
            setTimeout(() => this.getWeatherInfo(retries - 1, delay), delay);
          }
        }
      },
      fail: err => {
        console.error('天气信息获取失败', err);
        if (retries > 0) {
          setTimeout(() => this.getWeatherInfo(retries - 1, delay), delay);
        }
      }
    });
  },
  sendCommandToDevice(device, value, retries = 3) {

    console.log(`发送命令给设备 ${device}，值为 ${value}`);

    wx.request({
      url: 'https://iot-api.heclouds.com/thingmodel/set-device-desired-property',
      method: 'POST',
      header: {
        "authorization": "version=2018-10-31&res=products%2Foay0gSchgn%2Fdevices%2Fcabinet&et=1916285142&method=md5&sign=iE9rAZ8KU8zpHjSLQDvPUg%3D%3D"
      },
      data: JSON.stringify({
        "product_id": "oay0gSchgn",
        "device_name": "cabinet",
        "params": { [device]: value }
      }),
      success: res => {
        console.log('命令发送成功', res);
        if (res.statusCode === 200 && res.data.code === 0) {
          setTimeout(() => {
            this.get_info(); // 获取设备状态以检查命令是否成功
          }, 2000);
        } else {
          console.error('命令发送失败，状态码:', res.statusCode, '错误码:', res.data.code, '错误信息:', res.data.message);
          this.setData({ isSendingCommand: false });
          if (retries > 0) {
            setTimeout(() => this.sendCommandToDevice(device, value, retries - 1), 1000);
          }
        }
      },
      fail: err => {
        console.error('命令发送失败', err);
        this.setData({ isSendingCommand: false });
        if (retries > 0) {
          setTimeout(() => this.sendCommandToDevice(device, value, retries - 1), 1000);
        }
      }
    });
  },
  getCurrentTime: function () {
    const now = new Date();
    this.setData({
      currentTime: util.formatTime(now)
    });
  }
});