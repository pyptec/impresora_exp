#include<verprint.h>

/*funciones prototipo externas */
extern unsigned char  ValidaSensoresPaso(void);
extern void Debug_txt_Tibbo(unsigned char * str);
extern void PantallaLCD(unsigned char cod_msg);
extern void send_portERR(unsigned char cod_err);

/*variables externas*/
extern unsigned char g_cEstadoComSoft;
extern unsigned char g_cEstadoImpresion;
extern unsigned char ValTimeOutCom;
extern unsigned char buffer_ready;

/*----------------------------------------------------------------------------
tiempo de delay entre funciones
------------------------------------------------------------------------------*/

#define 	TIME_CARD					20		//50
#define 	TIME_RX						70		//
#define 	TIME_PLACA				55
#define 	TIME_PULSADOR			10

/*----------------------------------------------------------------------------
definicion de recepcion serial 
------------------------------------------------------------------------------*/

#define  ESPERA_RX 					0  					//espera el primer cmd de recepcion del verificado 
/*----------------------------------------------------------------------------
Definiciones de sequencias de lectura de ticket 
------------------------------------------------------------------------------*/

#define SEQ_INICIO						0X00	
#define SEQ_LEECODIGO					0X01

/*---------------------------------------------------------------------------------
definiciones de la pantalla
-----------------------------------------------------------------------------------*/
#define INGRESO									0xFA
#define BIENVENIDO							0XFE
#define AUDIO_IN							  0XA0
#define AUDIO_ENTER				      0XA1

/*-------------------------------------------------------------------------------------------------------------------------
procedimiento que lee el codigo de barra o el QR
SEQ_INICIO=00 se detecta lapresencia vehicular 
SEQ_QUEST_PRINT=1 si fue  presionado el boton 
---------------------------------------------------------------------------------------------------------------------------*/

void Lee_ticket(void)
{
	static unsigned char paso_una_vez=0;
	switch (g_cEstadoImpresion)
	{
		case SEQ_INICIO:
		
			
		if (ValTimeOutCom==1)																																	/*tiempo de espera */
		{
			if (ValidaSensoresPaso()!=0)																												/*pregunto q alla presencia vehicular*/
			{
				Debug_txt_Tibbo((unsigned char *) "Vehiculo en el loop\r\n\r\n");									/* se encuentra un sensor activo*/
				PantallaLCD(BIENVENIDO);
				if(paso_una_vez==0)
				{
				
				send_portERR(AUDIO_IN);																														/*habilito el audio de entrada*/
				paso_una_vez=1;
				}														
				ValTimeOutCom=TIME_RX;
				buffer_ready=0;																																		/* buffer del pto serie (0) inicia a esperar la trama*/
				g_cEstadoComSoft=ESPERA_RX;		
				g_cEstadoImpresion=SEQ_INICIO;																										/*volvemos a preguntar	*/	
							
				
			
			}
			else
				{
				Debug_txt_Tibbo((unsigned char *) "Vehiculo no en el loop\r\n\r\n");							/* no hay vehiculo en el loop*/
				paso_una_vez=0;																								
				ValTimeOutCom=TIME_PULSADOR;
				buffer_ready=0;																																		/* buffer del pto serie (0) inicia a esperar la trama*/
				g_cEstadoComSoft=ESPERA_RX;		
				g_cEstadoImpresion=SEQ_INICIO;																										/*volvemos a preguntar	*/	
							
				}
		}
		break;
/*-------------------------------------------------------------------------------------------------------------------------------------------
SEQ_LEECODIGO	
lee el dato en el pto serial del codigo qr
----------------------------------------------------------------------------------------------------------------------------------------------*/		
		case SEQ_LEECODIGO:
			if ((ValTimeOutCom==1)||(buffer_ready!=0))
			{
			}
		
		default:
		g_cEstadoImpresion=SEQ_INICIO;
		break;
	}
}