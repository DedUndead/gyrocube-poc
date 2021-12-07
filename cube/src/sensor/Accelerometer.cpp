#include <stdio.h>
#include "sensor/Accelerometer.h"
#include "interface/i2c.h"

Accelerometer::Accelerometer(I2C* i2c_, uint8_t address_) :
    fast_mode(false),
    address(address_),
    i2c(i2c_) 
{ /* ... */ }

/**
 * @brief Read sensor data
 * @param acc_measurement Buffer to read the data to
 * @return 0 on success, otherwise error status
 */
int Accelerometer::read(acc_measurements& buffer)
{
    if (fetch_data() < XYZ_BUFFER) {
        return ACC_ERROR_STATUS;
    }
    
    // Save data to buffer
    if (fast_mode) {
        printf("Raw reading: %d %d %d", buffer_short[0], buffer_short[1], buffer_short[2]);
        buffer.x = static_cast<int8_t>(buffer_short[0]);
        buffer.y = static_cast<int8_t>(buffer_short[1]);
        buffer.z = static_cast<int8_t>(buffer_short[2]);
    }
    else {
        uint16_t x_raw = (buffer_full[0] << 6) | (buffer_full[1] >> 2);
        buffer.x = static_cast<int16_t>(x_raw);

        uint16_t y_raw = (buffer_full[2] << 6) | (buffer_full[3] >> 2);
        buffer.y = static_cast<int16_t>(y_raw);

        uint16_t z_raw = (buffer_full[4] << 6) | (buffer_full[5] >> 2);
        buffer.z = static_cast<int16_t>(z_raw);

        printf("Raw reading: %d %d %d\n", x_raw, y_raw, z_raw);
    }

    return 0;
}

/**
 * @brief Enable accelerometer in fast mode
 * Reading will be decreased to 8 bits
 * @return 0 on success, error status on error
 */
int Accelerometer::enable_in_fast_mode()
{
    // Disable accelerometer to change configuration
    disable();
    
    // Write new configuration
    uint8_t config = 0b00001011;
    if (i2c->write(address, CTRL_REG1, &config, 1) == PICO_ERROR_GENERIC) {
        return ACC_ERROR_STATUS;
    }
    fast_mode = true;

    sleep_ms(10);

    return 0;
}

/**
 * @brief Enable accelerometer in normal mode
 * @return 0 on success, error status on error
 */
int Accelerometer::enable()
{
    uint8_t config = 0b00001001;
    if (i2c->write(address, CTRL_REG1, &config, 1) == PICO_ERROR_GENERIC) {
        return ACC_ERROR_STATUS;
    }
    fast_mode = false;
    
    sleep_ms(10);

    return 0;
}

/**
 * @brief Disable accelerometer
 * @return 0 on success, error status on error
 */
int Accelerometer::disable()
{
    // Set control register to read values in Xmsb, Ymsb, Zmsb order
    uint8_t config = 0b00001000;
    if (i2c->write(address, CTRL_REG1, &config, 1) == PICO_ERROR_GENERIC) {
        return ACC_ERROR_STATUS;
    }

    sleep_ms(10);
    
    return 0;
}

/**
 * @brief Fetch raw data from the device
 * @return Number of bytes read
 */
int Accelerometer::fetch_data()
{
    if (fast_mode) {
        printf("Fast mode reading.\n");
        return i2c->read(address, OUT_X_MSB, buffer_short, XYZ_BUFFER);
    }
    else {
        printf("Slow mode reading.\n");
        return i2c->read(address, OUT_X_MSB, buffer_full, XYZ_BUFFER_FULL);
    }
}