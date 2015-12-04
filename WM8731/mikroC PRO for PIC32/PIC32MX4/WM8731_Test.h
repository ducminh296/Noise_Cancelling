void PMPWaitBusy();
void Set_Index(unsigned short index);
void Write_Command( unsigned short cmd );
void Write_Data(unsigned int _data);

void WM8731_CMD(char address, unsigned int cmd);
void WM8731_Init();
void WM8731_SetVolume(char LeftCh, char RightCh );
void WM8731_Deactivate();
void WM8731_Activate();