/** \addtogroup cc2538-examples
 * @{
 *
 * \file
 *  /dev/sht21
 *  An example of a simple I2C use for the cc2538 and SHT21 sensor
 * \author
 *  Mehdi Migault
 */
#include <contiki.h>
#include "dev/i2c.h"
#include "sht21.h"
#include <stdio.h>

// #define DBG(...) printf(__VA_ARGS__)
#define DBG(...)

static uint16_t temp;
/*---------------------------------------------------------------------------*/
void
print_float(float num, uint8_t preci, char *buf)
{
	int integer=(int)num, decimal=0;
	preci = preci > 10 ? 10 : preci;
	num -= integer;
  DBG("num value: %d, preci value: %d\r\n", num, preci);
	while((num != 0) && (preci-- > 0)) {
		decimal *= 10;
		num *= 10;
		decimal += (int)num;
		num -= (int)num;
	}
	sprintf(buf, "%d.%d", integer, decimal);
  DBG("print float int dec value: %d.%d\r\n", integer, decimal);
  DBG("print float buf value: %s\r\n", buf);
}

uint8_t
check_crc_SHT21(uint8_t data[], uint8_t dataSize, uint8_t chksm)
{
	uint8_t crc=0, i, j;
	for(i=0; i<dataSize; ++i) {
		crc ^= data[i];
		for(j=8; j>0; --j) {
			if(crc & 0x80) {
				crc = (crc<<1) ^ SHT21_CRC_POLYNOMIAL;
			} else {
				crc = (crc<<1);
			}
		}
	}
	if(crc != chksm) {
		return -1;
	} else {
		return I2C_MASTER_ERR_NONE;
	}
}
/*---------------------------------------------------------------------------*/
uint8_t
read_SHT21(uint16_t * data, uint8_t regist)
{
  uint8_t dataByte[2];
  if(regist != SHT21_TEMP_REGISTER && regist != SHT21_HUMI_REGISTER) {
    return -1;
  }

  i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_SEND);
  i2c_master_data_put(regist);
  i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
  DBG("I2C_MASTER_CMD_BURST_SEND_START\r\n");  
  while(i2c_master_busy()){
  }  
  if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
    if(regist == SHT21_TEMP_REGISTER) {
      for(temp=0; temp<10; temp++) {
        clock_delay_usec(8500); //85ms
      }
    } else if(regist == SHT21_HUMI_REGISTER) {
      for(temp=0; temp<10; temp++) {
        clock_delay_usec(2900); //29ms
      }
    }
    /* Get the 2 bytes of data*/
    /* Data MSB */   
    i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_RECEIVE);
    i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
    DBG("I2C_MASTER_CMD_BURST_RECEIVE_START\r\n");
    while(i2c_master_busy()) {
    }    
    if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
      *data = i2c_master_data_get() << 8;
      
      /* Data LSB */
      i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
      DBG("I2C_MASTER_CMD_BURST_RECEIVE_CONT\r\n");
      while(i2c_master_busy()) {
      }
      if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
        *data |= i2c_master_data_get();
        
        /* Checksum */
        i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        DBG("I2C_MASTER_CMD_BURST_RECEIVE_FINISH\r\n");
        while(i2c_master_busy()) {
        }
        if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
          dataByte[0] = (*data)>>8;
          dataByte[1] = (*data)&0x00FF;
          DBG("CRC CHECK\r\n");
          if(check_crc_SHT21(dataByte, 2, i2c_master_data_get()) == I2C_MASTER_ERR_NONE){
            DBG("CRC CHECK SUCCESS\r\n");
            return I2C_MASTER_ERR_NONE;
          }
          else {
            DBG("CRC CHECK FAILED\r\n");
          }
        }
      }
    }
  }
  return i2c_master_error();
}
/*---------------------------------------------------------------------------*/
void
read_temperature(char *temp_val) {
    char ctemp[20];
    if(read_SHT21(&temp, SHT21_TEMP_REGISTER) == I2C_MASTER_ERR_NONE) {
        DBG("temp value: %d\r\n", temp);
        temp &= ~0x0003;
        print_float(((((float)temp)/65536)*175.72-46.85), SHT21_DECIMAL_PRECISION, ctemp);
        DBG("ctemp value: %s\r\n", ctemp);
        strcpy(temp_val, ctemp);
    }
    else {
        DBG("Error reading temperature sensor\r\n");
    }
}

void 
read_humidity(char *hum_val) {
    char chum[20];
    if(read_SHT21(&temp, SHT21_HUMI_REGISTER) == I2C_MASTER_ERR_NONE) {
        DBG("temp value: %d\r\n", temp);
        temp &= ~0x000F;
        print_float(((((float)temp)/65536)*125-6), SHT21_DECIMAL_PRECISION, chum);
        DBG("chum value: %s\r\n", chum);        
        strcpy(hum_val, chum);
    }
    else {
        DBG("Error reading humidity sensor\r\n");
    }
}
