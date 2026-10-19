// ADS Touch Sensor Test Example Program (IC P/N:ANMG08, ANMG04K )
// Code:
// Date: 2026.10.16  Ver.: 0.0.1
// H/W Target: ARDUINO UNO R3, S/W: Arduino IDE  1.8.19
// Author: Park, Byoungbae (yni2yni@hanmail.net)
// Note: More information? Please ,send to e-mail.
// Uno R3, A4:SDA,A5: SCL, Leonardo 2:SDA,3:SCL, Nano V3.0  A4:SDA,A5:SCL
// Register setting values are subject to change without prior notice to improve touch operation.

#include <Wire.h>

#define LF        0x0A //New Line
#define CR        0x0D //Carriage  return
#define SPC       0x20 //Space

#define Ch_enable    0x01 //Touch Channel Enable/Disable
#define Check_Reset  0x04 //Chip Reset Check Register
#define Global_Ctrl0 0x05 //Global Control Register 0

#define CRC16_Ctrl   0x23 //Clear CRC16
#define CRC16_Wdata1 0x24 //CRC16 WDATA1
#define CRC16_Wdata2 0x25 //CRC16 WDATA2
#define CRC16_Rdata1 0x26 //CRC16 RDATA1
#define CRC16_Rdata2 0x27 //CRC16 RDATA2

#define Output          0x2A //Touch Key Output Data Register
#define Checksum_Ro     0x2B //Output Checksum data
//#define Checksum_Ctrl   0xC9 //Touch Key Output Checksum Control

#define Clock_Ctrl   0x34 //Clock Control Register (clk_sel, rb_sel)
#define Global_Ctrl1 0x36 //Global Option Control Register1
                          //(response_off_ctrl, response_ctrl, bf_mode, Software Reet)
#define Pre_Scaler   0x37 //Cal_pre_scale
#define Global_Ctrl2 0x38 //Global Option Control Register2
                          //(imp_sel,Single/Multi ,Cal_Hold_time,clock_off)

// Sensitivity level (threshold, Register Value X 0.025% = (1 Step=0.025%)							
#define Sensitivity1  0x39 //ch1,Default: 0x1C X 0.025% = 0.70% (threshold)
#define Sensitivity2  0x3A //ch2
#define Sensitivity3  0x3B //ch3
#define Sensitivity4  0x3C //ch4
#define Sensitivity5  0x3D //ch5
#define Sensitivity6  0x3E //ch6
#define Sensitivity7  0x3F //ch7
#define Sensitivity8  0x40 //ch8
#define Hys_Sensitivity    0x49 // Hysteresis level
#define Hys_Levle_Opt      0x4A // Hysteresis levle Option Register

#define Cal_Speed1   0x41 //Calibration Speed Control at BF mode
#define Cal_Speed2   0x42 //Calibration Speed Control at BS mode

#define PWM_Ctrl1    0x43 //LED PWM control Register (D2,D1)
#define PWM_Ctrl2    0x44 //LED PWM control Register (D4,D3)
#define PWM_Ctrl3    0x45 //LED PWM control Register (D6,D5)
#define PWM_Ctrl4    0x46 //LED PWM control Register (D8,D7)
#define Port_Mode    0x47 //Select the output port operation mode of each channel.

#define  Reg_Unlock     0x65 //Register Lock/unlock 
#define  BIST_Unlock    0xA9
//BIST(Buli-In Self Test) Register address 0x6Eh ~0x77h,0xE3h~0xEFh Lock/unlock

#define Dtr_Ctrl3       0x6D //DTR Mode
#define Alarm3          0x8B //DTR Status 
#define DTR_ICtrl1      0xD9 //DTR Enable, DTR Value
#define DTR_ICtrl2      0xDA //DTR Enable, DTR Value
#define DTR_ICtrl3      0xDB //DTR Enable, DTR Value
#define DTR_ICtrl4      0xDC //DTR Enable, DTR Value
#define DTR_ICtrl5      0xDD //DTR Enable, DTR Value
#define DTR_ICtrl6      0xDE //DTR Enable, DTR Value
#define DTR_ICtrl7      0xDF //DTR Enable, DTR Value
#define DTR_ICtrl8      0xE0 //DTR Enable, DTR Value
#define KDTR_Ctrl1      0xE1 //DTR Period

#define KSys_Ctrl1   0xBD //Noise Thresold Control Register
#define KSys_Ctrl2   0xBE //Noise Thresold Control Register
#define KSys_Ctrl3   0xBF //Noise Thresold Control Register

#define Checksum_Ctrl   0xC9 // Checksum Control Register
#define Checksum_W      0xCA // Checksum of Write Data
#define Checksum_R      0xCB // Checksum of Read Data

// ============= ANMG08K I2C Slave Address ============================//
#define ANMG08K_ID_OPEN  0x24 //0x48 >>1( 0b0100100 7bit + R/W 1 Bit , 7bit=0x24, 8bit=0x48)
#define ANMG08K_ID_GND  0x54 //0xA8 >>1( 0b1010100 7bit + R/W 1 Bit , 7bit=0x54, 8bit=0xA8)
// ============= ANMG08K I2C Slave Address ============================//

void  Init_ANMG08K(void); //Initialize ANMG08K

byte read_data1 = 0;   //Touch output read data
byte read_data2 = 0;   //Touch output read data
byte checksum1= 0;     //Checksum Data

#define RESET_PIN 7 //Reset pin
#define EN_PIN    6 //I2C Enable Pin

void Register_Dump()
{
   byte read_data[1] = {0};

   for (int i = 0; i < 256; i += 16)
   {
      for (int j = 0; j <= 15; j++)
      {
         Wire.beginTransmission(ANMG08K_ID_OPEN);    // sned ic slave address
         Wire.write((i + j));                   // sends register address
         Wire.endTransmission();                // stop transmitting
         Wire.requestFrom(ANMG08K_ID_OPEN, 1);       // data read (1 byte)
         read_data[0] = Wire.read();            //
         print2hex(read_data, 1);               //
      }
      Serial.write(LF);
      Serial.write(CR);
   }
   delay(500);
}

void print2hex(byte *data, byte length) //Print Hex code
{
   Serial.print("0x");
   for (int i = 0; i < length; i++)
   {
      if (data[i] < 0x10)
      {
         Serial.print("0");
      }
      Serial.print(data[i], HEX);
      Serial.write(SPC);
   }
}

void setup(){
  delay(200); //wait for 200[msec], Power on Reset

  Wire.begin();          // join i2c bus (address optional for master)
  Wire.setClock(200000); // 200Khz (200Kbps)
  Serial.begin(115200);  // start serial for output (Speed)
  //put your setup code here, to run once:

  pinMode(RESET_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  
  Init_ANMG08K(); //Initialize ANMG08K
  delay(100); //wait for 100[msec]
    
}
void loop() {
  
   byte read_data[2] = {0};

   Wire.beginTransmission(ANMG08K_ID_OPEN); // sned ic slave address
   Wire.write(byte(Output));           // sends register address
   Wire.endTransmission();             // stop transmitting
   Wire.requestFrom(ANMG08K_ID_OPEN, 2);    // key data read (1 byte)
   while ( Wire.available() )
    {
      read_data[0] = Wire.read();
      read_data[1] = Wire.read();
   }
   //Wire.endTransmission(); // I2C Stop

   Serial.write(10);
   Serial.print("-------Touch Sensor Output Data , Checksum Data ---- > "); // Test Code
   delay(20);

   print2hex(read_data, 2);
   //Serial.print(read_data, HEX);
   //Serial.write(SP);
   //Serial.write(LF);
   //Serial.write(CR);
   delay(20);   

}

void  Init_ANMG08K(void)
{
   // ---------------- Register unlock Control Start----------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Reg_Unlock));       // 0x65h
   Wire.write(0xAA);
   // 0xAA : Register Unlock Enable
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Reg_Unlock));       // 0x65h
   Wire.write(0x55);
   // 0x55 : Register Unlock Enable 
   Wire.endTransmission(); //

    Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(BIST_Unlock));     // 0xA9h
   Wire.write(0xAA);
   // 0xAA :  Register unlock Enable 
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(BIST_Unlock));     // 0xA9h
   Wire.write(0x55);
   // 0x55 :  Register unlock Enable 
   Wire.endTransmission(); //
// ---------------- Register unlock Control End----------------------------
   
   //------------------ Software Reset Enable (Reset )----------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN); // 
   Wire.write(byte(Global_Ctrl1)); //  0x36h
   Wire.write(byte(0x49)); // Software Reset Enable, Response time ON =3, OFF=3, Normal Mode
   Wire.endTransmission(); //
   //-----------------------------------------------------------------------

   // --------------- Hidden Register Start ---------------------------------
   // user does not change the register. please contact us to change.
   // -----------------------------------------------------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(0x10)); // address
   Wire.write(byte(0xCE)); // data
   Wire.endTransmission(); //   

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(0x13)); // start address
   Wire.write(byte(0x92)); //0x13h, 0x92
   Wire.write(byte(0x83)); //0x14h, 0x83
   Wire.write(byte(0x73)); //0x15h, 0x74
   Wire.write(byte(0x64)); //0x16h, 0x64
   Wire.write(byte(0xFF)); //0x17h, 0xFF  
   Wire.write(byte(0x1B)); //0x18h, 0x1B
   Wire.endTransmission(); //   

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(0x1A)); // address
   Wire.write(byte(0x00)); // data
   Wire.endTransmission(); // 

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(0x35)); // address
   Wire.write(byte(0x01)); // MFM up Exp time
   Wire.endTransmission(); //   

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(0x48)); // address
   Wire.write(byte(0x0D)); // data (sense loop )
   Wire.endTransmission(); //
   // --------------- Hidden Register End-------------------------------

   // ---------------- user code ---------------------//   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Ch_enable)); // 0x01h
   Wire.write(0xFF); // 0:Touch Key Disable, 1: Touch Key Enable
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Check_Reset));          // 0x04h
   Wire.write(0x03);                   // H/W Check Rest
   Wire.endTransmission();             //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Global_Ctrl0));  // 0x05h
   Wire.write(0xF1);                // Global Control Register 0
   Wire.endTransmission();  

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Clock_Ctrl)); //  0x34h
   Wire.write(0x26); // 
   Wire.endTransmission(); // 

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Pre_Scaler)); // 0x37h
   Wire.write(0xE6); // 
   // Calibration Speed Pre-scaler , (bit4 ~ bit0)
   Wire.endTransmission(); //   

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Global_Ctrl2)); // 0x38h
   Wire.write(0xBC); // Expire time Max
   //Wire.write(0x80); // Expire time disable
   Wire.endTransmission(); //  
   
//------------ Sensitivity control  -----------------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity1)); // 0x39h
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //      

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity2)); // 0x3Ah
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity3)); // 0x3Bh
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity4)); // 0x3Ch
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity5)); // 0x3Dh
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity6)); // 0x3Eh
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity7)); // 0x3Fh
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Sensitivity8)); // 0x40h
   Wire.write(0x28); // HEX x 0.025 = 1.0%
   Wire.endTransmission(); //  
 
// ------------ Calibration Speed Control ------------------------ 
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Cal_Speed1)); // 0x41h
   Wire.write(0x66); //  Calibration Speed defualt
   //Wire.write(0x66); // Default Value 
   Wire.endTransmission(); //   
   
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Cal_Speed2)); // 0x42h
   Wire.write(0x66); //  Calibration Speed defualt
   //Wire.write(0x66); // Default Value
   Wire.endTransmission(); //   
// -------------------- LED PWM Control -----------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(PWM_Ctrl1)); // 0x43h
   Wire.write(0x00); // D2,D1
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(PWM_Ctrl2)); // 0x44h
   Wire.write(0x00); // D4,D3
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(PWM_Ctrl3)); // 0x45h
   Wire.write(0x00); // D6,D5
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(PWM_Ctrl4)); // 0x46h
   Wire.write(0x00); // D8,D7
   Wire.endTransmission(); //
// ---------------- Port Mode Control Register ------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Port_Mode)); // 0x47h
   Wire.write(0x00); // 0: Parallel output mode, 1: LED Drive mode
   Wire.endTransmission(); //

   // ---------------- Hysteresis Sensitivity Register --------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Hys_Sensitivity)); // 0x49h
   Wire.write(0x80); // Hysteresis Sensitivity enable
   Wire.endTransmission(); //

   // ---------------- Hysteresis Level Option Register --------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(Hys_Levle_Opt)); // 0x4Ah
   Wire.write(0x07); // Hysteresis Sensitivity % ratio factor
   Wire.endTransmission(); //

   // ---------------- Noise threshold Control Register --------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN);// 
   Wire.write(byte(KSys_Ctrl1)); // 0xBDh
   Wire.write(0x02); // 0xBDh, Noise Calibration Hold Enable
   Wire.write(0xE5); // 0xBEh, Cal. Hold level Sensitivity x 75%(Re Cal. hold 75%)
   Wire.write(0x7F); // 0xBFh, Cal. Hold tiem 64 period
   Wire.endTransmission(); //   

   // ---------------- DTR (Direct touch reset) Contrl Register --------------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Dtr_Ctrl3));        // 0x6Dh
   Wire.write(0x01); // DTR Enable
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(DTR_ICtrl1));        // 0xD9h
   Wire.write(0x07); //0xD9h, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDAh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDBh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDCh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDDh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDEh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xDFh, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.write(0x07); //0xE0h, DTNegative Reset %, 0x07= 0x07 x 0.1% = 0.7%
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(KDTR_Ctrl1));        // 0xE1h
   Wire.write(0x0E); // set period (1cycle t 64 cycle)
   Wire.endTransmission(); //

   //------------------ Software Reset Disable (Clear) ---------------------
   Wire.beginTransmission(ANMG08K_ID_OPEN); // 
   Wire.write(byte(Global_Ctrl1)); // 
   Wire.write(byte(0x48)); // Software Reset Disable, Response time ON =3, OFF=3
   //Wire.write(byte(0x4A)); // Software Reset Disable, Response time ON =3, OFF=3,BF mode(LED)
   Wire.endTransmission(); //

   // ---------------- Register Lock Control ---------------------------------
   // It is recommended to use it to prepare for communication errors. 
   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(Reg_Unlock));       // 0x65h
   Wire.write(0x01);                   
   // 0x01 : Register Lock Enable (Address 0x00h~0x6Dh, 0xABh~9xE2h)
   Wire.endTransmission(); //

   Wire.beginTransmission(ANMG08K_ID_OPEN); //
   Wire.write(byte(BIST_Unlock));      // 0xA9h
   Wire.write(0x01);                   
   // 0x01 : BIST Register Lock Enable (Address 0x6Eh ~ 0x77h,0xE3h~0xEFh)
   Wire.endTransmission(); //

   //================ END Initialize ANMG08K ==================================
   }

// End
