#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define LEDVERDE 6
#define LEDROJO 5
#define ANALOGPILA 0

TinyGPS gps;
SoftwareSerial softSerial(4, 3); // (tx,rx)

String dato; //Se crea la variable dato tipo entero
const int BOTON = 2;
int val = 0;	  //val se emplea para almacenar el estado del boton
int state = 0;	// 0 LED apagado, mientras que 1 encendido
int old_val = 0;  // almacena el antiguo valor de val
char cadena[255]; //Creamos un array de caracteres de 256 cposiciones
int i = 0;		  //Tamaño actual del array
String lati, lon, coor;
long inicio, fin;
int sw = 0, paso = 0;
int analogValor = 0;
float voltaje = 0;
int ledDelay = 2000;
// Umbrales de niveles
float maximo = 4.3;
float minimo = 3.8;

void setup()
{
	Serial.begin(38400);
	softSerial.begin(9600);
	pinMode(7, OUTPUT); //Definimos el pin 7 como salida
	pinMode(2, INPUT);  // y BOTON como señal de entrada
	// Los pines de LED nivel de bateria en modo salida
	pinMode(LEDVERDE, OUTPUT);
	pinMode(LEDROJO, OUTPUT);
}

void loop()
{
	val = digitalRead(2); // lee el estado del Boton

	if (val == HIGH)
	{
		state = 1 - state;
		Serial.println("Panico");
	}
	old_val = val; // valor del antiguo estado
	if (state == 1)
	{
		digitalWrite(7, HIGH); // enciende el LED
		apagado();
	}
	else
	{
		digitalWrite(7, LOW); // enciende el LED
		localizador();
		bateria();
	}
}

void localizador()
{
	bool newData = false;
	unsigned long chars;
	unsigned short sentences, failed;
	if (sw == 0)
	{
		sw = 1;
		inicio = millis();
	}
	fin = millis();
	if (fin - inicio < 6000)
	{
	}
	else
	{
		sw = 0;

		// Intentar recibir secuencia durante
		for (unsigned long start = millis(); millis() - start < 10000;)
		{
			while (softSerial.available())
			{
				char c = softSerial.read();
				if (gps.encode(c)) // Nueva secuencia recibida
				{
					newData = true;
				}
			}
		}
	}
	if (newData)
	{
		float flat, flon;
		unsigned long age;
		gps.f_get_position(&flat, &flon, &age);
		lati = String(flat, 4);
		lon = String(flon, 4);
		coor = ("#" + lati + "/" + lon + "$");
		Serial.println(coor);
	}
}
void clean()
{
	for (int cl = 0; cl <= i; cl++)
	{
		cadena[cl] = 0;
	}
	i = 0;
}
void bateria()
{
	// Leemos valor de la entrada analógica
	analogValor = analogRead(ANALOGPILA);

	// Obtenemos el voltaje
	voltaje = 0.0044 * analogValor;

	// Dependiendo del voltaje mostramos un LED u otro
	if (voltaje >= maximo)
	{
		digitalWrite(LEDVERDE, HIGH);
		delay(ledDelay);
		digitalWrite(LEDVERDE, LOW);
	}
	else if (voltaje < maximo && voltaje > minimo)
	{
		digitalWrite(LEDROJO, HIGH);
		delay(ledDelay);
		digitalWrite(LEDROJO, LOW);
	}
}
void apagado()
{
	if (Serial.available() > 0) //Confirmamos si existe un valor en el puerto serie
	{
		dato = Serial.read(); //leemos el valor y lo asignamos a la variable dato

		//   switch(dato)	//comparamos el valor guardado en la variable dato
		// {
		//      case 'a': 	//si el dato leido es b
		//            digitalWrite(7,0);	//setiamos a 0V el pin 13
		//            state =0;
		//            break;
		//   }

		if (dato == "a")
		{
			state = 0;
			digitalWrite(7, 0);
		}
		dato = " ";
		clean();
	}
}
