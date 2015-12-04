/**************************************************************************************************
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111020 (JK):
*       - initial release;
* Description:
*       - This is driver file for WM8731 Audio module
*
* Target:
*       MCU:             pic32mx795f512l
*       Dev.Board:       PIC32MX7
*                        http://www.mikroe.com/eng/products/view/573/multimedia-board-for-pic32mx7/
*       Oscillator:      80000000 Hz (80MHz)
*       SW:              mikroC PRO for PIC32
*                        http://www.mikroe.com/eng/products/view/623/mikroc-pro-for-pic32/
* NOTES:
**************************************************************************************************/
#include <built_in.h>

/**************************************************************************************************
* WM8731 sound chip register addresses
**************************************************************************************************/
const WM8731_ADDRESS             = 0x34;       // WM8731 chip address on I2C bus
const WM8731_REG_LLINE_IN        = 0x00;       // Left Channel Line Input Volume Control
const WM8731_REG_RLINE_IN        = 0x01;       // Right Channel Line Input Volume Control
const WM8731_REG_LHPHONE_OUT     = 0x02;       // Left Channel Headphone Output Volume Control
const WM8731_REG_RHPHONE_OUT     = 0x03;       // Right Channel Headphone Output Volume Control
const WM8731_REG_ANALOG_PATH     = 0x04;       // Analog Audio Path Control
const WM8731_REG_DIGITAL_PATH    = 0x05;       // Digital Audio Path Control
const WM8731_REG_PDOWN_CTRL      = 0x06;       // Power Down Control Register
const WM8731_REG_DIGITAL_IF      = 0x07;       // Digital Audio Interface Format
const WM8731_REG_SAMPLING_CTRL   = 0x08;       // Sampling Control Register
const WM8731_REG_ACTIVE_CTRL     = 0x09;       // Active Control
const WM8731_REG_RESET           = 0x0F;       // Reset register

/**************************************************************************************************
* WM8731 sound chip constants (for default set up)
**************************************************************************************************/
const _WM8731_left_lineIn        = 0x0180;     // Mic settings: Enable mute, Enable simultaneous load to left and right channels
const _WM8731_Right_lineIn       = 0x0180;     // Mic settings: Enable mute, Enable simultaneous load to left and right channels
const _WM8731_Left_hp            = 0x00F0;     // Headphone settings : -9dB output
const _WM8731_Right_hp           = 0x00F0;     // Headphone settings : -9dB output
const _WM8731_AnalogAudio        = 0xD0;       // DAC Select
const _WM8731_DigitalAudio       = 0x06;
const _WM8731_power              = 0x00;       // Disable Power down
const _WM8731_DAIF               = 0x43;       // Enable Master Mode and 32bit data
const _WM8731_Sampling           = 0x23;       // 44100Hz
const _WM8731_Activate           = 0x01;       // Module is ON
const _WM8731_Deactivate         = 0x00;       // Module is OFF
const _WM8731_Reset              = 0x00;       // Reset value

/**************************************************************************************************
* Function WM8731_CMD
* -------------------------------------------------------------------------------------------------
* Overview: Function write word of data to WM8731
* Input: register address, data
* Output: Nothing
**************************************************************************************************/
void WM8731_CMD(char address, unsigned int cmd){
char addr;
  // B[15:9] Are Control Address Bits
  // B[8:0]  Are Control Data Bits
  addr = address << 1;                 // Shift left for one positions
  addr = addr | (hi(cmd) & 1);

  I2C2_Start();                        // Start I2C2 module
  I2C2_Write(WM8731_ADDRESS);          // Write Adress of WM8731 chip
  I2C2_Write(addr);                    // Write register address
  I2C2_Write(lo(cmd));                 // Write command
  I2C2_Stop();                         // Stop I2C2 module
}

/**************************************************************************************************
* Function WM8731_Init
* -------------------------------------------------------------------------------------------------
* Overview: Function Initializes WM8731
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void WM8731_Init(){
  // Initialize WM8731 module
  I2C2_init(100000);                          // Init I2C2 module (100Khz)
  delay_ms(100);

  // Write to WM8731 registers appropriate values
  WM8731_CMD(WM8731_Reg_Reset, _WM8731_Reset);               // Reset module
  WM8731_CMD(WM8731_Reg_Lline_in, _WM8731_left_lineIn);      // Left line in settings
  WM8731_CMD(WM8731_Reg_Rline_in, _WM8731_Right_lineIn);     // Rigth line in settings
  WM8731_CMD(WM8731_Reg_Lhphone_out, _WM8731_Left_hp);       // Left headphone out settings
  WM8731_CMD(WM8731_Reg_Rhphone_out, _WM8731_Right_hp);      // Right headphone out settings
  WM8731_CMD(WM8731_Reg_Analog_path, _WM8731_AnalogAudio);   // Analog paths
  WM8731_CMD(WM8731_Reg_Digital_path, _WM8731_DigitalAudio); // Digital paths
  WM8731_CMD(WM8731_Reg_Pdown_ctrl, _WM8731_power);          // Power down control
  WM8731_CMD(WM8731_Reg_Digital_IF, _WM8731_DAIF);           // Digital interface
  WM8731_CMD(WM8731_Reg_Sampling_ctrl, _WM8731_Sampling);    // Sampling control
//  WM8731_CMD(WM8731_Reg_Active_ctrl, _WM8731_Activate);      // Activate module

  delay_ms(10);
}

/**************************************************************************************************
* Function WM8731_SetVolume
* -------------------------------------------------------------------------------------------------
* Overview: Function Sets volume on both output channels
* Input: Left and Right channel Volume
*        Inputs are from 0 to 80 and outout is from -73dB to 6dB in 1dB steps
*        0 = mute
*        1 = - 73 dB
*        80 = + 6 dB
* Output: Nothing
**************************************************************************************************/
void WM8731_SetVolume(char LeftCh, char RightCh){
unsigned int temp;

  if (LeftCh > 80)
    LeftCh = 80;
  if (RightCh > 80)
    RightCh = 80;
  
  temp = LeftCh + 0x00AF;
  WM8731_CMD(WM8731_Reg_Lhphone_out, temp);      //left headphone out
  temp = RightCh + 0x00AF;
  WM8731_CMD(WM8731_Reg_Rhphone_out, temp);      //right headphone out
}

/**************************************************************************************************
* Function WM8731_Activate
* -------------------------------------------------------------------------------------------------
* Overview: Function Activates WM8731
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void WM8731_Activate(){
  WM8731_CMD(WM8731_Reg_Active_ctrl, _WM8731_Activate);
}

/**************************************************************************************************
* Function WM8731_Deactivate
* -------------------------------------------------------------------------------------------------
* Overview: Function Deactivates WM8731
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void WM8731_Deactivate(){
  WM8731_CMD(WM8731_Reg_Active_ctrl, _WM8731_Deactivate);
}

/**************************************************************************************************
* End Of File
**************************************************************************************************/