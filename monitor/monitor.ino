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
SoftwareSerial SerialGPS(4, 3); // Modulo GPS (RX = PIN 4 | TX = PIN 3)

// VARIABLES
String lati, lon, coor;
int dato;  // Se crea la variable dato tipo String
int i = 0; // Tamaño actual del array
int sw = 0;
int analogValor = 0;
unsigned long inicio, fin;
float voltaje = 0;
// Umbrales de niveles
float maximo = 4.3;
float minimo = 3.8;
char cadena[255]; // Creamos un array de caracteres de 256 posiciones

// SUPER GLOBALES
bool ESTADO = OFF; // OFF: LED apagado / ON: LED encendido
unsigned long START;

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
	SerialGPS.begin(9600); // Puerto virtual para GPS
	Serial.begin(38400);   // Puerto fisico para Bluetooth

	// PINES DE ENTRADA
	pinMode(PIN_BOTON, INPUT);

	// PINES DE SALIDA
	pinMode(LED_PANICO, OUTPUT);
	pinMode(PIN_BUZZER, OUTPUT);
	pinMode(LED_VERDE, OUTPUT);
	pinMode(LED_ROJO, OUTPUT);

	// INICIALIZAR PINES
	digitalWrite(LED_PANICO, LOW);
	digitalWrite(PIN_BUZZER, LOW);
	digitalWrite(LED_VERDE, LOW);
	digitalWrite(LED_ROJO, LOW);

	// INTERRUCCION
	// RISING, Dispara en el flanco de subida (Cuando pasa de LOW a HIGH)
	attachInterrupt(0, activacionBoton, RISING);
}

// INICIO_____________________________________________________
void loop()
{
	// PROCESO
	if (ESTADO == ON) // PANICO: se detiene todo
	{
		detachInterrupt(0); // suspendemos la interruccion 0
		resetPanico();		// esperamos trama para salir del modo PANICO
	}
	else if (ESTADO == OFF) // NORMAL: todo en funcionamiento
	{
		localizador();
		bateria();
	}
}

// FUNCIONES__________________________________________________

// En esta funcion NO SE PUEDE USAR: delay, serial, llamar otras funciones
void activacionBoton()
{
	digitalWrite(PIN_BUZZER, HIGH); // Encender buzzer
	digitalWrite(LED_PANICO, HIGH); // Apagar LED
	ESTADO = ON;
	START = 2000;
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
	if ((fin - inicio) < 6000)
	{
		// NADA
	}
	else
	{
		sw = 0;
		// Intentar recibir secuencia durante
		for (START = millis(); (millis() - START) < 2000;)
		{
			while (SerialGPS.available())
			{
				char c = SerialGPS.read();

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
		digitalWrite(LED_VERDE, HIGH);
		digitalWrite(LED_ROJO, LOW);
	}
	else if ((voltaje > minimo) && (voltaje < maximo))
	{
		digitalWrite(LED_VERDE, LOW);
		digitalWrite(LED_ROJO, HIGH);
	}
}

void resetPanico()
{
	// Esperar trama desde modulo Bluetooth
	while (Serial.available() == 0)
	{
		// ESPERANDO...
	}

	if (Serial.available() > 0) // Confirmamos si existe un valor en el modulo Bluetooth
	{
		char dato = Serial.read(); // leemos el valor y lo asignamos a la variable dato

		// Serial.println("caracter: = " + (String)dato + " recibido");

		switch (dato) // comparamos el valor guardado en la variable dato
		{
		// si el dato leido es 'a'
		case 'a':
			digitalWrite(PIN_BUZZER, LOW);				 // Apagar buzzer
			digitalWrite(LED_PANICO, LOW);				 // Apagar led panico
			ESTADO = OFF;								 // Reiniciar estado del boton
			attachInterrupt(0, activacionBoton, RISING); // Reanudamos la interruccion 0
			break;

		// si el dato leido es 'b'
		case 'b':
			// Otra Accion
			break;
		}

		clean();
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
