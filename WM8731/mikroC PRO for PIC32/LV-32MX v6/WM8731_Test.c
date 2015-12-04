/**************************************************************************************************
* File: WM8731_Test
* File Type: Source Code File
* (C) mikroElektronika, 2010-2011
* Revision History:
*     Credits:
*       Ian Davidson
*       - initial release;
*     20120406 (JK)
* Description:
*     This project demonstrates communication with WM8731 audio codec.
*     Program reads one wav file from MMC and sends it to WM8731 for decoding
*     and playing.
*     MMC is connected on SPI2 and WM8731 on I1C2 and SPI1
* Target:
*       MCU:             P32MX460F512L
*       Dev.Board:       LV-32MX v6 : ac:LV32MX
*                        http://www.mikroe.com/eng/products/view/462/lv-32mx-v6-development-system/
*       Modules:         WM8731 on-board module
*                        http://www.cs.columbia.edu/~sedwards/classes/2012/4840/Wolfson-WM8731-audio-CODEC.pdf
*       Oscillator:      80MHz
*       SW:              mikroC PRO for PIC32
*                        http://www.mikroe.com/eng/products/view/623/mikroc-pro-for-pic32/
* NOTES:
*     - Make sure that MMC card is properly formatted (to FAT16 or just FAT)
*       before testing it on this example.
*     - Make sure that MMC card contains appropriate wav file. (sound.wav)
*     - Enable I2C for communication with WM8731 module (SW 14.1 and SW14.3)
*     - Enable Audio On/Out (SW14.5, 14.6, 14.7, 14.8)
*     - Enable MMC (SW13.4, 13.5, 13.6, 13.7, 13.8)
*     - Enable TFT (SW17, SW18, SW19.2, 19.3, 19.4, 19.5, 19.6, 19.7)
**************************************************************************************************/
#include "WM8731_Resources.h"
#include "WM8731_Test.h"
#include <built_in.h>
#include "__Lib_MmcFat16.h"
/**************************************************************************************************
* MMC Chip Select connection
**************************************************************************************************/
sbit MMC_Chip_Select            at  LATG9_bit;
sbit MMC_Chip_Select_Direction  at TRISG9_bit;

/**************************************************************************************************
* Global variables
**************************************************************************************************/
char filename[12] = "sound.wav";

struct MyRouundBuffer {
  char buffer[1024];
  unsigned int buf_in;
  unsigned int buf_out;
} RBuff;

char ucWM8731_run_test;

/**************************************************************************************************
* Function MCU_Init
* -------------------------------------------------------------------------------------------------
* Overview: Function initializes PIC32 controller and TFT screen
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void MCU_Init(){
  AD1PCFG = 0xFFFFFFFF;                 // Configure AN pins as digital I/O
  // PMP setup
  PMMODE = 0;
  // PMAEN: Parallel Master Port Address Enable Register
  PMAEN  = 0;  // sve seovano na 0 znaci pinovi se ne koriste kao adresni vec normalni I/O
  PMCON  = 0;  // WRSP: Write Strobe Polarity bit
              // 0 = Read strobe active-low (PMWR) - 0 je setovano po resetu
  LATD0_bit = 0;
  //  PMCON = 0x1b03
  //  PMMODE = 0x60A
  PMMODE = 0;
  PMAEN = 0;
  PMCON = 0;
  PMMODE = 0x0604;
  PMCON = 0x8300;
  TRISE = 0;
  TRISD = 0;

  TFT_Set_Active(Set_Index,Write_Command,Write_Data);
  TFT_Rotate_180(1);
  TFT_Init(320, 240);
}

/**************************************************************************************************
* Function DrawWM8731Scr
* -------------------------------------------------------------------------------------------------
* Overview: Function draws screen for WM8731 example
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void DrawWM8731Scr(){
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("WM8731  TEST", 75, 14);
  TFT_Set_Font(Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("LV-32MX v6", 19, 223);
  TFT_Set_Font(Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}

/**************************************************************************************************
*  Interrupt Service Routine: SPI1 transmit
***************************************************************************************************
*  ICS:                          SRS
*  Interrupt priority level:     7
*  Interrupt suppriority level:  3
**************************************************************************************************/
void Spi_Output() iv IVT_SPI_1 ilevel 7 ics ICS_SRS {
  char i;
  unsigned long int temp;

  if (SPI1TXIF_bit == 1) {            // 32bits were transmited
    temp = 0;                         // Reset temp
    for (i = 1; i < 5; i ++){         // Read four bytes from buffer
      temp = temp << 8;               // And make 32bit data
      temp = temp | RBuff.buffer[RBuff.buf_out];
      RBuff.buf_out ++;               // Increment buffer pointer
      if (RBuff.buf_out == 1023)        // Check if the buffer pointer is at the end of buffer
        RBuff.buf_out = 0;            // If so reset to first byte in buffer
    }
    
    SPI1BUF = temp;                   // Send temp to SPI1BUF
    SPI1TXIF_bit = 0;                 // Clear TX interrupt flag
  }
}

/**************************************************************************************************
* Function SPI1_Init_FrameMode
* -------------------------------------------------------------------------------------------------
* Overview: Function Initializes SPI module in Slave Frame mode
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void SPI1_Init_FrameMode(){
  // WM8731 module is connected to SPI1 module
 
  // Initialize SPI2 module
  // slave_mode    = _SPI_SLAVE
  // data_mode     = _SPI_32_BIT
  // clock_divider = 1-1024
  // slave_select  = _SPI_SS_ENABLE (slave mode)
  // data_sample   = _SPI_DATA_SAMPLE_MIDDLE
  // clock_idle    = _SPI_CLK_IDLE_LOW
  // edge          = _SPI_IDLE_2_ACTIVE
  Spi1_init_advanced(_SPI_SLAVE, _SPI_32_BIT, 64, _SPI_SS_ENABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_LOW, _SPI_IDLE_2_ACTIVE);

  // We will use SPI1 Slave Frame mode
  SPIFE_bit = 1;                      // Frame Sync Pulse Edge Select Bit
  FRMPOL_bit = 1;                     // Frame Sync Polarity Bit
  FRMSYNC_bit = 1;                    // Frame Sync Pulse Direction Control
  FRMEN_bit = 1;                      // Frame SPI Support Bit

  SPI1BUF = 0;                        // Clear SPI1BUF buffer
}

/**************************************************************************************************
* Function GetFileHeader
* -------------------------------------------------------------------------------------------------
* Overview: Function reads header from wmv file and displays it on TFT
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void GetFileHeader(){                 // Read file header according to Canonical Wave File Format
unsigned long int temp;
char TempString[11];
char TempString1[22];
char character;
     
  TFT_Set_Font(Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  
  MMC_Fat_Seek(22);  // Set pointer to appropriate position in file
  
  Mmc_Fat_Read(&character);             // Display No. of channels info
  temp = character;
  Mmc_Fat_Read(&character);
  temp += character << 8;
  wordtostr(temp, TempString);
  TempString1[0] = 0;
  strcat(TempString1, "No. of channels: ");
  strcat(TempString1, TempString);
  TFT_Write_Text(TempString1, 50, 120);
  
  Mmc_Fat_Read(&character);             // Display sample rate info
  temp = character;
  Mmc_Fat_Read(&character);
  temp += character << 8;
  Mmc_Fat_Read(&character);
  temp += character << 8;
  Mmc_Fat_Read(&character);
  temp += character << 8;
  longwordtostr(temp, TempString);
  TempString1[0] = 0;
  strcat(TempString1, "sample rate: ");
  strcat(TempString1, TempString);
  TFT_Write_Text(TempString1, 50, 140);
  
  MMC_Fat_Seek(MMC_Fat_Tell() + 6);
  
  Mmc_Fat_Read(&character);             // Display bit sample info
  temp = character;
  Mmc_Fat_Read(&character);
  temp += character << 8;
  wordtostr(temp, TempString);
  TempString1[0] = 0;
  strcat(TempString1, "bits per sample: ");
  strcat(TempString1, TempString);
  TFT_Write_Text(TempString1, 50, 160);
  
  TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}

/**************************************************************************************************
* Function WM8731_Play
* -------------------------------------------------------------------------------------------------
* Overview: Function plays wav file from SD card
*           Make sure that appropriate file is opened for reading before invoking this function
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void WM8731_Play(){
unsigned long int size;
char character;

  size = Mmc_Fat_Get_File_Size();     // Get file size
  size = size - 44;                   // Header is 44 bytes
  MMC_Fat_Seek(45);                   // Position pointer on next byte (here starts music)

  RBuff.buf_in = 0;                   // Reset buffer to the start
  RBuff.buf_out = 0;                  // Reset output buffer pointer to the start

  while (RBuff.buf_in < 511){         // Load first 512 bytes to the buffer
    Mmc_Fat_Read(&character);
    RBuff.buffer[RBuff.buf_in ++] = character;
    size --;
  }
  
  SPI1TXIE_bit = 1;                   //Enable spi interrupts
  EnableInterrupts();                 // Enable all interrupts
  WM8731_Activate();

  while (size > 0){
    if (RBuff.buf_in != RBuff.buf_out) {
      Mmc_Fat_Read(&character);       // Load new value to buffer
      RBuff.buffer[RBuff.buf_in ++] = character;
      if (RBuff.buf_in == 1023)
        RBuff.buf_in = 0;

    size --;
    }
  }
  
  // Stop Playing
  WM8731_Deactivate();
}

/**************************************************************************************************
* Function WM8731_Test()
* -------------------------------------------------------------------------------------------------
* Overview: Function run WM8731 test
* Input: Nothing
* Output: test status: 0 - skiped; 1 - pass; 2 - fail
**************************************************************************************************/
void WM8731_Test(char *test){
char fileHandle;
  
  // Reset error flag
  *test = 0;
  
  SPI_Set_Active(SPI2_Read, SPI2_Write);               // Sets the SPI1 module active for mmc card
  delay_ms(100);
  
  if (Mmc_Fat_Exists(&filename) == 1) {                 // Check if file exists
    fileHandle = Mmc_Fat_Open(&filename, FILE_READ, 0); // Open wav file
    
    TFT_Write_Text("2. File Assigned", 30, 80);

    TFT_Write_Text("3. Get file header:", 30, 100);
    GetFileHeader();                                   // Read header of wav file
    
    TFT_Write_Text("4. Play audio... :)", 30, 180);
    WM8731_Play();                                     // Play music
    
    TFT_Write_Text("5. End.", 30, 200);

    *test = 1;
  }
  else{
    TFT_Write_Text("2. File Not Found", 30, 80);
    *test = 2;
  }
}

/**************************************************************************************************
* main function
**************************************************************************************************/
void main(){
  MCU_Init();

/**************************************************************************************************
*  Set interrupt priority levels
**************************************************************************************************/
  SPI1IP0_bit = 1;          // Set SPI interrupt
  SPI1IP1_bit = 1;          // priority level
  SPI1IP2_bit = 1;          // to 7
  SPI1IS0_bit = 1;          // And subpriority
  SPI1IS1_bit = 1;          // to 3

/*************************************************************************************************/
/*************************************************************************************************/
  DrawWM8731Scr();

/**************************************************************************************************
*  Initialize SPI2 for communication with SD card
**************************************************************************************************/
  // Initialize SPI2 module
  // master_mode   = _SPI_MASTER
  // data_mode     = _SPI_8_BIT
  // clock_divider = 1-1024
  // slave_select  = _SPI_SS_DISABLE (Only for slave mod)
  // data_sample   = _SPI_DATA_SAMPLE_MIDDLE
  // clock_idle    = _SPI_CLK_IDLE_LOW
  // edge          = _SPI_ACTIVE_2_IDLE
  SPI2_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, 64, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);
  Delay_ms(10);

  TFT_Write_Text("1. Initializing MMC_FAT", 30, 60);
  if (Mmc_Fat_Init() == 0){                 // Reinitialize spi at higher speed
    SPI2_Init_Advanced(_SPI_MASTER, _SPI_8_BIT, 8, _SPI_SS_DISABLE, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_HIGH, _SPI_ACTIVE_2_IDLE);

/**************************************************************************************************
*  Start test
**************************************************************************************************/
    // Initialise SPI1
    SPI1_Init_FrameMode();
   
    WM8731_Init();
//    WM8731_SetVolume(60, 60);
    WM8731_Test(&ucWM8731_run_test);
  }
  else
    TFT_Write_Text("2. MMC FAT not initialized", 30, 80);
}
/**************************************************************************************************
* End Of File
**************************************************************************************************/