Page({
    data: {
      one: false,
      two: false,
      three: false,
      fan: false,
      isSendingCommand: false, // 控制按钮状态
      lastCommandDevice: '',
      lastCommandValue: false,
      commandRetries: 3
    },
  
    onLoad: function () {
      console.log('控制页面加载');
      this.get_info();
      // 每隔1秒自动刷新设备状态
    this.intervalId = setInterval(() => {
        this.get_info();
      }, 1000);
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
          console.log('请求成功', res);
          const dataMap = res.data.data.reduce((acc, item) => {
            acc[item.identifier] = item.value;
            return acc;
          }, {});
  
          this.setData({
            one: this.formatBoolean(dataMap.one),
            two: this.formatBoolean(dataMap.two),
            three: this.formatBoolean(dataMap.three),
            fan: this.formatBoolean(dataMap.fan)
          });
  
          // 如果是在发送命令后获取信息，检查命令是否成功
          if (this.data.isSendingCommand) {
            this.checkCommandSuccess(dataMap);
          }
        },
        fail: err => {
          console.error('请求失败', err);
        }
      });
    },
  
    formatBoolean(value) {
      return value === "true";
    },
  
    sendCommandToDevice(device, value, retries = 3) {
      if (this.data.isSendingCommand) return;
  
      console.log(`发送命令给设备 ${device}，值为 ${value}`);
      this.setData({ isSendingCommand: true });
  
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
  
    checkCommandSuccess(dataMap) {
      const device = this.data.lastCommandDevice;
      const value = this.data.lastCommandValue;
  
      if (dataMap[device] === (value ? "true" : "false")) {
        console.log('命令发送成功');
      } else {
        // 如果命令未成功，继续重试
        if (this.data.commandRetries > 0) {
          setTimeout(() => this.sendCommandToDevice(device, value, this.data.commandRetries - 1), 40);
        }
      }
  
      this.setData({ isSendingCommand: false });
    },
  
    onDeviceChange(e) {
      const device = e.currentTarget.dataset.device;
      const value = e.detail.value;
      console.log(`设备 ${device} 的状态改变为 ${value}`);
  
      wx.showToast({
        title: `${value ? '开启' : '关闭'} ${device}`,
        icon: 'none',
        duration: 2000
      });
  
      this.setData({
        lastCommandDevice: device,
        lastCommandValue: value,
        commandRetries: 4, // 重试次数
        [device]: value
      }, () => {
        this.sendCommandToDevice(device, value);
      });
    },
  });