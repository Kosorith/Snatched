/******************************************************************************
 *
 * File:
 *    eeprom.c
 * 
 * Description:
 *    The library is responsible for communication with PCA9532 module and 
 *    retrieving temperature from the sensor.
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "../pre_emptive_os/api/general.h"
#include "i2c.h"

/***********/
/* Defines */
/***********/

#define EEPROM_SIZE       0x2000  // 64kbit = 8KByte
#define LOCAL_EEPROM_ADDR 0x0
#define EEPROM_ADDR       0xA0

#define I2C_EEPROM_ADDR   (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1))
#define I2C_EEPROM_RCV    (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1) + 0x01)
#define I2C_EEPROM_SND    (EEPROM_ADDR + (LOCAL_EEPROM_ADDR << 1) + 0x00)

/*************/
/* Functions */
/*************/

/******************************************************************************
 *
 * Description:
 *    Start a read
 *
 * Params:
 *    [in] devAddr - device address
 *    [in] address - eeprom address
 *
 * Returns:
 *    I2C_CODE_OK or I2C_CODE_ERROR
 *
 *****************************************************************************/
tS8 eepromStartRead(tU8 devAddr, tU16 address) {
    tS8 retCode = 0;
    tU8 status = 0;

    /* Generate Start condition */
    retCode = i2cStart();

    /* Transmit address */
    if (I2C_CODE_OK == retCode) {
        /* Write SLA+W */
        retCode = i2cPutChar(devAddr);
        while (I2C_CODE_BUSY == retCode) {
            retCode = i2cPutChar(devAddr);
        }
    }

    if (I2C_CODE_OK == retCode) {

        /* Wait until data transmitted */
        while (1) {
            /* Get new status */
            status = i2cCheckStatus();

            if ((0x18 == status) || (0x28 == status)) {
                /* Data transmitted and ACK received */


                /* Write data */
                retCode = i2cPutChar((tU8) (address & 0xff));
                while (I2C_CODE_BUSY == retCode) {
                    retCode = i2cPutChar((tU8) (address & 0xff));
                }

                break;
            } else if (0xf8 != status) {
                /*  error */
                retCode = I2C_CODE_ERROR;
                break;
            }
        }
    }

    /* Wait until data transmitted */
    while (1) {
        /* Get new status */
        status = i2cCheckStatus();

        if (0x28 == status) {
            /* Data transmitted and ACK received */
            break;
        } else if (0xf8 != status) {
            /* error */
            retCode = I2C_CODE_ERROR;
            break;
        }
    }

    /* Generate Start condition */
    retCode = i2cRepeatStart();

    /* Transmit device address */
    if (I2C_CODE_OK == retCode) {
        /* Write SLA+R */
        retCode = i2cPutChar(devAddr + 0x01);
        while (I2C_CODE_BUSY == retCode) {
            retCode = i2cPutChar(devAddr + 0x01);
        }
    }

    /* Wait until SLA+R transmitted */
    while (1) {
        /* Get new status */
        status = i2cCheckStatus();

        if (0x40 == status) {
            /* Data transmitted and ACK received */
            break;
        } else if (0xf8 != status) {
            /* error */
            retCode = I2C_CODE_ERROR;
            break;
        }
    }

    // Do not generate a stop bit

    return retCode;
}

/******************************************************************************
 *
 * Description:
 *    Read from LM75
 *
 * Params:
 *    [in] address - device address
 *    [in] pBuf - buffer for received data
 *    [in] len - buffer length
 *
 * Returns:
 *    I2C status code
 *
 *****************************************************************************/
tS8 lm75Read(tU8 address, tU8* pBuf, tU16 len) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU16 i = 0;

    retCode = eepromStartRead(address, 0);


    if (I2C_CODE_OK == retCode) {
        /* wait until address transmitted and receive data */
        for (i = 1; i <= len; i++) {
            /* wait until data transmitted */
            while (1) {
                /* Get new status */
                status = i2cCheckStatus();

                if ((0x40 == status) || (0x48 == status) || (0x50 == status)) {
                    /* Data received */

                    if (i == len) {
                        /* Set generate NACK */
                        retCode = i2cGetChar(I2C_MODE_ACK1, pBuf);
                    } else {
                        retCode = i2cGetChar(I2C_MODE_ACK0, pBuf);
                    }

                    /* Read data */
                    retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    while (I2C_CODE_EMPTY == retCode) {
                        retCode = i2cGetChar(I2C_MODE_READ, pBuf);
                    }
                    pBuf++;

                    break;
                } else if (0xf8 != status) {
                    /* ERROR */
                    i = len;
                    retCode = I2C_CODE_ERROR;
                    break;
                }
            }
        }
    }

    /* Generate Stop condition */
    i2cStop();

    return retCode;
}

 /******************************************************************************
 *
 * Description:
 *    Communicate with PCA9532
 *
 * Params:
 *    [in] pBuf - buffer of bytes to write
 *    [in] len - length of pBuf
 *    [in] pBuf2 - buffer for received data
 *    [in] len - length of pBuf2
 *
 * Returns:
 *    I2C status code
 *
 *****************************************************************************/
tS8 pca9532(tU8* pBuf, tU16 len, tU8* pBuf2, tU16 len2) {
    tS8 retCode = 0;
    tU8 status = 0;
    tU16 i = 0;

    do {
        /* generate Start condition */
        retCode = i2cStart();
        if (I2C_CODE_OK != retCode) {
            break;
        }

        /* write pca9532 address */
        retCode = i2cWriteWithWait(0xc0);
        if (I2C_CODE_OK != retCode) {
            break;
        }

        /* write data */
        for (i = 0; i < len; i++) {
            retCode = i2cWriteWithWait(*pBuf);
            if (I2C_CODE_OK != retCode) {
                break;
            }

            pBuf++;
        }

    } while (0);

    if (len2 > 0) {
        /* Generate Start condition */
        retCode = i2cRepeatStart();

        /* Transmit device address */
        if (I2C_CODE_OK == retCode) {
            /* Write SLA+R */
            retCode = i2cPutChar(0xc0 + 0x01);
            while (I2C_CODE_BUSY == retCode) {
                retCode = i2cPutChar(0xc0 + 0x01);
            }
        }

        /* Wait until SLA+R transmitted */
        while (1) {
            /* Get new status */
            status = i2cCheckStatus();

            if (0x40 == status) {
                /* Data transmitted and ACK received */
                break;
            } else if (0xf8 != status) {
                /* error */
                retCode = I2C_CODE_ERROR;
                break;
            }
        }

        if (I2C_CODE_OK == retCode) {
            /* wait until address transmitted and receive data */
            for (i = 1; i <= len2; i++) {
                /* wait until data transmitted */
                while (1) {
                    /* Get new status */
                    status = i2cCheckStatus();

                    if ((0x40 == status) || (0x48 == status) || (0x50 == status)) {
                        /* Data received */

                        if (i == len2) {
                            /* Set generate NACK */
                            retCode = i2cGetChar(I2C_MODE_ACK1, pBuf2);
                        } else {
                            retCode = i2cGetChar(I2C_MODE_ACK0, pBuf2);
                        }

                        /* Read data */
                        retCode = i2cGetChar(I2C_MODE_READ, pBuf2);
                        while (I2C_CODE_EMPTY == retCode) {
                            retCode = i2cGetChar(I2C_MODE_READ, pBuf2);
                        }
                        pBuf2++;

                        break;
                    } else if (0xf8 != status) {
                        /* ERROR */
                        i = len2;
                        retCode = I2C_CODE_ERROR;
                        break;
                    }
                }
            }
        }
    }

    /* Generate Stop condition */
    i2cStop();

    return retCode;
}
