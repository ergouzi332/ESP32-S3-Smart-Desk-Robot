#include "max30102.h"
#include <Wire.h>
#include <Arduino.h>

static TwoWire &maxWire = Wire1;//给硬件I2C1起别名maxWire

/*MAX30102的I2C引脚*/
#define MAX30102_I2C_SDA 16
#define MAX30102_I2C_SCL 15

/* MAX30102 寄存器（常用） */
#define REG_PART_ID       0xFF    // 芯片型号ID寄存器（固定值，用于识别是不是MAX30102）
#define REG_REV_ID        0xFE    // 芯片版本ID寄存器（厂家硬件版本号）
#define REG_FIFO_WR_PTR   0x04    // FIFO 写指针（传感器自动往这里写入新数据）
#define REG_FIFO_OVF_COUNTER 0x05 // FIFO 溢出计数器（数据满了没及时读会计数）
#define REG_FIFO_RD_PTR   0x06    // FIFO 读指针（我们从这里读取数据）
#define REG_FIFO_DATA     0x07    // FIFO 数据寄存器（真正的心率/血氧原始数据存在这里）
#define REG_FIFO_CONFIG   0x08    // FIFO 配置寄存器（设置数据采样、滚动模式）
#define REG_MODE_CONFIG   0x09    // 模式配置（设置心率模式/血氧模式/待机）
#define REG_SPO2_CONFIG   0x0A   // 血氧采样配置（ADC精度、采样率、脉冲宽度）
#define REG_LED1_PA       0x0C    // 红外灯(LED1)亮度/电流控制（越大信号越强，也越耗电）
#define REG_LED2_PA       0x0D    // 红光灯(LED2)亮度/电流控制
#define REG_PILOT_PA      0x10    // 环境光抵消/校准灯电流（一般不用改）

/* 全局状态定义 */
MAX30102_Typedef MAX30102_State;
static bool MAX30102_Ready = false;//传感器就绪标志位

/* 红外数据缓存与索引 */
static const int BUFFER_SIZE = 100;		//心率数据缓冲区大小：存储最近100组红外数据
static uint32_t irBuffer[BUFFER_SIZE];
static int irIndex = 0;					//缓冲区索引：记录当前数据该存入数组的第几个位置
static uint32_t lastBeatTime = 0;		//上一次检测到心跳的时间戳

/*向MAX30102的指定寄存器写入一个字节数据*/
static bool writeReg(uint8_t reg, uint8_t val)
{
	maxWire.beginTransmission(MAX30102_I2C_ADDR);
	maxWire.write(reg);
	maxWire.write(val);
	return maxWire.endTransmission() == 0;//返回是否写入成功
}

/*从MAX30102指定寄存器读取1字节数据*/
static int readReg(uint8_t reg)
{
	maxWire.beginTransmission(MAX30102_I2C_ADDR);
	maxWire.write(reg);
	if (maxWire.endTransmission(false) != 0) {
		return -1;
	}
	maxWire.requestFrom((int)MAX30102_I2C_ADDR, (int)1); //向传感器请求读取 1 字节数据
	if (maxWire.available()) return maxWire.read(); //如果有数据可用，就读取并返回
	return -1;
}

/*从MAX30102 FIFO读取一组样本*/
static long readFIFOsample()
{
	/*判断是否初始化*/
	if (!MAX30102_Ready) {
		return -1;
	}
	/*读写指针获取*/
	int writePtr = readReg(REG_FIFO_WR_PTR);//读取FIFO写指针
	int readPtr = readReg(REG_FIFO_RD_PTR);//读取FIFO读指针
	if (writePtr < 0 || readPtr < 0) {
		return -1;
	}
	/*检查传感器缓存里有没有凑够一组完整的心率数据*/
	int available = (writePtr - readPtr + 32) % 32;//计算FIFO中可用的数据数量(环形缓冲区)
	if (available < 2) //1次有效采样=红外光+红光=占用2个FIFO位置
	{
	return -2;
	}
    /*连续读取 6 字节原始采样值*/
	maxWire.beginTransmission(MAX30102_I2C_ADDR);
	maxWire.write(REG_FIFO_DATA);
	int err = maxWire.endTransmission(false);
	if (err != 0) {
		Serial.print("MAX30102: FIFO register write failed, err=");
		Serial.println(err);
		return -1;
	}
	int avail = maxWire.requestFrom((int)MAX30102_I2C_ADDR, (int)6);
	if (avail < 6) {
		Serial.print("MAX30102: FIFO read available=");
		Serial.println(avail);
		return -1;
	}
	/* 拼接红光采样值 */
	long redSample = 0;
	redSample = ((long)maxWire.read() & 0xFF) << 16;// 读第1个字节 → 左移16位 → 放到最高8位位置
	redSample |= ((long)maxWire.read() & 0xFF) << 8;// 读第2个字节 → 左移8位  → 放到中间8位
	redSample |= ((long)maxWire.read() & 0xFF);		// 读第3个字节 → 直接放    → 最低8位
	redSample &= 0x3FFFF;							// 只保留18位有效数据（MAX30102是18位ADC）
	/* 拼接红外采样值 */
	long irSample = 0;
	irSample = ((long)maxWire.read() & 0xFF) << 16;//红外1
	irSample |= ((long)maxWire.read() & 0xFF) << 8;//红外2
	irSample |= ((long)maxWire.read() & 0xFF);	   //红外3
	irSample &= 0x3FFFF;

	if (irSample == 0 || irSample == 0x3FFFF) {
		// 如果第二个样本无效，尝试将第一组样本视为 IR
		if (redSample != 0 && redSample != 0x3FFFF) {
			return redSample;
		}
	}
	return irSample;//返回红外
}

/*初始化MAX30102*/
void MAX30102_Init(void)
{
	MAX30102_Ready = false;
	/*启动I2C总线，指定SDA/SCL引脚*/
	maxWire.begin(MAX30102_I2C_SDA, MAX30102_I2C_SCL);
	maxWire.setClock(100000);
	/*清空状态变量*/
	MAX30102_State.heartRate = 0;
	MAX30102_State.spo2 = 0;
	MAX30102_State.valid = false;
	irIndex = 0;
	/*读取传感器芯片ID，检查是否在线*/
	int partId = readReg(REG_PART_ID);
	if (partId < 0) {
		Serial.println("MAX30102: I2C device not found; check wiring and address.");
		MAX30102_Ready = false;
		return;
	}
	MAX30102_Ready = true;

	/*软复位（重启传感器）*/
	writeReg(REG_MODE_CONFIG, 0x40);
	delay(100);
	/*FIFO 配置：允许滚动覆盖，不做数据平均*/
	writeReg(REG_FIFO_CONFIG, 0x1F);
	/*设置 LED 亮度（电流）*/
	writeReg(REG_LED1_PA, 0x20);//红灯电流
	writeReg(REG_LED2_PA, 0x7F); //红外灯电流（更强）
	/*SpO2 配置：18位精度，100Hz 采样率*/
	writeReg(REG_SPO2_CONFIG, 0x27);
	/*设置工作模式：心率 + 血氧模式 (0x02)*/
	writeReg(REG_MODE_CONFIG, 0x02);
	/*等待传感器启动采样*/
	delay(200);

	/*清空FIFO读写指针*/
	writeReg(REG_FIFO_WR_PTR, 0);
	writeReg(REG_FIFO_OVF_COUNTER, 0);
	writeReg(REG_FIFO_RD_PTR, 0);
	delay(200);
	/*检查初始化是否正常*/
	int wrPtr = readReg(REG_FIFO_WR_PTR);
	int rdPtr = readReg(REG_FIFO_RD_PTR);
	/*如果 FIFO 仍然为空，重试一次更强的配置*/
	if (wrPtr == 0 && rdPtr == 0) {
		// 备用配置：全亮 LED 并尝试模式 0x03
		writeReg(REG_LED1_PA, 0x7F);	//红灯开到最大
		writeReg(REG_LED2_PA, 0x7F);	//红外灯开到最大
		writeReg(REG_MODE_CONFIG, 0x03); //切换到模式
		delay(500);
	}
}

/*判断是否是心跳*/
static bool detectBeat(long value)
{
	/*计算最近20个历史数据的平均值*/
	uint32_t sum = 0;
	int cnt = 0;
	for (int i = 1; i <= 20; i++) {
		int j = (irIndex - 1 - i + BUFFER_SIZE) % BUFFER_SIZE;
		sum += irBuffer[j];
		cnt++;
	}
	uint32_t avg = (cnt > 0) ? sum / cnt : 0;

	static bool above = false;//记录当前是否在阈值上方
	static long prevValue = 0;//存上一次的数值，用于计算上升幅度
	const uint32_t threshold = avg + 40; // 阈值 = 平均值 + 固定偏移
	const uint32_t minRise = 18;         // 最小上升幅度，避免抖动误触发
	uint32_t rise = (value > prevValue) ? (uint32_t)(value - prevValue) : 0;// 计算当前值比上一次上升了多少
	// 条件1：当前值 > 平均值阈值
	// 条件2：数值快速上升
	// 条件3：上一次不在峰值区（防止重复触发）
	if (value > threshold && rise > minRise) {
		if (!above) {
			above = true;//标记进入心跳峰值
			prevValue = value;//保存当前值
			return true;//检测到一次有效心跳
		}
	} else {
		above = false; //低于阈值，标记为波谷
	}
	prevValue = value;	//更新上一次数值
	return false;//未检测到心跳
}
/*定时读取数据 → 存入缓冲区 → 检测心跳 → 计算心率 BPM*/
void readMAX30102(void)
{
	static unsigned long lastReadMillis = 0;
	const unsigned long minInterval = 25;//每25ms读一次
	unsigned long nowMs = millis();
	if (nowMs - lastReadMillis < minInterval) {
		return;
	}
	lastReadMillis = nowMs;
	/*传感器未就绪，直接退出*/
	if (!MAX30102_Ready) {
		static bool warned = false;
		if (!warned) {
			warned = true;
		}
		MAX30102_State.valid = false;
		return;
	}
	/*读取一组红外采样值*/
	long sample = readFIFOsample();
	if (sample == -2 || sample < 0) {
		MAX30102_State.valid = false;
		return;
	}
	/*无效值过滤*/
	if (sample == 0x3FFFF || sample == 0) {
		MAX30102_State.valid = false;
		return;
	}
    /*把采样值存入环形缓冲区*/
	irBuffer[irIndex++] = (uint32_t)sample;
	if (irIndex >= BUFFER_SIZE) irIndex = 0;

	/*拿到最新的红外值*/
	uint32_t irVal = irBuffer[(irIndex - 1 + BUFFER_SIZE) % BUFFER_SIZE];
	/*判断是否有手指（信号太弱 = 没手指）*/
	if (irVal < 30000) {
		MAX30102_State.valid = false;
		return;
	}
	/*计算BPM*/
	if (detectBeat(irVal)) {
		uint32_t now = millis();
		if (lastBeatTime != 0) {
			uint32_t delta = now - lastBeatTime;// 两次心跳间隔时间(ms)
			if (delta > 300 && delta < 3000) // 合法心率范围：300ms ~ 3000ms
			{
				int bpm = (int)(60000.0 / delta);
				/*4次平滑滤波，让心率更稳定*/
				static int prevBpm[4] = {0,0,0,0};
				static int spot = 0;
				prevBpm[spot++] = bpm;
				spot %= 4;
				int s = 0; for (int i = 0; i < 4; i++) s += prevBpm[i];
				int avgBpm = s / 4;
				MAX30102_State.heartRate = avgBpm;
				MAX30102_State.valid = true;
			}
		}
		lastBeatTime = now;
	}
}


