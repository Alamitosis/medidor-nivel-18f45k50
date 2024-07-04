/* PROYECTO 1:
   Monitoreo de nivel en un tanque (sensor alto y bajo)

Por: Alan Oswaldo Huerta Becerra*/

/* Configuración del PIC */
#include <18f45k50.h>
#device PASS_STRINGS = IN_RAM
#use delay(internal=48M)
#build(reset=0x02000,interrupt=0x02008)          
#org 0x0000, 0x1FFF { }
#use I2C(MASTER, I2C1, FAST = 500000, stream = SSD1306_STREAM)
#include "SSD1306OLED.c"
#include <Imagenes.h>

//Definción de etiquetas.
#define SENSOR_HIGH		PIN_B6
#define SENSOR_LOW		PIN_B7
#define BOMBA			PIN_B2
#define MODO			PIN_D7
#define BOMBA_ON		PIN_D6
#define BOMBA_OFF		PIN_D4

//Declaración de funciones.
void tuberias();
void noflujo_bomba();
void bomba_encendida();
void Tanque_lleno();
void Tanque_bajo();
void control_bomba_manual();

//Declaración de variable selec=0 modo:automático, selec=1 modo: manual.
unsigned int selec=0;

//Inicio de función principal.
void main(void) {	

	output_float(SENSOR_HIGH);		//Sensor nivel alto
	output_float(SENSOR_LOW);		//Sensor nivel bajo
	output_float(MODO);				//Botón operación auto/manual
	output_float(BOMBA_ON);			//Botón para encender la bomba (disponible únicamente en modo manual)
	output_float(BOMBA_OFF);		//Botón para apagar la bomba (disponible únicamente en modo manual)
	output_drive(BOMBA);			//Pin de salida donde se coloca la bomba

  	// Initialize the SSD1306 OLED with an I2C addr = 0x7A (default address)
  	SSD1306_Begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	SSD1306_ClearDisplay();			//Limpia la pantalla OLED

	//Portada
	output_low(BOMBA);
	SSD1306_ROMBMP(0,0, &Portada_Oled, 128, 64);
	SSD1306_DrawText(15,3,"INST TEC LAZARO C",1);
	SSD1306_DrawText(48,17,"AP. MICROS",1);
	SSD1306_DrawText(45,27,"PROYECTO U1:",1);
	SSD1306_DrawText(35,35,"MONITOREO NIVEL",1);
	SSD1306_DrawText(35,47,"DAYANA GOMEZ B",1);
	SSD1306_DrawText(55,56,"19560091",1);
	SSD1306_Display();				//Despliega en la pantalla los cambios realizados en ella
	delay_ms(5000);					//hasta este punto
	SSD1306_ClearDisplay();
	
	tuberias(); 					//Función encargada de trazar las rectas de las tuberías.

  	for(;;){						//Ciclo infinito for

		SSD1306_DrawLine(0,8,127,8,TRUE);			 // Líneas de separación para encabezado	
		SSD1306_DrawLine(0,10,127,10,TRUE);
		delay_us(200);


		if (selec==1 ){											// MODO: MANUAL
			
			SSD1306_FillRect(0,0,128,8,FALSE);
			SSD1306_DrawText(2,20,"MANU",1);

			if (!input(SENSOR_HIGH) && !input(SENSOR_LOW)){		//Si NH=0 y NL=0,tanque vacío
				SSD1306_DrawText(31,0,"TANQUE VACIO",1);
				SSD1306_ROMBMP(75,26, &Tanque_vacio, 52, 40);	//Animación de tanque vacío
				SSD1306_Display();
				control_bomba_manual();							//Control de encendido o apagado de la bomba
			}

			if (input(SENSOR_HIGH) && input(SENSOR_LOW)){		//Si NH=1 y NL=1, tanque lleno
				SSD1306_DrawText(33,0,"NIVEL ALTO",1);
				Tanque_lleno();									//Animación de tanque lleno
				control_bomba_manual();							//Control de encendido o apagado de la bomba
			}													

			if (!input(SENSOR_HIGH) && input(SENSOR_LOW)){		//Si NH=0 y NL=1, tanque bajo
				SSD1306_DrawText(33,0,"NIVEL BAJO",1);
				Tanque_bajo();									//Animación de tanque bajo
				control_bomba_manual();							//Control de encendido o apagado de la bomba
			}

			if(input(SENSOR_HIGH) && !input(SENSOR_LOW)){		//Si NH=1 y NL=0, falla en sensores
				SSD1306_DrawText(19,0,"REVISAR SENSORES",1);
				SSD1306_ROMBMP(75,26, &Tanque_idk, 52, 40);		//Animación de nivel desconocido en tanque
				SSD1306_Display();
				control_bomba_manual();							//Control de encendido o apagado de la bomba
			}	
		}


		if(selec==0){											//MODO: AUTOMÁTICO
			SSD1306_FillRect(0,0,128,8,FALSE);
			SSD1306_DrawText(2,20,"AUTO",1);

			if (!input(SENSOR_HIGH) && !input(SENSOR_LOW)){		//Si NH=0 y NL=0, tanque vacío
				SSD1306_DrawText(31,0,"TANQUE VACIO",1);
				bomba_encendida();								//Animación bomba encendida
				SSD1306_ROMBMP(75,26, &Tanque_vacio, 52, 40);	//Animación tanque vacío
				output_high(BOMBA);								//pin BOMBA=1
				SSD1306_Display();
				delay_us(300);
			}	
			else if (input(SENSOR_HIGH) && input(SENSOR_LOW)){	//Si NH=1 y NL=1, tanque lleno
				SSD1306_DrawText(33,0,"NIVEL ALTO",1);
				noflujo_bomba();								//Animación bomba apagada
				Tanque_lleno();									//Animación tanque lleno
				output_low(BOMBA);								//pin BOMBA=0
			}

			else if (!input(SENSOR_HIGH) && input(SENSOR_LOW)){	//Si NH=0 y NL=1, tanque bajo
				SSD1306_DrawText(33,0,"NIVEL BAJO",1);
				Tanque_bajo();									//Animación tanque bajo
			}

			else if(input(SENSOR_HIGH) && !input(SENSOR_LOW)){	//Si NH=1 y NL=0, falla en sensores								
				noflujo_bomba();								//Animación bomba apagada
				output_low(BOMBA);								//pin BOMBA=0
				SSD1306_ROMBMP(75,26, &Tanque_idk, 52, 40);		//Animación de nivel desconocido en tanque
				SSD1306_DrawText(19,0,"REVISAR SENSORES",1);
				SSD1306_Display();	
			}
	}
		
		if(!input(MODO)){										//Botón de selección de modo manual o automático
			selec++;											//Incremento de variable en una unidad
			if(selec>1){selec=0;}								//Si selec es mayor a 1, asigna valor de 0
			if(selec==1){
				noflujo_bomba();
				output_low(BOMBA);}								//Si selec=1, entra modo MANUAL y se apaga la bomba
			delay_us(200);
			while(!input(MODO));
		}

		
	}
}

void tuberias(){												//Función para construir líneas de tuberías
		SSD1306_DrawLine(3,50,3,63,TRUE);
		SSD1306_DrawLine(7,54,7,63,TRUE);
		SSD1306_DrawLine(7,54,15,54,TRUE);
		SSD1306_DrawLine(3,50,15,50,TRUE);
		SSD1306_DrawLine(29,50,35,50,TRUE);
		SSD1306_DrawLine(29,54,39,54,TRUE);

		SSD1306_DrawLine(35,50,35,18,TRUE);
		SSD1306_DrawLine(35,18,96,18,TRUE);
		SSD1306_DrawLine(96,18,96,25,TRUE);

		SSD1306_DrawLine(39,54,39,22,TRUE);
		SSD1306_DrawLine(39,22,92,22,TRUE);
		SSD1306_DrawLine(92,22,92,25,TRUE);

		SSD1306_DrawCircle(22, 52, 6);
		SSD1306_DrawLine(19,59,15,63,TRUE);
		SSD1306_DrawLine(25,59,29,63,TRUE);
		SSD1306_DrawLine(16,63,28,63,TRUE);
		SSD1306_DrawText(62,38,"NH",1);
		SSD1306_DrawText(62,55,"NL",1);
}

void noflujo_bomba(){										//Función para representar el NO flujo de agua
		SSD1306_DrawLine(5,52,5,63,FALSE);					//en tuberías y bomba apagada
		SSD1306_DrawLine(5,52,15,52,FALSE);					
		SSD1306_DrawLine(29,52,37,52,FALSE);
		SSD1306_DrawLine(37,52,37,20,FALSE);
		SSD1306_DrawLine(37,20,94,20,FALSE);
		SSD1306_DrawLine(94,20,94,25,FALSE);

		SSD1306_FillCircle(22, 52, 5,FALSE);
		SSD1306_DrawLine(22,43,22,44,FALSE);
		SSD1306_DrawLine(17,43,18,44,FALSE);
		SSD1306_DrawLine(26,44,27,43,FALSE);
		SSD1306_DrawLine(15,46,16,47,FALSE);
		SSD1306_DrawLine(28,47,29,46,FALSE);
		SSD1306_DrawLine(16,57,15,58,FALSE);
		SSD1306_DrawLine(28,57,29,58,FALSE);
}

void bomba_encendida(){										//Función para representar el flujo de agua
		SSD1306_DrawLine(5,52,5,63,TRUE);					//en tuberías y bomba encendida
		SSD1306_DrawLine(5,52,15,52,TRUE);
		SSD1306_DrawLine(29,52,37,52,TRUE);
		SSD1306_DrawLine(37,52,37,20,TRUE);
		SSD1306_DrawLine(37,20,94,20,TRUE);
		SSD1306_DrawLine(94,20,94,25,TRUE);

		SSD1306_FillCircle(22, 52, 6,TRUE);
		SSD1306_DrawLine(22,43,22,44,TRUE);
		SSD1306_DrawLine(17,43,18,44,TRUE);
		SSD1306_DrawLine(26,44,27,43,TRUE);
		SSD1306_DrawLine(15,46,16,47,TRUE);
		SSD1306_DrawLine(28,47,29,46,TRUE);
		SSD1306_DrawLine(16,57,15,58,TRUE);
		SSD1306_DrawLine(28,57,29,58,TRUE);
		return;
}

void Tanque_lleno(){										//Despliegue de imágenes que simulan
		SSD1306_ROMBMP(75,26, &Tanque_lleno1, 52, 40);		//el nivel lleno del tanque
		SSD1306_Display();
		SSD1306_ROMBMP(75,26, &Tanque_lleno2, 52, 40);
		SSD1306_Display();
		delay_ms(20);
}

void Tanque_bajo(){											//Despliegue de imágenes que simulan
		SSD1306_ROMBMP(75,26, &Tanque_bajo1, 52, 40);		//el nivel bajo del tanque
		SSD1306_Display();
		SSD1306_ROMBMP(75,26, &Tanque_bajo2, 52, 40);
		SSD1306_Display();
		delay_ms(20);
}

void control_bomba_manual(){
		if(!input(BOMBA_ON)){
		bomba_encendida();
		SSD1306_Display();
		output_high(BOMBA);
		}
		else if(!input(BOMBA_OFF)){
		noflujo_bomba();
		SSD1306_Display();
		output_low(BOMBA);						
		}
		delay_us(200);
}