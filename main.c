/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "math.h"
#include "stdbool.h"
#include "stdlib.h"
#include <stdio.h>
#include "RPi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct roomStruct{
	float temp;
	float hum;
	uint16_t co2;
	uint16_t people;
	float Vflow;
	uint16_t valveOpeningDegree;
	float exchange;
	bool ventilate;
} room1, roomPrew1, script1 , script2, room2, roomPrew2;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define S1 "AT+BIND=21,13,005FA6\r\n" //sens 
#define S2 "AT+BIND=19,8,3650F0\r\n" //valve 2
#define S3 "AT+BIND=21,13,004BEB\r\n" //valve 1
#define S4 "AT+BIND=21,13,008DAB\r\n" //sens
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t trans_str[100]; //string to data uart
uint16_t ventSpeed = 0;
uint16_t prewVentSpeed = 0;
uint8_t want = -1;
float U0 = 3.6, U1 = 0, U2 = 0;
float Uout = 0;
float k = 0.96, n = 0.5;
float adc;

//param for RX data RPi
bool flagRXPi = 0;
uint8_t RXbufPi[5] = {0,};
uint8_t RXdataPi[100] = {0,};
uint8_t posRXPi = 0;

//param for RX data ESP
bool flagRXEsp = 0;
uint8_t RXbufEsp[5] = {0,};
uint8_t RXdataEsp[100] = {0,};
uint8_t posRXEsp = 0;

bool flagCheck = 0;

float a = 0;
//param for RX data V2
uint8_t posV2 = 0;
uint8_t bufV2[30] = {0,};
uint8_t fakebufV2[3] = {0,};

bool flagRXV2 = 0;
bool flagStatusOK = 0;
bool flagInitAT = 0;

//param for USB
uint8_t posUSB = 0;
uint8_t bufUSB[30] = {0,};
uint8_t fakebufUSB[3] = {0,};
bool flagRXUSB = 0;

//param for valve1
uint8_t posV1 = 0;
uint8_t bufV1[30] = {0,};
uint8_t fakebufV1[3] = {0,};
bool flagRXV1 = 0;
bool flagEndVentilation = 0;;
			int v1 = 1;
			int v2 = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void parseValve1(void);
void setValveRoom1(void);

void parseValve2(void);
void setValveRoom2(void);

void parseSensors1(void);
void parseSensors2(void);

void checkRoom1(void);
void checkTempRoom1(void);
void checkHumRoom1(void);
void checkCO2Room1(void);

void getSensorsRoom1(void);
void getADCRoom1(void);

void co2ExchangeRoom1(void);
void peopleExchangeRoom1(void);

void setVent(void);

void checkRoom2(void);
void checkTempRoom2(void);
void checkHumRoom2(void);
void checkCO2Room2(void);

void getSensorsRoom2(void);
void getADCRoom2(void);

void co2ExchangeRoom2(void);
void peopleExchangeRoom2(void);

void AT_ON(void);
void AT_OFF(void);
void parseStatus(void);
void sendV2(char* data);
void connectBT(char* addr);
void parseBT();

void parsePeople(void);
void sendRPi(char* data);

void sendUSB(char* data);
void sendV1(char* data);

void ventilateRoom1();
void ventilateRoom2();
void ventilateAllRooms();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void ventilateRoom1(){
	prewVentSpeed = ventSpeed;
	ventSpeed = 3;
	setVent();
	//roomPrew1.valveOpeningDegree = room1.valveOpeningDegree;
	//room1.valveOpeningDegree = 3;
	//setValveRoom1();
	HAL_TIM_Base_Start_IT(&htim4);
	
}
void ventilateRoom2(){
	prewVentSpeed = ventSpeed;
	ventSpeed = 3;
	setVent();
	//roomPrew2.valveOpeningDegree = room2.valveOpeningDegree;
	//room2.valveOpeningDegree = 3;
	//setValveRoom2();
	HAL_TIM_Base_Start_IT(&htim4);
	
}
void ventilateAllRooms(){
	prewVentSpeed = ventSpeed;
	ventSpeed = 3;
	setVent();
	//roomPrew2.valveOpeningDegree = room2.valveOpeningDegree;
	//roomPrew1.valveOpeningDegree = room1.valveOpeningDegree;
	//room1.valveOpeningDegree = 3;
	//setValveRoom1();
	//room2.valveOpeningDegree = 3;
	//setValveRoom2();
	HAL_TIM_Base_Start_IT(&htim4);
}

void parseBT(){
	if (bufV2[0] == '1') {	//sens1 
		sendUSB("PARSE sens1\r\n");
		//parseSensors1();
	}
	if (bufV2[0] == '2') {	//valve2
		sendUSB("PARSE VALVE1\r\n");
		parseValve1();
	}
	if (bufV2[0] == '4') {	//sens2 
		sendUSB("PARSE sens2\r\n");
		parseSensors2();
	}
	if (bufV2[0] == '3') {	//valve1 
		sendUSB("PARSE VALVE2\r\n");
		parseValve2();
	}
	memset(bufV2,0,strlen(bufV2));
	strcmp(bufV2, "");
	posV2 = 0;
}

void parseStatusV1(){
	if (bufV1[0] == 'O'){
		sendUSB("STATUS OK\r\n");
		flagStatusOK = 1;
	}
	if (bufV1[0] == 'E'){
		sendUSB("STATUS ERROR\r\n");
		flagStatusOK = 0;
	}
	memset(bufV1,0,strlen(bufV1));
	posV1= 0;	
}


void parseStatus(){
	if (bufV2[0] == 'O'){
		sendUSB("STATUS OK\r\n");
		flagStatusOK = 1;
	}
	if (bufV2[0] == 'E'){
		sendUSB("STATUS ERROR\r\n");
		flagStatusOK = 0;
	}
	memset(bufV2,0,strlen(bufV2));
	posV2 = 0;	
}

void AT_ON(){
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);//en = 0;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);//key = 1;
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);//en = 1;	
	sendUSB("AT_ON\r\n");
}

void AT_OFF(){
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);//en = 0;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);//key = 1;
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);//en = 1;	
	sendUSB("AT_OFF\r\n");
}

void connectBT(char* addr){
	AT_ON();
	flagInitAT = 0;
	flagRXV2 = 0;
	sendV1(addr);
	int i=0;
	while ((flagStatusOK == 0)&&(i < 10)){
		if (flagRXV2 == 1){
			parseStatus();
			flagRXV2 = 0;
		}
		sendV1(addr);
		i++;
		HAL_Delay(1000);
	}
	if (flagStatusOK == 1){
		sendUSB("BIND OK\r\n");
		sendUSB(addr);
		AT_OFF();
		flagStatusOK = 0;
	}
	else {
		sendUSB("BIND NOT OK\r\n");
		sendUSB(addr);
		AT_OFF();
		flagStatusOK = 0;
	}
}

void sendV2(char* data){
	HAL_UART_Transmit(&huart2,(uint8_t*)data,strlen(data),0xFFFF);
}

void sendUSB(char* data){
	HAL_UART_Transmit(&huart3,(uint8_t*)data,strlen(data),0xFFFF);
}

void sendV1(char* data){
	HAL_UART_Transmit(&huart5,(uint8_t*)data,strlen(data),0xFFFF);
}

void sendESP(char* data){
	HAL_UART_Transmit(&huart4,(uint8_t*)data,strlen(data),0xFFFF);
}

void sendRPi(char* data){
	HAL_UART_Transmit(&huart6,(uint8_t*)data,strlen(data),0xFFFF);
}

void parseValve1(){
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t adcValve[10] = {0,};
	adcValve[0] = -1;
	while (bufV1[j] != ';') {
			adcValve[i] = bufV1[j];
			i++;
			j++;
	}
	adc = atof(adcValve);
	Uout = (adc / 1000) * 2.67;
	room1.Vflow = pow(((Uout - U0)*(Uout + U0)),((1/n)))/((pow(k, (1/n)))*pow(U0, (2/n)));				
	room1.exchange = room1.Vflow * 3600 * 0.0025; // расчет воздухообмена как произведение скорости потока воздуха на время на площадь сечения клапана/ вентилятора, м3/ч
	snprintf(trans_str, 63, "flow = %.2f exchange = %.2f adc = %.2f\r\n" , room1.Vflow, room1.exchange, adc);
	sendUSB(trans_str);
	memset(bufV1,0,strlen(bufV1));
	posV1 = 0;
}
void parseValve2(){
			uint8_t i = 0;
			uint8_t j = 0;
			float U0 = 3.6, U1 = 0, U2 = 0;
			float Uout = 0;
			float k = 0.96, n = 0.5;
			j++; 
			uint8_t adcValve[10] = {0,};
			while (trans_str[j] != ';') {
					adcValve[i] = trans_str[j];
					i++;
					j++;
			}
			float adc = atof(adcValve);
			Uout = (adc / 1000)*3;
			room2.Vflow = pow(((Uout - U0)*(Uout + U0)),((1/n)))/((pow(k, (1/n)))*pow(U0, (2/n)));				
			room2.exchange = room1.Vflow * 3600 * 0.00785; // расчет воздухообмена как произведение скорости потока воздуха на время на площадь сечения клапана/ вентилятора, м3/ч
}

void parseSensors1(){
			uint8_t i = 0;
			uint8_t j = 0;
			j++; 
			uint8_t temp[10] = {0,};
			uint8_t hum[10] = {0,};
			uint8_t co2[10] = {0,};
			while (bufV1[j] != ' ') {
					temp[i] = bufV1[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			while (bufV1[j] != ' ') {
					hum[i] = bufV1[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			while (bufV1[j] != ';') {
					co2[i] = bufV1[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			if ((atof(temp)) < 0.01 || (atof(temp) > 50 )){
				__nop;
			}
			else{
				room1.temp = atof(temp);
			}
			if ((atof(hum)) < 0.01 || (atof(hum) > 50 )){
				__nop;
			}
			else{
				room1.hum = atof(hum);
			}
			if ((atoi(co2)) < 0.01 || (atoi(co2) > 2000 )){
				__nop;
			}
			else{
				room1.co2 = atoi(co2);
			}
			memset(bufV1, 0 ,strlen(bufV1));
			posV1 = 0;
}

void parseSensors2(){
			uint8_t i = 0;
			uint8_t j = 0;
			j++; 
			uint8_t temp[10] = {0,};
			uint8_t hum[10] = {0,};
			uint8_t co2[10] = {0,};
			while (bufV2[j] != ' ') {
					temp[i] = bufV2[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			while (bufV2[j] != ' ') {
					hum[i] = bufV2[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			while (bufV2[j] != ';') {
					co2[i] = bufV2[j];
					i++;
					j++;
			}
			i = 0;
			j++;
			if ((atof(temp)) < 0.01 || (atof(temp) > 50 )){
				__nop;
			}
			else{
				room2.temp = atof(temp);
			}
			if ((atof(temp)) < 0.01 || (atof(temp) > 50 )){
				__nop;
			}
			else{
				room2.hum = atof(hum);
			}
			if ((atof(temp)) < 0.01 || (atof(temp) > 50 )){
				__nop;
			}
			else{
				room2.co2 = atoi(co2);
			}
			/*room2.temp = atof(temp);
			room2.hum = atof(hum);
			room2.co2 = atof(co2);*/
			memset(bufV2, 0 ,strlen(bufV2));
			posV2 = 0;
}
void getSensorsRoom1(){
	int i = 0;
	if ((flagRXV1 == 0)&&(i < 10)){
		sendV1("+0;\r\n");
		sendUSB("+0;\r\n");
		i++;
		HAL_Delay(1000);
	}
}


void setValveRoom1(){
	//int i = 0;
	uint8_t q[30] = {0,};
	snprintf(q, 29, "*%d;\r\n",room1.valveOpeningDegree);
	//HAL_Delay(10);
	sendV1(q);
	sendUSB(q);
	memset(q, 0,strlen(q));
}

void getADCRoom1(){
	int i = 0;
	flagRXV1 = 0;
	snprintf(trans_str, 63, "#999;\r\n");
	while ((flagRXV1 == 0) && (i < 10)){
		sendV1(trans_str);
		//sendUSB(trans_str);
		i++;
		HAL_Delay(2000);
		if (flagRXV1 == 1){
			parseValve1();
		}
	}
	flagRXV1 = 0;
	//snprintf(trans_str, 63, "flow = %.2f exchange = %.2f\r\n" , room1.Vflow, room1.exchange);
	//sendUSB(trans_str);
}

void getSensorsRoom2(){
	//connectBT(S?);
	HAL_Delay(1000);
	if (flagRXV2 == 1){
			parseBT();
			flagRXV2 = 0;
	}
	int i = 0;
	if ((flagRXV2 == 0)&&(i < 10)){
		sendV1("#read;\r\n");
		sendUSB("#read;\r\n");
		i++;
		HAL_Delay(1000);
	}
	if (flagRXV2 == 1){
			parseBT();
			flagRXV2 = 0;
	}
}

void setValveRoom2(){
	int i = 0;
	uint8_t V2[50] = {0,};
	snprintf(V2, 49, "*%d;\r\n",room2.valveOpeningDegree);
	//while ((flagStatusOK == 0) && (i < 10)){
	sendV2(V2);
	sendUSB(V2);
		//if (flagRXV2 == 1){
		//	parseStatusV2();
		//	flagRXV2 = 0 ;
		//}
		//i++;
		//HAL_Delay(1000);
	//}
	//flagStatusOK = 0;
	//flagRXV2 = 0;
	memset(V2, 0,strlen(V2));
}

void getADCRoom2(){
	//connectBT(S?);
	HAL_Delay(1000);
	if (flagRXV2 == 1){
			parseBT();
			flagRXV2 = 0;
	}
	int i = 0;
	if ((flagRXV2 == 0)&&(i < 10)){
		sendV1("#read;\r\n");
		sendUSB("#read;\r\n");
		i++;
		HAL_Delay(1000);
	}
	if (flagRXV2 == 1){
			parseBT();
			flagRXV2 = 0;
	}
}

//send init commands to sensors
/*voAq0.
3id initSensors(){

	room1.valveOpeningDegree = 0;
	setValveRoom1();
	
	room2.valveOpeningDegree = 0;
	setValveRoom2();
	
	getSensorsRoom1();

	getSensorsRoom2();

	ventSpeed = 0;
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
	setVent();
}*/

void setVent(){
	a = 4095/3*ventSpeed;
	HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R, a);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
        if(htim->Instance == TIM1) //10 sec, send actual data to ESP
        {
					/*if (room1.temp > 10000){
						room1.temp = 0;
					}
					else room1.temp++;
					if (room1.hum > 10000){
						room1.hum = 0;
					}
					else room1.hum++;
					if (room1.co2 > 10000){
						room1.co2 = 0;
					}
					else room1.co2++;
					//if (room1.people > 10000){
					//	room1.people = 0;
					//}room1.people++;
					if (room2.temp > 10000){
						room2.temp = 0;
					}
					else room2.temp++;
					if (room2.hum > 10000){
						room2.hum = 0;
					}
					else room2.hum++;
					if (room2.co2 > 10000){
						room2.co2 = 0;
					}
					else room2.co2++;*/
					//if (room2.people > 10000){
					//	room2.people = 0;
					//}else room2.people++;

					//parseSensors1();
					//parseSensors2();
					uint8_t send[100] = {0,};
					snprintf(send, 63, "*%.2f %.2f %d %d %.2f %.2f %d %d %d %d;\r\n",room1.temp, room1.hum, room1.co2, room1.people, room2.temp, room2.hum, room2.co2, room2.people, script1.ventilate, script2.ventilate);
					sendESP(send);
					sendUSB(send);
					script1.ventilate = 1;
					script2.ventilate = 1;
					
					
        }
				if(htim->Instance == TIM2) //60 sec, polling RaspberryPi 
        {
					sendRPi("GET_ALL\r\n");
					sendUSB("GET_ALL\r\n");
        }
				if(htim->Instance == TIM3) //60 sec, chack param
        {
					flagCheck = 1;
        }
				if(htim->Instance == TIM4) //60 sec, chack param
        {
					flagEndVentilation = 1;
					HAL_TIM_Base_Stop(&htim4);
        }
				if(htim->Instance == TIM5) //9 sec. опрос датчиков
        {
					//getSensorsRoom1();
        }
				
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{					
	if(huart == &huart6) {
		RXdataPi[posRXPi] = RXbufPi[0]; 
		posRXPi++;
		if ((posRXPi>0) && ((RXdataPi[posRXPi-1] == ';')|| (posRXPi > 30)))
		{
			flagRXPi = 1;
			//uint8_t a[5] = {0,};
			//snprintf(a , "%d\r\n",flagRXPi);
			sendUSB(RXdataPi);
			//sendUSB(flagRXPi);
		}
		HAL_UART_Receive_IT(&huart6,RXbufPi,1);
	}
	if(huart == &huart4) {
		RXdataEsp[posRXEsp] = RXbufEsp[0]; 
		posRXEsp++;
		if ((posRXEsp>0) && ((RXdataEsp[posRXEsp-1] == ';') || (posRXEsp > 100)))
		{
			flagRXEsp = 1;
			//sendUSB("GET DATA ESP\r\n");
			//sendUSB(RXdataEsp);
		}
		HAL_UART_Receive_IT(&huart4,RXbufEsp,1);
	}
	if(huart == &huart2) {
		bufV2[posV2] = fakebufV2[0]; 
		posV2++;
		if ((posV2>0) && ((bufV2[posV2-1] == ';')|| (posV2 > 60)))
		{
			flagRXV2 = 1;		
			//sendUSB("receive data:");
			//sendUSB(bufV2);
			//sendUSB("\r\n");
		}
		HAL_UART_Receive_IT(&huart2,fakebufV2,1);
	}
	
	if(huart == &huart3) {
		bufUSB[posUSB] = fakebufUSB[0]; 
		posUSB++;
		if ((posUSB>0) && ((bufUSB[posUSB-1] == ';')|| (posUSB > 30)))
		{
			flagRXUSB = 1;
		}
		HAL_UART_Receive_IT(&huart3,fakebufUSB,1);
	}
	if(huart == &huart5) {
		bufV1[posV1] = fakebufV1[0]; 
		posV1++;
		if ((posV1>0) && ((bufV1[posV1-1] == ';')|| (posV1 > 30)))
		{
			flagRXV1 = 1;
			//sendUSB(bufUSB);
		}
		HAL_UART_Receive_IT(&huart5,fakebufV1,1);
	}
}

void parsePeople(){	
	if (RXdataPi[0] == '|') {	
			uint8_t i1 = 0;
			uint8_t j1 = 0;
			j1++; 
			uint8_t room11[10] = {0,};
			uint8_t room22[10] = {0,};
			uint8_t people11[10] = {0,};
			uint8_t people22[10] = {0,};
			while (RXdataPi[j1] != '_') {
					room11[i1] = RXdataPi[j1];
					i1++;
					j1++;		
			}
			i1 = 0;
			j1++;
			while (RXdataPi[j1] != '|') {
					people11[i1] = RXdataPi[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataPi[j1] != '_') {
					room22[i1] = RXdataPi[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataPi[j1] != ';') {
					people22[i1] = RXdataPi[j1];
					i1++;
					j1++;
			}
			room1.people = atoi(people11);
			room2.people = atoi(people22);
			sendUSB("first room people: ");
			sendUSB(people11);
			sendUSB("second room people: ");
			sendUSB(people22);
			}
	memset(RXdataPi,0,strlen(RXdataPi));
	posRXPi = 0;
}


void parseEsp(){
	if (RXdataEsp[0] == '*') {	
			uint8_t i1 = 0;
			uint8_t j1 = 0;
			j1++; 
			uint8_t temp1[10] = {0,};
			uint8_t hum1[10] = {0,};
			uint8_t co21[10] = {0,};
			uint8_t temp2[10] = {0,};
			uint8_t hum2[10] = {0,};
			uint8_t co22[10] = {0,};
			uint8_t vent1[10] = {0,};
			uint8_t vent2[10] = {0,};
			while (RXdataEsp[j1] != ' ') {
					temp1[i1] = RXdataEsp[j1];
					i1++;
					j1++;		
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {
					hum1[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {
					co21[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {
					temp2[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {
					hum2[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {
					co22[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ' ') {//проверка флагов проветрить 
					vent1[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			i1 = 0;
			j1++;
			while (RXdataEsp[j1] != ';') {
					vent2[i1] = RXdataEsp[j1];
					i1++;
					j1++;
			}
			script1.temp = atof(temp1);
			script1.hum = atof(hum1);
			script1.co2 = atoi(co21);
			script2.temp = atof(temp2);
			script2.hum = atof(hum2);
			script2.co2 = atoi(co22);

			v1 = atoi(vent1);
			v2 = atoi(vent2);
			uint8_t want[100] = {0,};	
			snprintf(want,100, "WANT = %.2f %.2f %d %.2f %.2f %d FLAGS_END: %d %d \r\n", script1.temp, script1.hum, script1.co2, script2.temp, script2.hum, script2.co2, v1, v2);
			sendUSB(want);				
			if ((v1 == 1 ) && (v2 == 1)){
				ventilateAllRooms();
			}
			else if (v1 == 1){
				ventilateRoom1();
			}
			else if (v2 == 1){
				ventilateRoom2();
			}
	}
	memset(RXdataEsp, 0, strlen(RXdataEsp));
	posRXEsp = 0;
}

void checkRoom1(){
	getSensorsRoom1();
	getADCRoom1();
	checkTempRoom1();
	checkHumRoom1();
	checkCO2Room1();
}

void checkTempRoom1(){
	if (room1.temp < (script1.temp + 1)) //если температура выше желаемой + 0.5градуса
	{
		//включить нагреватель
	}
	if (room1.temp > (script1.temp + 1))// если температура выше желаемой + 0,5 градуса
	{
		//выключить нагреватель
	}
}

void checkHumRoom1(){
	if (room1.hum < (script1.hum + 5)) //если температура выше желаемой + 0.5градуса
	{
		//включить увлажнитель
	}
	if (room1.temp > (script1.hum + 5))// если температура выше желаемой + 0,5 градуса
	{
		//выключить увлажнитель
	}
}

void checkCO2Room1(){
	if (room1.people > 0){
		peopleExchangeRoom1();
	}
	else {
		co2ExchangeRoom1();
	}
}

void peopleExchangeRoom1(){
	getADCRoom1();
	float requiredExchange = room1.people  * 23 * 0.65;
	if (requiredExchange > room1.exchange){
		if (room1.valveOpeningDegree < 3){
			room1.valveOpeningDegree++;
		}
		else	{
			if (ventSpeed < 3) ventSpeed++;
		}
		setVent();
		setValveRoom1();
		
	}
	else {
		co2ExchangeRoom1();
	}
}

void co2ExchangeRoom1(){
	if (room1.co2 < 400) {
		room1.valveOpeningDegree = 0;
		setValveRoom1();
		ventSpeed = 0;
		setVent();
	}//00
	if ((room1.co2 < 500) && (room1.co2 >= 400)){
		room1.valveOpeningDegree = 1;
		setValveRoom1();
		ventSpeed = 0;
		setVent();
	}//01
	if ((room1.co2 < 550) && (room1.co2 >= 500)){
		room1.valveOpeningDegree = 2;
		setValveRoom1();
		ventSpeed = 0;
		setVent();
	}//02
	if ((room1.co2 < 600) && (room1.co2 >= 550)){
		room1.valveOpeningDegree = 3;
		setValveRoom1();
		ventSpeed = 0;
		setVent();
	}//03
	if ((room1.co2 < 650) && (room1.co2 >= 600)){
		room1.valveOpeningDegree = 3;
		setValveRoom1();
		ventSpeed = 1;
		setVent();
	}//13
	if ((room1.co2 < 700) && (room1.co2 >= 650)){
		room1.valveOpeningDegree = 3;
		setValveRoom1();
		ventSpeed = 2;
		setVent();
	}//23
	if (room1.co2 >= 700){
		room1.valveOpeningDegree = 3;
		setValveRoom1();
		ventSpeed = 3;
		setVent();
	}//33
}

void checkRoom2(){
	getSensorsRoom2();
	getADCRoom2();
	checkTempRoom2();
	checkHumRoom2();
	checkCO2Room2();
}

void checkTempRoom2(){
	if (room2.temp < (script2.temp + 1)) //если температура выше желаемой + 0.5градуса
	{
		//включить нагреватель
	}
	if (room2.temp > (script2.temp + 1))// если температура выше желаемой + 0,5 градуса
	{
		//выключить нагреватель
	}
}

void checkHumRoom2(){
	if (room2.hum < (script2.hum + 5)) //если температура выше желаемой + 0.5градуса
	{
		//включить увлажнитель
	}
	if (room2.temp > (script2.hum + 5))// если температура выше желаемой + 0,5 градуса
	{
		//выключить увлажнитель
	}
}

void checkCO2Room2(){
	if (room2.people > 0){
		peopleExchangeRoom2();
	}
	else {
		co2ExchangeRoom2();
	}
}

void peopleExchangeRoom2(){
	getADCRoom2();
	float requiredExchange = room2.people  * 23 * 0.65;
	if (requiredExchange > room2.exchange){
		if (room2.valveOpeningDegree < 3){
			room2.valveOpeningDegree++;
		}
		else	{
			if (ventSpeed < 3) ventSpeed++;
		}
		setVent();
		setValveRoom2();
		
	}
	else {
		co2ExchangeRoom2();
	}
}

void co2ExchangeRoom2(){
	if (room2.co2 < 400) {
		room2.valveOpeningDegree = 0;
		setValveRoom2();
		ventSpeed = 0;
		setVent();
	}//00
	if ((room2.co2 < 500) && (room2.co2 >= 400)){
		room2.valveOpeningDegree = 1;
		setValveRoom2();
		ventSpeed = 0;
		setVent();
	}//01
	if ((room2.co2 < 550) && (room2.co2 >= 500)){
		room2.valveOpeningDegree = 2;
		setValveRoom2();
		ventSpeed = 0;
		setVent();
	}//02
	if ((room2.co2 < 600) && (room2.co2 >= 550)){
		room2.valveOpeningDegree = 3;
		setValveRoom2();
		ventSpeed = 0;
		setVent();
	}//03
	if ((room2.co2 < 650) && (room2.co2 >= 600)){
		room2.valveOpeningDegree = 3;
		setValveRoom2();
		ventSpeed = 1;
		setVent();
	}//13
	if ((room2.co2 < 700) && (room2.co2 >= 650)){
		room2.valveOpeningDegree = 3;
		setValveRoom2();
		ventSpeed = 2;
		setVent();
	}//23
	if (room2.co2 >= 700){
		room2.valveOpeningDegree = 3;
		setValveRoom2();
		ventSpeed = 3;
		setVent();
	}//33
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM1_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_UART5_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	/*@info UART
	UART4 - esp
  USART2 - BT 
  USART3 - USB
  USART6 - RaspberryPi
	USART5 - valve 1 9600

	@info TIMER
	TIM1 - 10 sec, send actual data to ESP
	TIM2 - 60 sec, polling RaspberryPi 
	TIM3 - 62 sec, check room param
	TIM4 - 30 vent delay
	TIM5 - 9 poll sensors
	*/
	
	//initSensors();
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
	//HAL_TIM_Base_Start_IT(&htim5);
	//HAL_TIM_Base_Start_IT(&htim3);
	HAL_UART_Receive_IT(&huart2,fakebufV2,1);
	HAL_UART_Receive_IT(&huart4,RXbufEsp,1);
	HAL_UART_Receive_IT(&huart6,RXbufPi,1);
	HAL_UART_Receive_IT(&huart3,fakebufUSB,1);
	HAL_UART_Receive_IT(&huart5,fakebufV1,1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
	HAL_DAC_Start(&hdac,DAC_CHANNEL_1);
	sendUSB("START\r\n");
	script1.ventilate = 1;
	script2.ventilate = 1;
	setVent();
	//ventSpeed = 3;
	//setVent();
	//sendV2("*2;\r\n");
	//flagEndVentilation = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if (flagRXPi == 1){
			parsePeople();
			flagRXPi = 0;
		}
		if (flagRXEsp == 1){
			parseEsp();
			flagRXEsp = 0;
		}
		if (flagRXV1 == 1){
			parseSensors1();
			flagRXV1 = 0;
		}
		if (flagRXV2 == 1){
			//parseSensors2();
			flagRXV2 = 0;
		}
		/*if (flagCheck == 1){
			flagCheck = 0;
			//checkRoom1();
			//checkRoom2();
		}*/
		if (flagRXUSB == 1){
			//uint8_t a[30] = {0,};
			//a[0] = bufUSB[1];
 			//room2.valveOpeningDegree = atoi(a);
			//snprintf(a, 15, "*%d;\r\n", room1.valveOpeningDegree);
			//sendV1(a);
			//sendUSB(a);
			//setValveRoom2();
			//sendUSB("1");
			//room2.valveOpeningDegree = atoi(a);
			//setValveRoom2();
			//memset(bufUSB,0,strlen(bufUSB));
			//posUSB = 0;
			flagRXUSB = 0;
		}
		if (flagEndVentilation == 1){
			if ((v1 == 1 ) && (v2 == 1)){
				//script1.ventilate = 0;
				v1 = 0;
				v2 = 0;
				script2.ventilate = 0;
				ventSpeed = 0;
				setVent();
				//room1.valveOpeningDegree = 0;
				//setValveRoom1();
				//room2.valveOpeningDegree = 0;
				//setValveRoom2();
			}
			else if (v1 == 1){
				v1 = 0;
				script1.ventilate = 0;
				ventSpeed = 0;
				setVent();
				//room1.valveOpeningDegree = 0;
				//setValveRoom1();
			}
			else if (v2 == 1){
				script2.ventilate = 0;
				v2 = 0;
				ventSpeed = 0;
				setVent();
				//room2.valveOpeningDegree = 0;
				//setValveRoom2();
			}
			flagEndVentilation = 0;
		}
		/*if (flagRXV1 == 1){
			parseV1();
			flagRXV1 = 0;
		}*/
		HAL_Delay(1000);
		//sendV2("*0;\r\n");
		//sendUSB("*0;\r\n");
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
