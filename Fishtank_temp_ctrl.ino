/*
 Name:		Fishtank_temp_ctrl.ino
 Created:	7/11/2018 12:46:23 PM
 Author:	James, Timothy, & Brandon

 thermistor = pin A3
 heater pin = 10
 LCD pin = 2
 Di solenoid pin = 5
 salt solenoid pin = 6

 NOTES: sketch to control the heater and maintain a set temp

 Standard deviation Temp = 1
 Standard deviation of Salt times three = 5
*/

/*libraries/ lcd setup*/
#include <SoftwareSerial.h>
SoftwareSerial LCD(3, 2);

/*Pin Vars*/
const byte conSensPower = 8;	//power on control for the conductivity sensor
const byte conSensRead = 1;		//reads the conductivity sensor
const byte diSolenoid = 6;		//valve for di water
const byte stSolenoid = 5;		//valve for salty water 
const byte thermistor = 3;		//thermistor is on pin A3 this will not change
const byte heater = 10;			//heaterpin


/*User defined SETPOINT here*/
const byte userSetpoint = 25;		//the temp in degC that the user wants
const byte	threeStDev = 5;			//holds 3 times the standard deviation, this is calculated and set by the user
const float saltySetpoint = 0.09;	//holds the salinity (in fractional form) that the user wants to mantain

	
						/*vars that do change*/
//ints
int setpoint;		//the user defined setpoint
int	anSetTemp;		//holds the set temp in analog form
int anRawT;			//holds thermistor measure
int analogValue;	//temp in analog
int LCL;			//holds the lower control limit
int UCL;			//holds the upper control limit
int currentTemp;	//holds valve of current temp
int sLCL;			//holds the lower control limit
int sUCL;			//holds the upper control limit
int setSaltAnalog;	//holds the salt setpoint in analog
int conCheck;		//var to hold conductivity reading

//floats
float temp;			//var for temp in degC
float cTempinC;		//holds vaolue of current temp in degC

//longs
unsigned long mainTimer;	//used to hold a master time for time based events, will be used in the future


void setup()
{
	
	/*An setpoint, and upper and lower control limit settings*/
	//temp
	setpoint = tempinA(userSetpoint);	//the setpoint in An, gets temp from the setpoint and converts it to An
	LCL = setpoint - 3;	//takes the userdefined setpoint and creates a LCL 3 standard deviations below the setpoint
	UCL = setpoint + 3;	//takes the userdefined setpoint and creates a UCL 3 standard deviations above the setpoint

	//salinity
	setSaltAnalog = (fracSlatToAn(saltySetpoint));	//converts the user setpoint to analog for the computer to read
	sUCL = (setSaltAnalog + threeStDev);	//set ulc to setpoint plus 3 std
	sLCL = (setSaltAnalog - threeStDev);	//set lcl to setpoint minus 3 std

						/*Serial Outputs*/
	Serial.begin(9600);	//starts serial print

	/*Serial print to header for debug*/
	//temp
	/*
	Serial.println("			USER SETPOINT AND CNTL LIMITS");	//serial print header
	Serial.print("Setpoint degC = ");	Serial.print(userSetpoint);			Serial.print("	SetPoint Analog = ");		Serial.println(setpoint);
	Serial.print("UCL degC = ");		Serial.print(tempinC(UCL));			Serial.print("	UCL Analog = ");			Serial.println(UCL);
	Serial.print("LCL degC = ");		Serial.print(tempinC(LCL));			Serial.print("	LCL Analog = ");			Serial.println(LCL);
	*/

	//salinity
	Serial.print("Frac setpoint	= ");	Serial.print(saltySetpoint);	Serial.print("	An setpoint = ");	Serial.println(setSaltAnalog);
	Serial.print("An sUCL = ");	Serial.print(sUCL);		Serial.print("	Frac sUCL = ");	Serial.println((anSaltToFrac(sUCL)), 6);
	Serial.print("An sLCL = ");	Serial.print(sLCL);		Serial.print("	Frac sLCL = ");	Serial.println((anSaltToFrac(sLCL)), 6);


					/*Pinmodes*/
	pinMode(thermistor, INPUT);
	pinMode(heater, OUTPUT);
	pinMode(stSolenoid, OUTPUT);
	pinMode(diSolenoid, OUTPUT);

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

}

// the loop function runs over and over again until power down or reset
void loop()
{

	
	/*these should work in sqence heating the water to ucl then exit and turn off heater till it hits the lcl and repeat this*/
	/*
	currentTemp = getTemp();	//gets the current temp
	cTempinC = tempinC(currentTemp);	//converts current temp analog to degC

	//keeps heater on while temp is below UCL
	while (currentTemp < UCL)
	{
		heaterOn(cTempinC);					//turns heater on and updates LCD
		currentTemp = getTemp();	//checks Temp
		cTempinC = tempinC(currentTemp);	//converts current temp analog to degC
		Serial.println("	Heater on");	//serial print for debug
	}
	//turns heater off till LCL
	while (currentTemp > LCL)
	{
		heaterOff(cTempinC);				//turns heater off and updates LCD
		currentTemp = getTemp();	//checks Temp
		cTempinC = tempinC(currentTemp);	//converts current temp analog to degC
		Serial.println("	Heater off");	//serial print for debug
	}
	*/


	conCheck = conSensGet(1);	//test on point and return the value

	Serial.println(anSaltToFrac(conCheck), 6);

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

//gets temp in degC from analog
float tempinC(int aValue)
{
	temp = 0.1147 * aValue - 32.512;
	return temp;
}

//gets temp in analog from degC
int tempinA(float cTemp)
{
	analogValue = 8.7091 * cTemp + 283.79;
	return analogValue;
}

//gets the raw analog temp 
int	RawTemp()	//reads thermistor and gets analog val
{
	return(analogRead(thermistor));	//returns analog temp from themistor
}

//turns heater off and updates LCD
void heaterOff(float tempInC)
{
	digitalWrite(heater, LOW);	//turns heater off
	/*LCD control*/
	/*
	LCD.write(254); LCD.write(212);	// move cursor to row 3, position 0
	LCD.write("Temp=");				// write text to row 3, position 0
	LCD.write(254); LCD.write(217);	// move cursor to row 3, position 5
	LCD.print(tempInC, 1);				// write text to row 3, position 5
	LCD.write(254); LCD.write(222);	// move cursor to row 3, position 10
	LCD.write("HEATER=OFF");		// write text to row 3, position 10
	*/
	return;
}	

//turns heater on and updates LCD
void heaterOn(float tempInC)
{
	digitalWrite(heater, HIGH);	//turn heater on
	/*LCD control*/
	/*
	LCD.write(254); LCD.write(212); // move cursor to row 3, position 0
	LCD.write("Temp=");             // write text to row 3, position 0
	LCD.write(254); LCD.write(217);	// move cursor to row 3, position 5
	LCD.print(tempInC, 1);			    // write text to row 3, position 5
	LCD.write(254); LCD.write(222); // move cursor to row 3, position 10
	LCD.write("HEATER=ON ");			// write text to row 3, position 10 
	*/
	return;
}

//checks temp and runs an average a user defined test number
int tempCheck(int testAmount)	// (val should be the EXACT number you want) gets a set number of test temp checks and returns the ave
{
	float holdVal; //used to hold number
	float minTemp = 1023; //holds min temp
	float maxTemp = 0;	//holds max temp
	float aveNoMinMax;	//holds ave no min or max values
	for (int i = 0; i <= (testAmount - 1); i++)
	{
		int check = RawTemp();	//gets analog value
								//Serial.print("I = ");	//prints i for loop count check
								//Serial.println(i);
		holdVal += check;	//adds new value to old value
		if (minTemp > check)
		{
			minTemp = check;	//sets min temp to new low
		}
		if (maxTemp < check)
		{
			maxTemp = check;	//sets max temp to new low
		}
	}
	aveNoMinMax = (holdVal - minTemp - maxTemp) / (testAmount - 2); //gets the ave minus min and max	
	return(aveNoMinMax);	//give back analog temp
}

//gets temp in both degC and analog
int getTemp()	//checks the temp 100 times and does the avd minus min and max, then prints the anolog and *C Temp
{
	int getTemp = tempCheck(100);		// reads the thermistor 100 times and gives average minus the min and max
	tempPrint(getTemp);		//prints analog and degC val of Temp
	return(getTemp);	//returns anTemp
}

//serial prints for both analog and degC
void tempPrint(int anVal)	//prints the temp sent to it **Must be analog value**
{
	Serial.print("analog Temp = ");
	Serial.print(anVal);	//prints the raw analog from thermistor
	Serial.print("		Temp in C = ");
	Serial.println(tempinC(anVal));	//gets the Cvalue for the curent temp
	return;	//ends function
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

	saltAn = pow(EULER, (8 * (25 * saltAn - 17087)) / 15689);	//computes frac value from the input AnVal

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
