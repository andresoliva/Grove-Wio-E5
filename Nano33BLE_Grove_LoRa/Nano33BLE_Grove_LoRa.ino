//This a custom version of the Seed example for their Grove LoRa_E5 module
//That works with Seeeduino XIAO, Seeeduino XIAO expantion board and DHT11 Temperature and  Humidity Sensor
//But adaptated for the Arduino Nano 33 BLE Sense connected to a Grove LoRa_E5 module
//Original code; https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-code
//Modified by: Andres Oliva Trevisan
//Working with Arduino 1.8.14 IDE and forward
//--------------------------------------------------
#include <Arduino.h>
#include <stdarg.h> //for using vsnprintf

#define BLE33_HUMIDITY_SENSOR //Tell the code we are using a sensor
#ifdef BLE33_HUMIDITY_SENSOR
#include <Arduino_HTS221.h>//For accesing the Humidty sensor
static bool is_humidity_sensor_ok=true; //For compatibility with the older revision of the board that does not have the sensor
#endif

static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;

static int at_send_check_response(char *p_ack, int timeout_ms, char*p_cmd, ...)
{
    int ch;
    int num = 0;
    int index = 0;
    int startMillis;
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    char cmd[255];
    vsnprintf(cmd, sizeof(cmd), p_cmd, args);
    Serial1.print(cmd); //Serial1.printf(p_cmd, args);
    Serial.print(cmd);  // Serial.print(p_cmd, args);
    va_end(args);
    delay(200);
    if (p_ack == NULL)
    {
        return 0;
    }
    startMillis = millis();//
    do
    {
        while (Serial1.available() > 0)
        {
            ch = Serial1.read();
            recv_buf[index++] = ch;
            Serial.print((char)ch);
            delay(2);  
        } 
        if (strstr(recv_buf, p_ack) != NULL)
           {
            return (1);
        }

    } while (millis() - startMillis < timeout_ms);
    return 0;
}

static void recv_prase(char *p_msg)
{
    if (p_msg == NULL)
    {
        return;
    }
char*p_start = NULL;
    int data = 0;
    int rssi = 0;
    int snr = 0;

    p_start = strstr(p_msg, "RX");
    if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data)))
    {
        Serial.println(data);
        led = !!data;
        if (led)
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }

    p_start = strstr(p_msg, "RSSI");
    if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi)))
    {
        Serial.print("rssi:");
        Serial.print(rssi);
    }
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
    {
        Serial.print("snr :");
        Serial.print(snr);
    }
}

void setup(void)
{

    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial1.begin(9600);
    Serial.print("E5 LORAWAN TEST\r\n");

    if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
    {
        is_exist = true;
        at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
        at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");
        at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");
        at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
        at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
        at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
        delay(200);
        Serial.print("LoRaWAN");
        is_join = true;
    }
    else
    {
        is_exist = false;
        Serial.print("No E5 module found.\r\n");
    }
    digitalWrite(LED_BUILTIN, LOW);
    #ifdef BLE33_HUMIDITY_SENSOR
    if (!HTS.begin()) {
       Serial.println("Failed to initialize humidity temperature sensor!. Either Your BLE Sense board is an old revision or the sensor is not working");
       is_humidity_sensor_ok=false;
    }
    #endif

}

void loop(void)
{
    //setup();//uncomment for getting DevUI id more easily
    float temp = 0;
    float humi = 0;
  
    #ifdef BLE33_HUMIDITY_SENSOR
    if (is_humidity_sensor_ok){
       temp = HTS.readTemperature();
       humi    = HTS.readHumidity();
    }
    #endif
    Serial.println("Humidity: ");
    Serial.print(humi);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");

    if (is_exist)
    {
        int ret = 0;
        if (is_join)
        {

            ret = at_send_check_response("+JOIN: Network joined", 12000, "AT+JOIN\r\n");
            if (ret)
            {
                is_join = false;
            }
            else
            {
                at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
                Serial.print("JOIN failed!\r\n\r\n");
                delay(5000);
            }
        }
        else
        {
            char cmd[128];
            /*Send 4 bytes (32 bits) using default DR (5470 bits per seconds) of the EU868 band(SF7/BW 125Khz) */
            //sprintf(cmd, "AT+CMSGHEX=\"%04X%04X\"\r\n", (int)temp, (int)humi);
            ret = at_send_check_response("Done", 5000, cmd);
            if (ret)
            {
                recv_prase(recv_buf);
            }
            else
            {
                Serial.print("Send failed!\r\n\r\n");
            }
            delay(5000);
        }
    }
    else
    {
        delay(1000);
    }
}
