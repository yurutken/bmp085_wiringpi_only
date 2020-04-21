
#include <wiringPiI2C.h>				// FOR WIRINGPI I2C FUNCTIONS
#include <wiringPi.h>					// for delay() FUNCTION
#include <cstdio>						// OLD FRIEND PRINTF()
#include <string>						// STRING FUNCTIONS 

#define ADDRESS_OF_BMP_SENSOR 0x77
#define FIRST_WRITE	0x2E
#define FIRST_WRITE_ADDRESS 0xF4

using namespace std;					// FOR USING STD::STRING




int read_pressure_temperature(int* temperature,long* pressure )					// READ TEMPERATURE AND PRESSURE FROM BMP SENSOR 
{
    int T;
    long pres;


 	int fd = wiringPiI2CSetup(ADDRESS_OF_BMP_SENSOR);				
	
	if ( fd == -1 ){
		printf( "ERROR\n" );
		return (-1);

		}
																				// READ ALL REGISTERS FOR CALIBRATION VALUES
	short MC_1 = wiringPiI2CReadReg8(fd,0xBC);
	short MC_2 = wiringPiI2CReadReg8(fd,0xBD);
	short MC = (MC_1*256)+MC_2;

	short MD_1 = wiringPiI2CReadReg8(fd,0xBE);
	short MD_2 = wiringPiI2CReadReg8(fd,0xBF);
	short MD = (MD_1*256)+MD_2;

	short MB_1 = wiringPiI2CReadReg8(fd,0xBA);
	short MB_2 = wiringPiI2CReadReg8(fd,0xBB);
	short MB = (MB_1*256)+MB_2;

	short AC1_1 = wiringPiI2CReadReg8(fd,0xAA);
	short AC1_2 = wiringPiI2CReadReg8(fd,0xAB);
	short AC1 = (AC1_1*256)+AC1_2;

	short AC2_1 = wiringPiI2CReadReg8(fd,0xAC);
	short AC2_2 = wiringPiI2CReadReg8(fd,0xAD);
	short AC2 = (AC2_1*256)+AC2_2;

	short AC3_1 = wiringPiI2CReadReg8(fd,0xAE);
	short AC3_2 = wiringPiI2CReadReg8(fd,0xAF);
	short AC3 = (AC3_1*256)+AC3_2;

	short AC4_1 = wiringPiI2CReadReg8(fd,0xB0);
	short AC4_2 = wiringPiI2CReadReg8(fd,0xB1);
	unsigned short AC4 = (AC4_1*256)+AC4_2;

	short AC5_1 = wiringPiI2CReadReg8(fd,0xB2);
	short AC5_2 = wiringPiI2CReadReg8(fd,0xB3);
	unsigned short AC5 = (AC5_1*256)+AC5_2;

	short AC6_1 = wiringPiI2CReadReg8(fd,0xB4);
	short AC6_2 = wiringPiI2CReadReg8(fd,0xB5);
	unsigned short AC6 = (AC6_1*256)+AC6_2;

	short B1_1 = wiringPiI2CReadReg8(fd,0xB6);
	short B1_2 = wiringPiI2CReadReg8(fd,0xB7);
	short B1 = (B1_1*256)+B1_2;
	
	short B2_1 = wiringPiI2CReadReg8(fd,0xB8);
	short B2_2 = wiringPiI2CReadReg8(fd,0xB9);
	short B2 = (B2_1*256)+B2_2;


	wiringPiI2CWriteReg8(fd, FIRST_WRITE_ADDRESS, 0x2E);						// WRITE 0x2E AND WAIT FOR 5 MS AS DATASHEET SAYS AFTER READ F6 AND F7 AND CALCULATE UT (PLEASE CHECK DATASHEET)
	delay(15);			// wait 5 ms										
	short data_msb = wiringPiI2CReadReg8(fd,0xF6);
	short data_lsb = wiringPiI2CReadReg8(fd,0xF7);
	long UT = (data_msb*256)+data_lsb;

	int X1  =  ( (UT-AC6) * AC5 ) / 32768  ;
	int X2  =   ( MC * 2048 ) / ( X1 + MD) ;			
    int B5  =  X1 + X2;

	 T = (B5 + 8) / 16 ;

	wiringPiI2CWriteReg8(fd, FIRST_WRITE_ADDRESS, 0x34);						// WRITE 0x34 AND WAIT AND CALCULATE UP FOR PRESSURE CALCULATION (PLEASE CHECK DATASHEET)
	delay(25);												// wait 5 ms
	data_msb = wiringPiI2CReadReg8(fd,0xF6);
	data_lsb = wiringPiI2CReadReg8(fd,0xF7);
	short data_xlsb = wiringPiI2CReadReg8(fd,0xF8);
	long UP = ((data_msb*65536)+ (data_lsb*256) + (data_xlsb) ) / 256;


																			 // MAKE THE BORING CALCULATIONS FOR PRESSURE (PLEASE CHECK DATASHEET)
	long B6 = B5 - 4000;
	long X1_1 = ( B2* ((B6*B6) / 4096 )) / 2048;
	long X2_1 =  ( AC2 * B6 ) / 2048;
	long X3_1 = X1_1 + X2_1;
	long B3 = (  ( AC1*4 )+ X3_1 +2) / 4;
	long X1_2 = ( AC3 * B6 ) / 8192;
	long X2_2 = ( B1 *  (( B6 * B6 ) / 4096 ) )/ 65536;
	long X3_2 = (X1_2 + X2_2 + 2 ) /  4;
	 unsigned long B4 = ( AC4 * (unsigned long)(X3_2 + 32768) ) / 32768;
	 unsigned long B7 = ((unsigned long)UP - B3 ) * 50000;


		if ( B7 < 0x80000000) {pres = (B7*2) / B4 ;}
		else {pres = (B7/B4)*2 ;}
	long X1_3 = (pres/256) * (pres/256) ;
	X1_3 = ( X1_3 * 3038 ) / 65536;
	long X2_3  =( -7357 * pres) / 65536;
	pres = pres +  (  ( X1_3 + X2_3 + 3791) / 16 );
	

    *temperature = T;									
    *pressure = pres;

    return 0;

	
}



int main( void )
{
    int temperature=0;
    long pressure=0;

    int is_ok = read_pressure_temperature(&temperature,&pressure );			

    if ( is_ok == -1 ){
		printf( "ERROR\n" );			// IF WIRING PI CANT TAKE HANDLE-INT(FD) FROM ADDRESS OF BMP IT GIVES ERROR 
		return (-1);
		}


        //printf(" Temperature %d  C \n",temperature);			
        //printf(" Pressure %d  hPa\n",pressure);
        
		///////////////////////////////////////////////				FORMAT AS STRING AND PUT "." 
		string temp = to_string(temperature);
		temp.insert((temp.length()-1),".");
		string press = to_string(pressure);
		press.insert((press.length()-2),".");
		
		printf(" Temperature %s  C \n",temp.c_str());
        printf(" Pressure %s  hPa\n",press.c_str());
		///////////////////////////////////////////////

}



