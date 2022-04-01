// install this library  https://github.com/tanakamasayuki/I2C_BM8563
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789     _panel_instance;
    lgfx::Bus_SPI       _bus_instance;   // SPIバスのインスタンス
    lgfx::Light_PWM     _light_instance;

  public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

        cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  (VSPI_HOST or HSPI_HOST)
        cfg.spi_mode = 0;             // SPI通信モードを設定 (0 ~ 3)
        cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
        cfg.freq_read  = 16000000;    // 受信時のSPIクロック
        cfg.spi_3wire  = false;        // 受信をMOSIピンで行う場合はtrueを設定
        cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
        cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)   使用するDMAチャンネルを設定 (0=DMA不使用)
        cfg.pin_sclk = 18;            // SPIのSCLKピン番号を設定
        cfg.pin_mosi = 23;            // SPIのMOSIピン番号を設定
        cfg.pin_miso = -1;            // SPIのMISOピン番号を設定 (-1 = disable)
        cfg.pin_dc   = 14;            // SPIのD/Cピン番号を設定  (-1 = disable)

        _bus_instance.config(cfg);    // 設定値をバスに反映します。
        _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
      }

      {
        auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。
        cfg.pin_cs           =    4;  // CSが接続されているピン番号   (-1 = disable)
        cfg.pin_rst          =    12;  // RSTが接続されているピン番号  (-1 = disable)
        cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

        cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
        cfg.memory_height    =   280;  // ドライバICがサポートしている最大の高さ
        cfg.panel_width      =   240;  // 実際に表示可能な幅
        cfg.panel_height     =   280;  // 実際に表示可能な高さ
        cfg.offset_x         =     0;  // パネルのX方向オフセット量
        cfg.offset_y         =     20;  // パネルのY方向オフセット量
        cfg.offset_rotation  =     0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
        cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
        cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
        cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
        cfg.invert           =  true;  // パネルの明暗が反転してしまう場合 trueに設定
        cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
        cfg.dlen_16bit       = false;  // データ長を16bit単位で送信するパネルの場合 trueに設定
        cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

        _panel_instance.config(cfg);
      }

      {
        auto cfg = _light_instance.config();    // バックライト設定用の構造体を取得します。

        cfg.pin_bl = 33;              // バックライトが接続されているピン番号
        cfg.invert = false;           // バックライトの輝度を反転させる場合 true
        cfg.freq   = 44100;           // バックライトのPWM周波数
        cfg.pwm_channel = 7;          // 使用するPWMのチャンネル番号

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
      }

      setPanel(&_panel_instance); // 使用するパネルをセットします。
    }
};
static LGFX lcd;

void drawGradation(void)
{
  // 背景にグラデーションを描画する
  lcd.startWrite();
  lcd.setAddrWindow(0, 0, lcd.width(), lcd.height());
  for (int y = 0; y < lcd.height(); ++y) {
    for (int x = 0; x < lcd.width(); ++x) {
      lcd.writeColor(lcd.color888(x >> 1, (x + y) >> 2, y >> 1), 1);
    }
  }
  lcd.endWrite();
}

#include "I2C_BM8563.h"

// RTC BM8563 I2C port

// I2C pin definition for M5Stick & M5Stick Plus & M5Stack Core2
#define BM8563_I2C_SDA 21
#define BM8563_I2C_SCL 22

// I2C pin definition for M5Stack TimerCam
// #define BM8563_I2C_SDA 12
// #define BM8563_I2C_SCL 14

I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire1);

int YEAR;
int MONTH;
int DATE;
int HOUR;
int MINUTE;
int SECOND;

void setup() {
  // Init Serial
  Serial.begin(115200);
  delay(50);

  // Init I2C
  Wire1.begin(BM8563_I2C_SDA, BM8563_I2C_SCL);

  // Init RTC
  rtc.begin();

  lcd.init();
  lcd.setBrightness(128); //back-light
  lcd.setTextFont(4);
  drawGradation();

  lcd.setTextColor(0xFFFF00U);
  lcd.setCursor(40, 10);
  lcd.print("PCF8563 RTC");

}

void loop() {
  I2C_BM8563_DateTypeDef dateStruct;
  I2C_BM8563_TimeTypeDef timeStruct;

  // Get RTC
  rtc.getDate(&dateStruct);
  rtc.getTime(&timeStruct);

  // Print RTC
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                dateStruct.year,
                dateStruct.month,
                dateStruct.date,
                timeStruct.hours,
                timeStruct.minutes,
                timeStruct.seconds
               );

  YEAR = dateStruct.year;
  MONTH = dateStruct.month;
  DATE = dateStruct.date;
  lcd.setCursor(50, 50);
  lcd.setTextFont(4);
  lcd.setTextColor(0xFFFF00U, 0x000000U);
  lcd.printf("%04d/%02d/%02d", YEAR , MONTH , DATE);

  HOUR = timeStruct.hours;
  MINUTE = timeStruct.minutes;
  SECOND = timeStruct.seconds;
  lcd.setCursor(10, 120);
  lcd.setTextFont(7);
  lcd.printf("%02d:%02d:%02d", HOUR, MINUTE, SECOND);

  // Wait
  delay(1000);

  if (millis() > 6000)
  {
    esp_deep_sleep_start();
  }

}
