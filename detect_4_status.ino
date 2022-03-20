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

#include <Pangodream_18650_CL.h>

#define ADC_PIN 34
#define CONV_FACTOR 1.8
#define READS 20
Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);

int usbValue = 0;
int batteryValue = 0;
int chargerValue = 0;

void setup() {

  Serial.begin(115200);
  lcd.init();
  lcd.setBrightness(128); //back-light
  lcd.setTextFont(4);
  //lcd.setFont(&fonts::Font4);
  drawGradation();

  lcd.setTextColor(0xFFFF00U);
  lcd.setCursor(40, 00);
  lcd.print("Detect 4 status");
  lcd.setCursor(0, 80);
  lcd.print("USB :");
  lcd.setCursor(0, 120);
  lcd.print("Battery :");
  lcd.setCursor(210, 120);
  lcd.print("%");
  lcd.setCursor(0, 160);
  lcd.print("Charger :");
  lcd.setCursor(0, 200);
  lcd.print("Brightness :");

  pinMode(32, INPUT);  //charger
  //pinMode(34, INPUT);  //battery
  pinMode(35, INPUT);  //USN in

}


void loop()
{
  usbValue = digitalRead(35);
  batteryValue = analogRead(34);
  chargerValue = digitalRead(32);

  Serial.println(usbValue);
  Serial.println(batteryValue);
  Serial.println(chargerValue);
  Serial.println("");

  if (usbValue)
  {
    lcd.setBrightness(255);
    lcd.setCursor(75, 80);
    lcd.setTextColor(0x000000U, 0x00FF00U);
    lcd.print("plug in   ");

    lcd.setCursor(150, 200);
    lcd.print("255");
  }
  else
  {
    lcd.setBrightness(80);
    lcd.setCursor(75, 80);
    lcd.setTextColor(0xFFFF00U, 0xFF0000U);
    lcd.print("plug out");

    lcd.setCursor(150, 200);
    lcd.print(" 80  ");
  }

  if (chargerValue)  // 0=charging  , 1=no charging
  {
    lcd.setCursor(120, 160);
    lcd.setTextColor(0xFFFF00U, 0x000000U);
    lcd.print("no              ");
  }
  else
  {
    lcd.setCursor(120, 160);
    lcd.setTextColor(0x000000U, 0x00FF00U);
    lcd.print("charging");
  }

  lcd.setTextColor(0xFFFF00U, 0x000000U);
  lcd.setCursor(105, 120);
  lcd.print(BL.getBatteryVolts());
  lcd.setCursor(180, 120);
  lcd.print(BL.getBatteryChargeLevel());

  delay(1000);
}
