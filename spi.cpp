#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

// Function to transfer data over SPI
void transferData(int spi_fd, uint8_t* tx_buffer, uint8_t* rx_buffer, int length) {
    spi_ioc_transfer transfer{};
    transfer.tx_buf = reinterpret_cast<unsigned long long>(tx_buffer);
    transfer.rx_buf = reinterpret_cast<unsigned long long>(rx_buffer);
    transfer.len = length;

    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer);
}

int main() {
    const char* device = "/dev/spidev0.0"; // SPI device path
    int spi_fd = open(device, O_RDWR);

    if (spi_fd < 0) {
        std::cerr << "Error opening SPI device" << std::endl;
        return EXIT_FAILURE;
    }

    // Set SPI mode and bits per word
    uint8_t mode = SPI_MODE_3;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) == -1) {
        std::cerr << "Error setting SPI mode" << std::endl;
        return EXIT_FAILURE;
    }

    uint8_t bits = 8;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
        std::cerr << "Error setting SPI bits per word" << std::endl;
        return EXIT_FAILURE;
    }

    // Set SPI speed (Hz)
    uint32_t speed = 1000000; // 1 MHz
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        std::cerr << "Error setting SPI speed" << std::endl;
        return EXIT_FAILURE;
    }

    // Example: Reading data from an SPI device
    const int buffer_size = 5;
    uint8_t tx_buffer[buffer_size] = {0x61}; // Command bytes
    uint8_t rx_buffer[buffer_size] = {0}; // Buffer to store received data

    transferData(spi_fd, tx_buffer, rx_buffer, 1);

    tx_buffer[0] = {0};
    transferData(spi_fd, tx_buffer, rx_buffer, buffer_size);

    // Print received data
    std::cout << "Received Data: ";
    for (int i = 0; i < buffer_size; i++) {
        std::cout << static_cast<int>(rx_buffer[i]) << " ";
    }
    std::cout << std::endl;


    tx_buffer[0] = {0x62}; // Command bytes
    
    transferData(spi_fd, tx_buffer, rx_buffer, 1);

    tx_buffer[0] = {0};
    transferData(spi_fd, tx_buffer, rx_buffer, buffer_size);

    // Print received data
    std::cout << "Received Data: ";
    for (int i = 0; i < buffer_size; i++) {
        std::cout << static_cast<int>(rx_buffer[i]) << " ";
    }
    std::cout << std::endl;

    tx_buffer[0] = {0x63}; // Command bytes
    
    transferData(spi_fd, tx_buffer, rx_buffer, 1);

    tx_buffer[0] = {0};
    transferData(spi_fd, tx_buffer, rx_buffer, buffer_size);

    // Print received data
    std::cout << "Received Data: ";
    for (int i = 0; i < buffer_size; i++) {
        std::cout << static_cast<int>(rx_buffer[i]) << " ";
    }
    std::cout << std::endl;

    // Close SPI device
    close(spi_fd);

    return EXIT_SUCCESS;
}
