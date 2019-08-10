// LIBRERIAS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// CONSTANTES
#define LED_VERDE 6
#define LED_ROJO 5
#define LED_PANICO 7
#define ANALOG_PILA 0
#define PIN_BOTON 2
#define PIN_BUZZER 8
// constantes para definir el estado del boton
#define ON true
#define OFF false

// OBJETOS
TinyGPS gps;
SoftwareSerial GPS(4, 3);		 // Modulo GPS (RX = PIN 4 | TX = PIN 3)
SoftwareSerial BLUETOOTH(10, 9); // Modulo Bluetooth (RX = PIN 10 | TX = PIN 9)

// VARIABLES
String lati, lon, coor;
int dato;  // Se crea la variable dato tipo String
int i = 0; // Tamaño actual del array
int sw = 0;
int analogValor = 0;
int ledDelay = 50; // delay de 2 segundos
unsigned long inicio, fin;
float voltaje = 0;
// Umbrales de niveles
float maximo = 4.3;
float minimo = 3.8;
char cadena[255]; // Creamos un array de caracteres de 256 posiciones

// SUPER GLOBALES
bool ESTADO = OFF; // OFF: LED apagado / ON: LED encendido

// CONFIGURACION_____________________________________________
void setup()
{
	/* NOTA: Puerto Serial
		1. El puerto serial fisico (Serial) si se usa para comunicar
		otros dispositivos puede interferir en la subida del codigo,
		es mejor dejarlo solo para depurar, es decir: ver lo que pasa
		en el arduino desde el monitor serial).

		2. El puerto serial virtual (SoftwareSerial) resiste velocidades
		muy altas, es el recomendado para la comunicacion con otros
		dispositvos
	*/

	// BAUDIOS
	Serial.begin(9600);	// Puerto serial fisico
	GPS.begin(38400);	  // Puerto virtual para GPS
	BLUETOOTH.begin(9600); // Puerto virtual para Bluetooth

	// PINES DE ENTRADA
	pinMode(PIN_BOTON, INPUT);

	// PINES DE SALIDA
	pinMode(LED_PANICO, OUTPUT);
	pinMode(PIN_BUZZER, OUTPUT);
	pinMode(LED_VERDE, OUTPUT);
	pinMode(LED_ROJO, OUTPUT);

	Serial.println("__INICIO__");
}

// INICIO_____________________________________________________
void loop()
{
	// MONITOR
	if (digitalRead(PIN_BOTON)) // se monitorea el estado del boton
	{
		// antirrebote: esperar hasta que se estabilice el boton
		while (digitalRead(PIN_BOTON))
		{
			// ESPERANDO...
		}
		/*
			estado toggle: cambia el comportamiento del boton
			cada vez que se pulsa
		*/
		ESTADO = !ESTADO;
	}

	// PROCESO
	if (ESTADO == ON) // PANICO: se detiene todo
	{
		Serial.println("__PARAR__");

		digitalWrite(LED_PANICO, HIGH); // enciende el LED
		digitalWrite(PIN_BUZZER, HIGH); // Encender buzzer

		resetPanico(); // esperamos trama para salir del modo PANICO
	}
	else if (ESTADO == OFF) // NORMAL: todo en funcionamiento
	{
		digitalWrite(LED_PANICO, LOW); // apaga el LED
		digitalWrite(PIN_BUZZER, LOW); // Encender buzzer

		localizador();
		bateria();
	}
}

// FUNCIONES__________________________________________________
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
	if ((fin - inicio) < 6000)
	{
		// NADA
	}
	else
	{
		sw = 0;

		// Intentar recibir secuencia durante
		for (unsigned long start = millis(); (millis() - start) < 10000;)
		{
			while (GPS.available())
			{
				char c = GPS.read();

				// GPS.println("Recibiendo  > " + (String)c + " < desde GPS"); // DEBUG: para el simulador
				Serial.println("caracter = > " + (String)dato + " < recibido");

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

void bateria()
{
	// Leemos valor de la entrada analógica
	analogValor = analogRead(ANALOG_PILA);

	// Obtenemos el voltaje
	voltaje = analogValor * (5.0 / 1023.0); // mas preciso que esto: (0.0044 * analogValor)

	// Dependiendo del voltaje mostramos un LED u otro
	if (voltaje >= maximo)
	{
		Serial.println("Voltaje = " + (String)voltaje + "v"); // mostrando voltaje

		digitalWrite(LED_VERDE, HIGH);
		delay(ledDelay);
		digitalWrite(LED_VERDE, LOW);
		delay(ledDelay);
	}
	else if ((voltaje > minimo) && (voltaje < maximo))
	{
		Serial.println("Voltaje = " + (String)voltaje + "v");

		digitalWrite(LED_ROJO, HIGH);
		delay(ledDelay);
		digitalWrite(LED_ROJO, LOW);
		delay(ledDelay);
	}
}

void resetPanico()
{
	// Esperar trama desde modulo Bluetooth
	while (BLUETOOTH.available() == 0)
	{
		// ESPERANDO...
	}

	if (BLUETOOTH.available() > 0) // Confirmamos si existe un valor en el modulo Bluetooth
	{
		char dato = BLUETOOTH.read(); // leemos el valor y lo asignamos a la variable dato

		// BLUETOOTH.println("Recibiendo  > " + (String)dato + " < desde BLUETOOTH"); // DEBUG: para el simulador
		Serial.println("caracter = > " + (String)dato + " < recibido");

		switch (dato) // comparamos el valor guardado en la variable dato
		{
		case 'a':						   // si el dato leido es 'a'
			digitalWrite(LED_PANICO, LOW); // Reiniciamos el led de panico
			ESTADO = OFF;				   // Reiniciamos el estado del boton
			break;

		case 'b':
			// Otra Accion
			break;
		}

		clean();
		Serial.println("__INICIO__");
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
