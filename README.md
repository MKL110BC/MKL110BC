
## Purpose
Created this project for easier development on MKL110BC.   
The project base on  nrf5_SDK 17.0 


## lorawan SDK
This project base on LoRa Basics Modem LoRaWAN stack.  
github url:https://github.com/Lora-net/SWL2001.     
And the sdk example github url :https://github.com/Lora-net/SWSD001  


## MAIN DIR
M_Prj   									: MDK project dir   
src												: main source code dir  
src/lora_basic_modem_sdk	: include LoRa Basics Modem LoRaWAN stack , SDK Port file and example app  


## note
in MDK compile option ,need add this define    
define:REGION_EU_868 REGION_AS_923  REGION_US_915 REGION_AU_915 REGION_CN_470 REGION_IN_865 REGION_KR_920 REGION_RU_864 REGION_CN_470_RP_1_0 REGION_WW2G4 LR1110 LR1110_TRANSCEIVER ENABLE_MODEM_GNSS_FEATURE