/*
 Name:		simple_salt_check.ino
 Created:	7/17/2018 12:28:34 PM
 Author:	James, Brandon, and Bryce
*/

/*libraries/ lcd setup*/
#include <SoftwareSerial.h>
SoftwareSerial LCD(3, 2);


			/*Pin Vars*/
const byte conSensPower = 8;	//power on control for the conductivity sensor
const byte conSensRead = 1;		//reads the conductivity sensor
const byte diSolenoid = 6;		//valve for di water
const byte stSolenoid = 5;		//valve for salty water 

			/*User Defined Settings*/

const byte	threeStDev = 5;	//holds 3 times the standard deviation, this is calculated and set by the user
const float saltySetpoint = 0.09;	//holds the salinity (in fractional form) that the user wants to mantain

								/*Vars that change*/

int sLCL;			//holds the lower control limit
int sUCL;			//holds the upper control limit
int setSaltAnalog;	//holds the salt setpoint in analog
int conCheck;		//var to hold conductivity reading

				// the setup function runs once when you press reset or power the board
void setup() {

			/*Pinmodes*/
	pinMode(stSolenoid, OUTPUT);
	pinMode(diSolenoid, OUTPUT);

	/*Get setpoint and control limits*/

	setSaltAnalog = (fracSlatToAn(saltySetpoint));	//converts the user setpoint to analog for the computer to read
	sUCL = (setSaltAnalog + threeStDev);	//set ulc to setpoint plus 3 std
	sLCL = (setSaltAnalog - threeStDev);	//set lcl to setpoint minus 3 std

											/*LCD Setup*/
	LCD.begin(9600); delay(500);		// set data rate to 9600 baud; wait for bootup
	LCD.write(254);  LCD.write(1);		// clear screen & move to top left position
	LCD.write(254);  LCD.write(132);	// move cursor to row 1, position 4
	LCD.write("LCL");					// write a text string starting at (1,4)
	LCD.write(254);  LCD.write(139);	// move cursor to row 1, position 11
	LCD.write("SP");					// write a text string starting at (1,11)
	LCD.write(254);  LCD.write(144);	// move cursor to row 1, position 16
	LCD.write("UCL");					// write a text string starting at (1,16)

	LCD.write(254);  LCD.write(192);	// move cursor to row 2, position 0
	LCD.write("S:");					// write a text string starting at (2,0)
	LCD.write(254);  LCD.write(195);	// move cursor to row 2, position 3   
	LCD.print(anSaltToFrac(sLCL), 2);	// write a text string starting at (2,3)
	LCD.write(254);  LCD.write(201);	// move cursor to row 2, position 9  
	LCD.print(saltySetpoint, 2);		// write a text string starting at (2,9)
	LCD.write(254);  LCD.write(207);	// move cursor to row 2, position 15
	LCD.print(anSaltToFrac(sUCL), 2);	// write a text string starting at (2,15)

	LCD.write(254);  LCD.write(148);	// move cursor to row 3, position 0
	LCD.write("T:");					// write a text string starting at (3,0)
	LCD.write(254);  LCD.write(152);	// move cursor to row 3, position 4   
	//LCD.print(tempinC(LCL), 1);		// write a text string starting at (3,4)
	LCD.write(254);  LCD.write(158);	// move cursor to row 3, position 10   
	//LCD.print(tempinC(setpoint), 1);	// write a text string starting at (3,10)
	LCD.write(254);  LCD.write(164);	// move cursor to row 3, position 16   
	//LCD.print(tempinC(UCL), 1);		// write a text string starting at (3,16)


	Serial.print(9600);
	
	Serial.print("Frac setpoint	= ");	Serial.print(saltySetpoint);	Serial.print("	An setpoint = ");	Serial.println(setSaltAnalog);
	Serial.print("An sUCL = ");	Serial.print(sUCL);		Serial.print("	Frac sUCL = ");	Serial.println((anSaltToFrac(sUCL)),6);
	Serial.print("An sLCL = ");	Serial.print(sLCL);		Serial.print("	Frac sLCL = ");	Serial.println((anSaltToFrac(sLCL)),6);

	/*pinmodes*/
	pinMode(conSensPower, OUTPUT);
	pinMode(conSensRead, INPUT);

}

// the loop function runs over and over again until power down or reset
void loop() {

	conCheck = conSensGet(1);	//test on point and return the value

	Serial.println(anSaltToFrac(conCheck),6);

	LCD.write(254); LCD.write(212);			// move cursor to row 3, position 0
	LCD.write("S=");						// write text to row 3, position 0
	LCD.write(254); LCD.write(214);			// move cursor to row 3, position 5
	LCD.print(anSaltToFrac(conCheck), 2);   // write text to row 3, position 5
	LCD.write(254); LCD.write(220);			// move cursor to row 3, position 10
	LCD.write("T=");						// write text to row 3, position 10 
	LCD.write(254); LCD.write(222);			// move cursor to row 3, position 10
	//LCD.write(tempinC, 1);				// write text to row 3, position 10 
	LCD.write(254); LCD.write(227);			// move cursor to row 3, position 10
	LCD.write("H=");						// write text to row 3, position 10 
	LCD.write(254); LCD.write(229);			// move cursor to row 3, position 10
	LCD.write("ON");						// write text to row 3, position 10 

	if (conCheck > sUCL)
	{
		openSolenoid(diSolenoid);
		delay(500);
		closeSolenoid(diSolenoid);
	}
	else if (conCheck < sLCL)
	{
		openSolenoid(stSolenoid);
		delay(500);
		closeSolenoid(stSolenoid);
	}
	delay(10000);
	Serial.println("10 seconds has passed!");
}

//auto pulses the sensor and gets the avd of a set amount of readings
int conSensGet(int checkCount)
{

	long int conAvd;	//var to hold the total of the checks

	for (int i = 0; i < checkCount; i++)
	{
		digitalWrite(conSensPower, HIGH);		//turns on the sensor power on
		delay(100);								//wait 0.1 sec
		conCheck = analogRead(conSensRead);		//read the conductivity and assign it it the var
		digitalWrite(conSensPower, LOW);		//turn off sensor power

		conAvd += conCheck;	//adds the totals of the check together to average them later

							/*Serial Prints*/
		//Serial.print("Check # = ");
		//Serial.print(i);

		//Serial.print("	conductivity reading = ");
		//Serial.print(conCheck);

		//Serial.print("	conAvd total = ");
		//Serial.print(conAvd);

		/*delay at end*/
		delay(1000);	//wait a sec
	}

	conAvd = (conAvd / checkCount);	//divide by the number of tests

	Serial.print("salinity = ");
	Serial.println(conAvd);

	return(conAvd);	//returns an average 

}

//converts fractional salt to analog salt
int fracSlatToAn(float saltFrac)
{

	saltFrac = 78.445*log(saltFrac) + 683.48;

	return(saltFrac);	//returns the fractional salt in analog form
}

//converts analog salt to a fractional salt 
float anSaltToFrac(float saltAn)
{
	
	saltAn = pow(EULER, (8*(25*saltAn-17087))/15689);	//computes frac value from the input AnVal

	return(saltAn);		//returns analog salt as a fraction
}

byte openSolenoid(byte whichSol)
{
	Serial.println("Enter Open Solenoid Function");
	digitalWrite(whichSol, HIGH);
	Serial.println("Exit Open Solenoid Function");
}

byte closeSolenoid(byte whichSol)
{
	Serial.println("Enter Close Solenoid Function");
	digitalWrite(whichSol, LOW);
	Serial.println("Exit Close Solenoid Function");
}
