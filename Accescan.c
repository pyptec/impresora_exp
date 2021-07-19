#include <Accescan.h>
#include <reg51.h>
#include <string.h>
extern void Block_write_clock_ascii(unsigned char *datos_clock);
extern void PantallaLCD(unsigned char cod_msg);
extern void Reloj_Pantalla_Lcd();
extern unsigned char rx_Data(void);
extern void Debug_txt_Tibbo(unsigned char * str);
extern void DebugBufferMF(unsigned char *str,unsigned char num_char,char io);
extern void Debug_Dividir_texto();
extern void tx_aux(unsigned char caracter);
extern unsigned char Dir_board();
extern void Block_read_clock_ascii(unsigned char *datos_clock);
extern void Two_ByteHex_Decimal(unsigned char *buffer,unsigned char id_h,unsigned char id_l);
extern void ByteHex_Decimal(unsigned char *buffer,unsigned char valorhex);
extern void PantallaLCD(unsigned char cod_msg);
extern void LCD_txt (unsigned char * msg,char enable_char_add );
extern unsigned char  ValidaSensoresPaso(void);
extern void tx_aux(unsigned char caracter);
extern void PantallaLCD_LINEA_2(unsigned char cod_msg, unsigned char *buffer);
extern unsigned char hex_bcd (unsigned char byte);
extern void Trama_pto_Paralelo_P(unsigned char *buffer_S1_B0,unsigned char *buffer_S1_B2,unsigned char cmd);
extern void  send_port(unsigned char *buffer_port, unsigned char length_char);
extern void graba_serie(unsigned char *buffer);
extern void graba_serie_ascii_hex(unsigned char *buffer);
unsigned char rd_eeprom (unsigned char control,unsigned int Dir); 
extern unsigned long int  Leer_serie ();
extern void serie_ascii_siceros_l(unsigned char *serie);
extern void Cmd_LPR_Salida_wiegand(unsigned char *buffer);

extern unsigned char Timer_wait;
extern unsigned char Tipo_Vehiculo;
extern unsigned char USE_LPR;
extern unsigned char  Debug_Tibbo;
extern unsigned char xdata Buffer_Tibbo_rx[];
extern unsigned char ValTimeOutCom;		

sbit rx_ip = P0^0;				
sbit lock = P1^7;						//Relevo 
sbit Atascado = P0^3;				//Rele de on/off del verificador o transporte
sbit led_err_imp = P0^2;			//Error 	


#define STX											02 
#define ETX											03 
#define LOTE_FULL								05
#define FUERA_DE_LINEA					0xb6
#define ON_LINE									0xAA

#define PRMR_NO_MENSUAL 						0XB1
#define PRMR_NO_IN_PARK 						0XB2
#define PRMR_IN_PARK								0XB3
#define PRMR_EXPIRO									0XB4
#define PRMR_MENSUAL_FUERA_HORARIO	0XB5
#define	PRMR_GRACIAS								'V'

/*mensajes de pantalla*/
#define BIENVENIDO							0XFE
#define SIN_PAGO								0XE7
#define LECTURA_WIEGAND					92

#define GRACIAS									91		
#define NO_MENSUAL_NI_PREPAGO		96
#define	NO_IN_PARK							93
#define MENSUAL_FUERA_HORARIO		98
#define IN_PARK									99
#define EXPIRO									94
#define BIENVENIDO_WIEGAN				0x9b

//unsigned char S1_B2[]={0x13, 0x03, 0x1D, 0x0B, 0x0E, 00, 00, 00, 00, 00, 0x01, 0x13, 0x03, 0x1D, 0x0E, 0x1D};
//unsigned char S1_B0[]={0x32, 0x31, 0x30, 0x37, 0x31, 0x35, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01};
//unsigned char S_B[]={0xE7, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01};
/*------------------------------------------------------------------------------
Rutina q valida los cmd del pto paralelo
------------------------------------------------------------------------------*/
void Valida_Trama_Pto(unsigned char *buffer, unsigned char length_trama)
{
	 unsigned char buff[11];
	/*-------------------------------CMD H reloj para el board y la pantalla lcd------------------------------------------*/
		if((length_trama==25)&&(*buffer==STX)&&(*(buffer+2)=='H')&&*(buffer+(length_trama-1))==ETX)													/*cmd de Accescan que me envia el reloj actualizado*/
		{ 
			
			Block_write_clock_ascii(buffer+3);																																								/* se escribe el reloj de hardware*/
		
			Reloj_Pantalla_Lcd();																																															/* Escribo el reloj en la pantalla lcd*/
		
		}
			/*-------------------------------CMD 05 lote full -----------------------------------------------------------------*/
		else if ((*buffer==LOTE_FULL)	)																																								/*ok si llega msj a;96;NO ES MENSUALIDAD NI PREPAGO<LF>*/
		{
			PantallaLCD(LOTE_FULL);																																														/*MSJ MENSUAL NO EN PARQUEADERO*/
		}		
			/*-------------------------------CMD B1 PRMR_NO_MENSUAL_NI PREPAGO -----------------------------------------------------------------*/
		else if ((*buffer==PRMR_NO_MENSUAL)	)																																								/*ok si llega msj a;96;NO ES MENSUALIDAD NI PREPAGO<LF>*/
		{
			PantallaLCD(NO_MENSUAL_NI_PREPAGO);																																														/*MSJ MENSUAL NO EN PARQUEADERO*/
		}		
			/*-------------------------------CMD B2 PRMR_NO_IN_PARK   -----------------------------------------------------------------*/
		else if ((*buffer==PRMR_NO_IN_PARK)	)																																									/*ok MSJ MENSUAL NO EN PARQUEADERO  */
		{
			PantallaLCD(NO_IN_PARK);																																														/*MSJ MENSUAL NO EN PARQUEADERO*/
		}	
		/*-------------------------------CMD B3 PRMR_IN_PARK   -----------------------------------------------------------------*/
		else if ((*buffer==PRMR_IN_PARK)	)																																										/*ok msj MENSUAL ESTA EN PARQUEADER*/
		{
			PantallaLCD(IN_PARK);																																														/*MSJ MENSUAL ESTA EN PARQUEADER*/
		}			
		/*-------------------------------CMD B4  EXPIRO mensualidad vencida ----------------------------------------------------------*/
		else if ((length_trama==1)&&(*buffer==PRMR_EXPIRO))																																		/*ok msj mensual vencida */
		{
				 PantallaLCD(EXPIRO);																																															/*mesualidad vencida*/
		}
		/*-------------------------------CMD B5  HORARIO mensualidad FUERA DE HORARIO ----------------------------------------------------------*/
		else if ((length_trama==1)&&(*buffer==PRMR_MENSUAL_FUERA_HORARIO))																										/* */
		{
				 PantallaLCD(MENSUAL_FUERA_HORARIO);																																							/*ok msj mesualidad fuera de horario*/
		}
		
		/*-------------------------------CMD B6 fuera de linea -------------------------------------------------------------*/
		else if(*buffer==FUERA_DE_LINEA)																																										/*cmd de Accescan que dice q esta fuera de linea*/
		{
			if (Timer_wait>=20)																																																/* se envia el msj fuera de linea*/
			{																																																									/* con un tiempo de retardo =((1/(22118400/12)*65535)*30)*/
				PantallaLCD(FUERA_DE_LINEA);
				led_err_imp=0;																																																	/*error led on*/
				Timer_wait=0;
				lock=0;																																																					/*relevo off despues de 1 minuto*/
				Atascado=0;
				ValTimeOutCom=10;				
			}
			if ((Debug_Tibbo==0)&&(USE_LPR==1))
				{
				Debug_Tibbo=1;
				Debug_txt_Tibbo((unsigned char *) "LIVE\n");
			
				Debug_Tibbo=0;
				}
		}
		/*-------------------------------CMD AA en linea ------------------------------------------------------------------*/
		else if (*buffer==ON_LINE)																																													/*en linea*/
		{
			
			if (Timer_wait>=20)																																																/* se envia el msj fuera de linea*/
			{	
				Timer_wait=0;																																																		/*se inicia el timer*/
				lock=0;
				led_err_imp=1;																																																	/*relevo off despues de 1 minuto*/
				Atascado=0;	
				ValTimeOutCom=10;
				if ((Debug_Tibbo==0)&&(USE_LPR==1))
				{
				Debug_Tibbo=1;
				Debug_txt_Tibbo((unsigned char *) "LIVE\n");
				
				Debug_Tibbo=0;
				}
			}
			
		}	
		/*-------------------------------CMD 'V'=PRMR_GRACIAS  msj Gracias y nombre del mensual-----------------------------------------*/
		else if ((length_trama == 0x13)&& (*(buffer+1)==PRMR_GRACIAS)&&*(buffer+(length_trama-1))==ETX)													/* */
		{
				 *(buffer+(length_trama-1))=0;
				 PantallaLCD_LINEA_2(GRACIAS,buffer+2);																																			/*SE ENVIA EL MSJ GRACIAS lo q envia el software*/
		}
		/*-------------------------------CMD 'O'=PRMR_BIENVENIDO  msj BIENVENIDO  nombre del mensual-----------------------------------------*/
		else if ((length_trama==19)&&(*buffer==STX)&&(*(buffer+1)=='O')&&*(buffer+(length_trama-1))==ETX)										/*mensaje de bienvenidos*/
		{
 			*(buffer+(length_trama-1))=0;
				 PantallaLCD_LINEA_2(BIENVENIDO_WIEGAN,buffer+2);				
 		}
		else if ((length_trama==1)&&(*buffer==0xA1))																																				/*cmd 0xA1 audio caja que es igual a no registra pago */
		{
				 PantallaLCD(SIN_PAGO);
		}
							/*-------------------------------CMD de wiegand---------------------------------------------------*/
		else if ((length_trama==6)&&(*buffer==STX)&&(*(buffer+1)=='W')&&*(buffer+(length_trama-1))==ETX)										/* cmd q comunica con monitor po wigan*/
		{
			/*-------------------------------se covierte el numero serie de la tarjeta------------------------------*/
							ByteHex_Decimal(buff,*(buffer+2));																																				/*convierto el primer byte_hex a decimal		*/
							buff[3]=' ';
							Two_ByteHex_Decimal(buff+4,*(buffer+3),*(buffer+4))	;		
				if (USE_LPR==1)
				{
																																	/*convierto un byte de 16 bits a decimal*/																									
						
							PantallaLCD_LINEA_2(LECTURA_WIEGAND,buff);
																																																												/*transmito el codigo de la tarjeta a la pantalla lcd*/
							/*--------------------------------------------------------------------------------------------------------*/
					
						//	while(!ValidaSensoresPaso());
								
							Cmd_LPR_Salida_wiegand(buff);
				}																																							
				
				else
				{
					 /*-------------------------------mensaje en la pantalla---------------------------------------------------*/
																												/*convierto un byte de 16 bits a decimal*/																									
							PantallaLCD_LINEA_2(LECTURA_WIEGAND,buff);																														/*transmito el codigo de la tarjeta a la pantalla lcd*/
																																																		
					/*--------------------------------------------------------------------------------------------------------*/	
				
				}
		}
		else if ((length_trama==12)&&(*buffer==STX)&&*(buffer+(length_trama-1))==ETX)																		/*graba el numero de consecutivo en eeprom*/
		{
			graba_serie_ascii_hex(buffer+1);
			
			
				Debug_txt_Tibbo((unsigned char *) "Grabacion Serie: ");																											/*trama recibida pto paralelo */
				DebugBufferMF(buffer,length_trama,1);
				
			
		}
}

