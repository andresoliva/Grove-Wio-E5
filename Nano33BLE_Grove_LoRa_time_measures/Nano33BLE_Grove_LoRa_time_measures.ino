//This a custom version of the Seed example for their Grove LoRa_E5 module
//That works with Seeeduino XIAO, Seeeduino XIAO expantion board and DHT11 Temperature and  Humidity Sensor
//But adaptated for the Arduino Nano 33 BLE Sense connected to a Grove LoRa_E5 module
//Original code; https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/#software-code
//Modified by: Andres Oliva Trevisan
//Working with Arduino 1.8.14 IDE and forward
//--------------------------------------------------
/*Define to print to the USER into UART terminal the commands messages sended and response recieved */
#define LORA_COMMAND_PRINT
/*Define to meassure and print */
#define LORA_COMMAND_TIME_MEASURE
/*Defines "LORA_COMMAND_TIME_MEASURE" dependencies*/
#if (defined(LORA_COMMAND_TIME_MEASURE)&(!defined(LORA_COMMAND_PRINT)))
  #define LORA_COMMAND_PRINT
#endif 
//-----------------------------
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


static int at_send_check_response(char *p_ack, int timeout_ms, char*p_cmd, ...){
  #ifdef LORA_COMMAND_TIME_MEASURE
    int rx_ACK_time=0;
    int tx_msj_time=0;
    char cmd_time[512];
    int  cmd_time_a=0;
    int  cmd_time_c=0;
  #endif 
    int ch;
    int num = 0;
    int i;
    int index = 0;
    int ret_val=0;//init with 0 as default return value
    int startMillis;
    /*parse the arguments and prepare the command to send*/
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    char cmd[255];
    vsnprintf(cmd, sizeof(cmd), p_cmd, args);
    va_end(args);
    //clean the serial port before issuing the command
    while (Serial1.available() > 0){ ch = Serial1.read();}//clean the read buffer
    Serial1.print(cmd); //sends command to Grove LoRa_E5 module
    //---------
    startMillis = millis();// Starts meassuring time after the command was sended
    #ifdef LORA_COMMAND_TIME_MEASURE
    tx_msj_time=millis();//original 0//ToDo: Check if 0 or millis(). check with longer packets because it does not seems to make a differnece
    #endif
    #ifdef LORA_COMMAND_PRINT
    Serial.print(cmd);  // Serial.print(p_cmd, args);
    #endif
    /*ensure a valid p_ack (pointer to command string expected response from the module) was provided*/
    if (p_ack == NULL) { return ret_val;}
    /*Parse the response to the command. Also meassure the time to get the response*/
    while ((millis() - startMillis) < timeout_ms) {
       if (Serial1.available() > 0){ //check if they are characters to be read 
           //we read one character at time because is the only way to get the ack tx time
            ch = Serial1.read();
            recv_buf[index++] = ch;
            //begin with times callculation
            #ifdef LORA_COMMAND_TIME_MEASURE
              //Tx message time
              if (tx_msj_time==0){ //check if command to send message was isued. Then retrieves time
                 if (strstr(recv_buf, "Start") != NULL){ tx_msj_time=millis();}
              }
              if(tx_msj_time>0){
                //if (strstr(recv_buf, "Start") != NULL){
                if (strstr(recv_buf, "Wait ACK") != NULL){
                  tx_msj_time=millis() - tx_msj_time;
                  cmd_time_a=sprintf(&cmd_time[cmd_time_c],"\r\nTime to TX message: %i ms.",tx_msj_time);
                  cmd_time_c+=cmd_time_a;//increase by cmd_time_a
                  tx_msj_time=-1; //indicates the program to stop this parsing 
                }
              }
              //Reception ACK wait time
              if (rx_ACK_time==0){
                if (strstr(recv_buf, "Wait ACK") != NULL){ rx_ACK_time=millis();}
              }
              if(rx_ACK_time>0){
                if (strstr(recv_buf, "ACK Received") != NULL){
                  rx_ACK_time=millis() - rx_ACK_time;
                  cmd_time_a=sprintf(&cmd_time[cmd_time_c],"\r\nTime to RX ACK from TX message: %i ms.",rx_ACK_time);
                  cmd_time_c+=cmd_time_a;//increase by cmd_time_a
                  rx_ACK_time=-1;//indicates the program to stop this parsing 
                }
              }
            #endif     
        //check if the command sended was acknowledged properly
        if (strstr(recv_buf, p_ack) != NULL) {
            ret_val= millis() - startMillis;//returns command execution time
            break;//goes outside of code
          }
       }
      else{//If there are no characters to be read, delays 1 ms and tryes to read again
           delay(1);
           }  
      }/*End of While parsing loop*/    
      #ifdef LORA_COMMAND_PRINT
      for (i = 0; i < index; ++i) {Serial.print(recv_buf[i]);}
      #endif
      #ifdef LORA_COMMAND_TIME_MEASURE
      /*add the time used to print*/
      cmd_time_a=sprintf(&cmd_time[cmd_time_c],"\r\nTotal Command Time + Time to get ACK response: %i ms.",ret_val);
      cmd_time_c+=cmd_time_a;//increase by cmd_time_a
      cmd_time[cmd_time_c] = 0;/*indicates the end of the string*/
      /*print the accumulated message*/
      for (i = 0; i < cmd_time_c; ++i) {Serial.print(cmd_time[i]);}
      #endif
    //end of coe: return cmd elapsed time in ms or 0 if did not work  
    return ret_val;
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
        #ifdef LORA_COMMAND_PRINT
        Serial.println(data);
        #endif
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
        #ifdef LORA_COMMAND_PRINT
        Serial.print("\r\nRSSI:");
        Serial.print(rssi);
        #endif
    }
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
    {
        #ifdef LORA_COMMAND_PRINT
        Serial.print(", SNR :");
        Serial.print(snr);
        #endif
    }
}

void setup(void)
{
    #ifdef LORA_COMMAND_PRINT
    Serial.begin(115200);
    #endif 
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial1.begin(9600);
    #ifdef LORA_COMMAND_PRINT
    Serial.print("\r\nE5 LORAWAN TEST\r\n");
    #endif 
    if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
    {
        is_exist = true;
        at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
        at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");
        at_send_check_response("+DR:", 1000, "AT+DR=1\r\n");//touch here to change Initial data rate. The router will then change it accordingly
        at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");
        at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
        at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
        at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
        delay(200);
        #ifdef LORA_COMMAND_PRINT
        Serial.print("\r\nLoRaWAN");
        #endif
        is_join = true;
    }
    else
    {
        is_exist = false;
        #ifdef LORA_COMMAND_PRINT
        Serial.print("\r\nNo groove E5 module found.\r\n");
        #endif
    }
    digitalWrite(LED_BUILTIN, LOW);
    #ifdef BLE33_HUMIDITY_SENSOR
    if (!HTS.begin()) {
      #ifdef LORA_COMMAND_PRINT
       Serial.println("\r\nFailed to initialize humidity temperature sensor!. Either Your BLE Sense board is an old revision or the sensor is not working");
      #endif
       is_humidity_sensor_ok=false;
    }
    #endif

}

void loop(void)
{
    //setup();//uncomment for getting DevUI id more easily
    float temp =0;
    float humi =0;
  
    #ifdef BLE33_HUMIDITY_SENSOR
    if (is_humidity_sensor_ok){
       temp = HTS.readTemperature();
       humi    = HTS.readHumidity();
    }
    #endif
    #ifdef LORA_COMMAND_PRINT
    Serial.print("\r\nHumidity: ");
    Serial.print(humi);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");
    #endif
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
                #ifdef LORA_COMMAND_PRINT
                Serial.print("\r\nJOIN failed!\r\n");
                #endif
                delay(5000);
            }
        }
        else
        {
            char cmd[128];
            /*Send 4 bytes using selected DR of the BAND selected */
            /*For example: If you choose EU868 band and do not set DR
             * DR used will be 5470 bits per seconds, and radio set up will be SF7/BW 125Khz*/
            //sprintf(cmd, "AT+CMSGHEX=\"%04X%04X\"\r\n", (int)temp, (int)humi);//Send 4 bytes
            sprintf(cmd, "AT+CMSGHEX=\"%04X%04X%04X%04X%04X%04X\"\r\n", (int)temp, (int)humi,(int)temp, (int)humi,(int)temp, (int)humi);
            ret = at_send_check_response("Done", 5000, cmd);
            if (ret>0)
            {
                recv_prase(recv_buf);
            }
            #ifdef LORA_COMMAND_PRINT
            else{
                Serial.print("\r\nSend failed!\r\n\r\n");
            }
            #endif
            delay(5000);
        }
    }
    else
    {
        delay(1000);
    }
}
