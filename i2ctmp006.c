 /*    ======== i2ctmp006.c ========
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Example/Board Header files */
#include "Board.h"

#define TASKSTACKSIZE       6400

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*
 *  ======== taskFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
Void taskFxn(UArg arg0, UArg arg1)
{
    unsigned int    i,a,b;
    //uint16_t        temperature;
    uint8_t         txBuffer[16];
    uint8_t         rxBuffer[2]; 
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;
    uint16_t        par_t1,par_t2,par_h1,par_h2;
    uint8_t         temp_lsb,temp_msb,temp_xlsb,par_t3,t1,t2,t2_msb,t2_lsb;
    uint8_t         hum_msb,hum_lsb,par_h11,par_h12,par_h21,par_h22,par_h3,par_h4,par_h5,par_h6,par_h7;
    int32_t        vart1,vart2,vart3,t_fine,temp_comp,temp_adc,temp_comp2,varh1,varh2,varh3,varh4,varh5,varh6,temp_scaled,hum_comp,press_adc,press_comp;//make this unsigned signed to get different value
    uint16_t        par_p1,par_p2,par_p4,par_p5,par_p8,par_p9;
    uint8_t         par_p3,par_p6,par_p7,par_p10,par_p11,par_p12,par_p21,par_p22,par_p41,par_p42,par_p51,par_p52,par_p81,par_p82,par_p91,par_p92;
    uint8_t         press_msb,press_lsb,press_xlsb;
    uint32_t        varp1,varp2,varp3;
    uint8_t         par_g1,par_g3,res_heat_range,par_g21,par_g22,gas_range;
    int8_t          res_heat_val;
    uint16_t        par_g2,gas_adc_lsb,gas_adc_msb,gas_adc; 
    uint32_t        gas_res,calc_gas_res;
    uint32_t         vars1,vars2,vars3,vars4,vars5;
    uint8_t         res_heat_x;
    int32_t         res_heat_x100;
    

    
   // float           final_temp; // floating value of temperature
    
    
    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;  // was initially 400khz
    i2c = I2C_open(Board_I2C_TMP, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    else {
        System_printf("I2C Initialized!\n");
    }
   
 
 

    /* resetting the sensor */
    txBuffer[0] = 0xE0; // address to reset Sensor
    txBuffer[1] = 0xB6; // value to write on reset register
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    if (I2C_transfer (i2c, &i2cTransaction))
    {
        System_printf("RESET COMPLETE \n");
        System_flush();
    }
    else 
    {
        System_printf("Resetting Failed. \n");
    }

    // checking the variant_id for BME688 its 0x01
    txBuffer[0] = 0xF0;  // variant_id address 
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    

        System_printf("Variant_ID: 0x%x\n",rxBuffer[0]);  // use %p for binary values or %x for hex
        System_flush();
    }
    else 
    {
        System_printf("Variant_ID Error !!!! \n");
    }
    // checking Chip ID 
    txBuffer[0] = 0xD0;   // chip ID address
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("Chip_ID: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        System_flush();
    }
    else 
    {
        System_printf("Chip_ID: ERROR!!! \n");
    }
    // setting the register to implement force mode
    /* set humidity samplling to 1X by writting 0b001 to bit<2:0> addressing 0x72 */
    
    txBuffer[0] = 0x72;   // control humidity register address
    txBuffer[1] = 0x01;   // value to set over sampling of humidity to 1
    txBuffer[2] = 0x74;   // oversampling register for pressure <4:2> and temperature <7:5>
    txBuffer[3] = 0x54;   // value to set over sampling of temp to 2X and pressure to 16x and mode oo mode bit remaining that's why or |
    txBuffer[4] = 0x75;   //  config register for IIR Filter bit<4:2>
    txBuffer[5] = 0x04;   // filter coefficient equals = 1
    txBuffer[6] = 0x71;   //  run gas enable bit<4> and heater step to be 1 <3:0> for heater step = 0 for gaswait0 and resheat0
    txBuffer[7] = 0x10;   // value to enable bit 4 as a run gas and nb_conv = 0
    txBuffer[8] = 0x64;   //  address to gas_wait_x 
    txBuffer[9] = 0x59;   // to select 100 ms heat up duration
    txBuffer[10] = 0x63;  //  set heater temperature res_heat_0<7:0>
    txBuffer[11] = 0xDF;  // value needs to be given for achieving heating resistance calculated value ( DF hex value)  decimal value 223
    txBuffer[12] = 0x71;  //  run gas enable bit<4> and heater step to be 1 <3:0>
    txBuffer[13] = 0x10;  // according to data sheet setting <3:0> = 0
   
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 14;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Ctrl_hum: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
       // System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
   

   for(i=1;i<26;i++)
 { 
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////implement loop for temp now////////////////////////////////////////////////////////////

    ////////////// setting the register to implement force mode /////////////////////////////////////////////////////
   
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    txBuffer[0] = 0x74;   //  oversampling register for pressure <4:2> and temperature <7:5> also mode register
    txBuffer[1] = 0x55;   // just changing mode to force mode changing from 54 to 55 just changes last two bits
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      /*  System_printf("Ctrl_MEAS:0x%x \n",rxBuffer[0]);  // use %p for decimal values or %x for hex

        System_flush(); */
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    } 
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////TEMPERATURE PARAMETER CALCULATION////////////////////////////////////////////

    txBuffer[0] = 0xEA;   //  Calibration parameter Par_t1 address MSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      //  System_printf("par_t1_MSB: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        t2 = rxBuffer[0];
        //System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0xE9;   //  Calibration parameter Par_t1 address LSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("par_t1_LSB: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        t1 = rxBuffer[0];
        //System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    
    par_t1 = t2 ;
    par_t1 = par_t1 << 8;
    par_t1 |= t1;
    
    txBuffer[0] = 0x8A;   //  Calibration parameter Par_t2 address LSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("t2_lsb: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        t2_lsb = rxBuffer[0];
       // System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x8B;   //  Calibration parameter Par_t2 address MSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("t2_msb: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        t2_msb = rxBuffer[0];
        //System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    /* Temperature calibration parameters*/
    par_t2 = t2_msb ;
    par_t2 = par_t2 << 8;
    par_t2 |= t2_lsb;

    
    txBuffer[0] = 0x8C;   //  Calibration parameter Par_t3 address MSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("par_t3: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        par_t3 = rxBuffer[0];
       // System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    /* temperature data collection part */
    
    txBuffer[0] = 0x23;   //  temp_lsb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("temp_lsb[0]: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        temp_lsb = rxBuffer[0];
        //System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x22;   //  temp_msb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("temp_msb[0]: 0x%x\n",rxBuffer[0]);  // use %p for decimal values or %x for hex
        temp_msb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
   
    txBuffer[0] = 0x24;   //  temp_xlsb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("temp_xlsb[0]: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        temp_xlsb = rxBuffer[0];
        //System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    uint32_t result;                      // result stores 20 bit value from  msb,xlsb,lsb 
    result = temp_msb;                    // temp_msb value 
    result = result << 12;                // bit shift left 12 bits to allow for lsb and xlsb values in the right most 12 bit position
    result |= (uint16_t)(temp_lsb << 4);             
    result |= (temp_xlsb >> 4);           // temp_xlsb appeneded in the last remaining 4 bits
   // System_printf("Result value : 0x%x \n",result);
    temp_adc = result;
   // System_printf("temp_adc value : 0x%x \n",temp_adc);
    

    // to create temperature adc
       // interger format  
    vart1 = ((int32_t) temp_adc >> 3) - ((int32_t)par_t1 << 1);
    vart2 = (vart1 * (int32_t)par_t2) >> 11;
    vart3 = ((((vart1 >> 1) * (vart1 >>1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14 ;
    t_fine = vart2 + vart3;
    temp_comp = ((t_fine * 5) + 128) >> 8;
    temp_comp2 = temp_comp % 100;
    //System_printf("Temperature Value : %d.%d C \n",temp_comp/100,temp_comp2); 

    //////////////////// Implementaining dealay ////////////////////////////////
    /*for (i = 1; i <= 1000;i++ )
    {
        for(i = 1; i <= 1000;i++)
        {}
    }*/
    ///// for loop ending just for temperature value 
    // implementing delay
   /* for (i = 1; a <= 10000;a++ )
    {
        for(i = 1; b <= 10000;b++)
        {}
    }
    */

    // float format
    /* var1 = (((double)temp_adc/ 16384.0) -((double)par_t1 / 1024.0)) * (double)par_t2;
     var2 = (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) * (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) * ((double)par_t3 * 16.0);
     t_fine = var1 + var2 ;
     temp_comp = t_fine / 5120.0; */
     //
     //
     /////////////// Humidity sensor Parameters////////
    txBuffer[0] = 0x25;   //  humidity msb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("hum_msb[0]: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        hum_msb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x26;   //  humidity lsb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("hum_lsb[0]: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        hum_lsb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0xE2;   //  humidity calibration parameter 11 <3:0> and 21 <7:4>
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h11: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h11 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0xE3;   //  humidity calibration parameter 12
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h12: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h12 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_h21 = (par_h11 >> 4);
    par_h11 = par_h11 << 4;  // left shift 4 bits so it has only 4 bit of data left
    par_h11 = par_h11 >> 4;  // bringing that 4 bit to original position with bit<7:4> = o  
    par_h1 = par_h12;
    par_h1 = par_h1 << 4;
    par_h1 |= par_h11;
   // System_printf("Par_h1:0x%x\n",par_h1);
    
    txBuffer[0] = 0xE1;   //  humidity calibration parameter 22
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Par_h22: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h22 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_h2 = par_h22;
    par_h2 = par_h2 << 4;
    par_h2 |= par_h21;
   // System_printf("Par_h2:0x%x\n",par_h2);

    txBuffer[0] = 0xE4;   //  humidity calibration parameter 3
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h3: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h3 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0xE5;   //  humidity calibration parameter 4
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h4: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h4 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0xE6;   //  humidity calibration parameter 5
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h5: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h5 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0xE7;   //  humidity calibration parameter 6
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Par_h6: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h6 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0xE8;   //  humidity calibration parameter 7
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_h7: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_h7 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    
 

    // humidity sensor adc 
    uint16_t hum_adc;
    hum_adc = hum_msb;
    hum_adc = hum_adc << 8;
    hum_adc |= hum_lsb;
   // System_printf("Humidity_ADC:0x%x \n",hum_adc);

    //data_sheet calculation for humidity sensor
       temp_scaled = (int32_t)temp_comp;
	   varh1 = (int32_t)hum_adc - (int32_t)((int32_t)par_h1 << 4) -((temp_scaled * (int32_t)par_h3) / ((int32_t)100) >> 1);
	   varh2 = ((int32_t)par_h2 *((temp_scaled *(int32_t)par_h4)/((int32_t)100)) + (((temp_scaled *((temp_scaled *(int32_t)par_h5) / ((int32_t)100))) >> 6) /((int32_t)(1 << 14)))) >> 10;
	   varh3 = varh1 * varh2;
	   varh4 = (((int32_t)par_h6 << 7) + ((temp_scaled * (int32_t)par_h7) /((int32_t)100))) >> 4;
	   varh5 = ((varh3 >> 14) * (varh3 >> 14)) >> 10;
	   varh6 = (varh4 * varh5) >> 1;
	   hum_comp = (varh3 +varh6) >> 12;
	   hum_comp = (((varh3 + varh6 ) >> 10) * ((int32_t)1000)) >> 12;
      // System_printf("Humidity_Comp:%d\n",hum_comp);
       
       //System_printf("Temperature Value : %d.%d C \t Humidity Value : %d.%d %%  \n",temp_comp/100,temp_comp2,hum_comp/100,hum_comp % 100);
      // System_printf("Humidity Value : %d.%d p.a \n",hum_comp/100,hum_comp % 100);
 
     ///////////// for loop end////////////////// for both humidity and temperature


      /////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////PRESSURE SENSOR//////////////////////////////////
    txBuffer[0] = 0x8F;   //  PRESSURE calibration parameter 1 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Par_p11: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p11 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }   
    
    txBuffer[0] = 0x8E;   //  PRESSURE calibration parameter 1 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p12: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p12 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }   
    par_p1 = par_p11;
    par_p1 = par_p1 << 8 ;
    par_p1 |= par_p12;
    //System_printf("Par_p1: 0x%x\n",par_p1);

    txBuffer[0] = 0x91;   //  PRESSURE calibration parameter 2 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Par_21: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p21 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x90;   //  PRESSURE calibration parameter 2 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_22: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p22 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_p2 = par_p21;
    par_p2 = par_p2 << 8 ;
    par_p2 |= par_p22;
    //System_printf("Par_p2: 0x%x\n",par_p2);
    
    txBuffer[0] = 0x92;   //  PRESSURE calibration parameter 3
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p3: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p3 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }        
    txBuffer[0] = 0x95;   //  PRESSURE calibration parameter 4 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p41: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p41 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x94;   //  PRESSURE calibration parameter 4 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p42: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p42 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_p4 = par_p41;
    par_p4 = par_p4 << 8 ;
    par_p4 |= par_p42;
    //System_printf("Par_p4: 0x%x\n",par_p4);

    txBuffer[0] = 0x97;   //  PRESSURE calibration parameter 5 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p51: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p51 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x96;   //  PRESSURE calibration parameter 5 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p52: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p52 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    par_p5 = par_p51;
    par_p5 = par_p5 << 8 ;
    par_p5 |= par_p52;
    //System_printf("Par_p5: 0x%x\n",par_p5);

    txBuffer[0] = 0x99;   //  PRESSURE calibration parameter 6
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p6: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p6 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x98;   //  PRESSURE calibration parameter 7
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      //  System_printf("Par_p7: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p7 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x9D;   //  PRESSURE calibration parameter 8 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p81: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p81 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x9C;   //  PRESSURE calibration parameter 8 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p82: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p82 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_p8 = par_p81;
    par_p8 = par_p8 << 8 ;
    par_p8 |= par_p82;
   // System_printf("Par_p8: 0x%x\n",par_p8);

    txBuffer[0] = 0x9F;   //  PRESSURE calibration parameter 9 msb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      //  System_printf("Par_p91: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p91 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    txBuffer[0] = 0x9E;   //  PRESSURE calibration parameter 9 lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      //  System_printf("Par_p92: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p92 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    par_p9 = par_p91;
    par_p9 = par_p9 << 8 ;
    par_p9 |= par_p92;
    //System_printf("Par_p9: 0x%x\n",par_p9);

    txBuffer[0] = 0xA0;   //  PRESSURE calibration parameter 10
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Par_p10: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_p10 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x1F;   //  PRESSURE adc msb[0]
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Press_msb: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        press_msb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x20;   //  PRESSURE adc lsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        //System_printf("Press_lsb: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        press_lsb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x21;   //  PRESSURE adc xlsb
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
      //  System_printf("Press_xlsb: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        press_xlsb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    //////////computing pressure adc////////////
    press_xlsb = (press_xlsb >> 4);
    press_adc = press_msb;
    press_adc = press_adc << 8;
    press_adc |= press_lsb;
    press_adc = press_adc << 4;
    press_adc |= press_xlsb;
    //System_printf("Pressure_ADC:0x%x \n",press_adc);
    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    varp1 = ((int32_t) t_fine >> 1) - 64000;
    varp2 = (((( varp1 >> 2 ) * ( varp1 >> 2 )) >> 11 ) * (int32_t) par_p6 ) >> 2;
    varp2 = varp2 + (( varp1 * (int32_t) par_p5 ) << 1 );
    varp2 = (varp2 >> 2) + ((int32_t) par_p4 << 16 );
    varp1 = (((((varp1 >> 2) * (varp1 >> 2)) >> 13) * ((int32_t) par_p3 << 5)) >> 3) + (((int32_t) par_p2 * varp1) >> 1);
    varp1 = varp1 >> 18;
    varp1 = ((32768 + varp1) * (int32_t) par_p1) >> 15;
    press_comp = 1048576 - press_adc ; 
    press_comp = (uint32_t)((press_comp - (varp2 >> 12)) * ((uint32_t)  3125 ));
    if (press_comp >= (1 << 30))
        press_comp = ((press_comp / (uint32_t) varp1 ) << 1);
    else 
        press_comp = ((press_comp << 1) / (uint32_t) varp1);
    varp1 =	((int32_t)par_p9 * (int32_t) (((press_comp >> 3) * (press_comp >> 3)) >> 13)) >> 12; 
    varp2 = ((int32_t) (press_comp >> 2 ) * (int32_t) par_p8) >> 13;
    varp3 = ((int32_t) (press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;
    press_comp = (int32_t) (press_comp) + ((varp1 + varp2 + varp3 + ((int32_t) par_p7 << 7)) >> 4);
    
    //System_printf("Pressure_comp:%d \n",press_comp);
    System_printf("Temperature: %d.%d C \t Humidity: %d.%d% %%\t Pressure: %d.%d hpa \n",temp_comp/100,temp_comp2,hum_comp/100,hum_comp % 100,press_comp/100,press_comp % 100);
  }  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////Loop here for Temperature pressure and humidity ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Gas Sensor////////////////////////////////////////////////////////////////////////////////////////////

    txBuffer[0] = 0xED;   //  res_heat_calculation parameter par_g1
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("Par_g1: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_g1 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0xEC;   //  res_heat_calculation parameter par_g1 MSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("par_g21: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_g21 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }


    txBuffer[0] = 0xEB;   //  res_heat_calculation parameter par_g1 lSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("par_g22: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_g22 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }


    par_g2 = par_g21;
    par_g2 = par_g2 << 8;
    par_g2 |= par_g22;
    System_printf("par_g2: 0x%x\n",par_g2);

    txBuffer[0] = 0xEE;   //  res_heat_calculation parameter par_g3
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("par_g3: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        par_g3 = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    txBuffer[0] = 0x02;   //  res_heat_range parameter <5:4>
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("res_heat_range: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        res_heat_range = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    res_heat_range = res_heat_range << 2;
    res_heat_range = res_heat_range >> 6;
    System_printf("res_heat_range<5:4>: 0x%x\n",res_heat_range);



    txBuffer[0] = 0x00;   //  res_heat_val
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("res_heat_val: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        res_heat_val = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

     /*  for (i = 1; a <= 100;a++)       // trying to implement delay to see if i could get any value on gas adc
      {
        for(i = 1; b <= 100;b++)
        {}
      }*/
    
    
    
    txBuffer[0] = 0x2D;   //  Gas_Adc_LSB <7:6>
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
    
        System_printf("Gas_Adc_Lsb: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        gas_adc_lsb = rxBuffer[0];
        System_flush();
        
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }
    
    


    txBuffer[0] = 0x2C;   //  GAS_ADC_MSB
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
        System_printf("Gas_Adc_MSB: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        gas_adc_msb = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }


    gas_adc = gas_adc_msb;
    gas_adc = gas_adc << 2;
    gas_adc_lsb = gas_adc_lsb >> 6;
    gas_adc |= gas_adc_lsb;
    System_printf("Gas_Adc: 0x%x\n",gas_adc); 
    
    txBuffer[0] = 0x2D;   //  Gas_range <3:0>
    i2cTransaction.slaveAddress = Board_BME688_ADDR; // BME SLAVE ADDRESS 0x76
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;
      if (I2C_transfer (i2c, &i2cTransaction))
    {    
        
       // System_printf("Gas_Range: 0x%x\n",rxBuffer[0]);  // use %p for binary  values or %x for hex
        gas_range = rxBuffer[0];
        System_flush();
    }
    else 
    {
        System_printf("Register read: ERROR!!! \n");
    }

    gas_range = gas_range << 4;
    gas_range = gas_range >> 4;
    System_printf("Gas_range<3:0>: 0x%x\n",gas_range);






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////GAS RESISTANCE READOUT////////////////////////////////////////////////////////////
     
    uint32_t  varg11 = UINT32_C(262144) >> gas_range; //
    int32_t varg22 = (int32_t) gas_adc - INT32_C(512);
    varg22 *= INT32_C(3);
    varg22 = INT32_C(4096) + varg22;
    calc_gas_res = (UINT32_C(10000) * varg11) / (uint32_t)varg22;
    gas_res = calc_gas_res * 100; 
    System_printf("Gas_res: %d\n",gas_res);
    System_flush();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Gas Sensor Heating and Measure//////////////////////////////////////////////////////////////

// GAS sensor heating and measurement 


vars1 = (((int32_t) (temp_comp/100) * par_g3) / 10) << 8; //temp_comp instead of 23 its a room temperature
vars2 = (par_g1 + 784) * (((((par_g2 +154009) * 300 * 5) / 100) + 3276800) / 10); // target_temp instead of 300
vars3 = vars1 + (vars2 >> 1);
vars4 = (vars3 / (res_heat_range + 4));
vars5 = (131 * res_heat_val) + 65536;
res_heat_x100 = (int32_t)(((vars4 /vars5) - 250) * 34);
res_heat_x = (uint8_t) ((res_heat_x100 + 50) / 100);
System_printf("Res_Heat_X: %d\n",res_heat_x); 
//System_printf("VARS5: %d\n",vars5);
System_flush();

// 




    /* Deinitialized I2C */
    I2C_close(i2c);
    System_printf("I2C closed!\n");

    System_flush();
} 


/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();

    /* Construct tmp006 Task thread */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the I2C example\nSystem provider is set to SysMin."
                  " Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
   while (1)
   {
    System_flush();

    /* Start BIOS */
    BIOS_start();


   } 
   // return (0);
}
