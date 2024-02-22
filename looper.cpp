#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>
#include <iostream>
#include <vector>
#include <utility>
#include <mutex>

#include "buffer_toggle.h"

#define DURATION_US 100 // Adjust the duration based on your requirements

class motor_driver {
public:
        motor_driver(int pin_dir, int pin_step) {
        chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip) {
            perror("Error opening GPIO chip");
            exit(EXIT_FAILURE);
        }

        line_dir = gpiod_chip_get_line(chip, pin_dir);
        line_step = gpiod_chip_get_line(chip, pin_step);

        if (!line_dir || !line_step) {
            perror("Error getting GPIO lines");
            exit(EXIT_FAILURE);
        }

        gpiod_line_request_input(line_dir, "motor_driver");
        gpiod_line_request_input(line_step, "motor_driver");
    }

    ~motor_driver() {
        // Cleanup GPIO
        gpiod_chip_close(chip);
    }

    int32_t read() {      
        bool dir = gpiod_line_get_value(line_dir);
        bool step = gpiod_line_get_value(line_step);
        //std::cout << "dir " << dir << " step " << step << "\n";
        return dir << 1 | step;
    }


    struct gpiod_chip *chip;
    struct gpiod_line *line_dir;
    struct gpiod_line *line_step;

};

class encoder {

private:
    // std::vector<std::pair<int, int>> noisy_seq = {{1, 0}, {1, 1}, {0, 1}, {1, 1}, {0, 1}, {1, 1}, {0, 1}, {0, 0}};
    std::vector<std::pair<int, int>> seq = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
    
public:

    encoder(int pin_a, int pin_b) {
        chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip) {
            perror("Error opening GPIO chip");
            exit(EXIT_FAILURE);
        }

        line_a = gpiod_chip_get_line(chip, pin_a);
        line_b = gpiod_chip_get_line(chip, pin_b);

        if (!line_a || !line_b) {
            perror("Error getting GPIO lines");
            exit(EXIT_FAILURE);
        }

        gpiod_line_request_output(line_a, "quadrature_signal_generator", GPIOD_LINE_ACTIVE_STATE_HIGH);
        gpiod_line_request_output(line_b, "quadrature_signal_generator", GPIOD_LINE_ACTIVE_STATE_HIGH);
    }

    ~encoder() {
        // Cleanup GPIO
        gpiod_chip_close(chip);
    }

    void generate_direct_signal() {      
        std::lock_guard<std::mutex> lock(m);
        for (int times = 0 ; times < 1; times ++) {               
            for (auto i = seq.begin(); i != seq.end(); ++i ) { 
                gpiod_line_set_value(line_a, i->first);
                gpiod_line_set_value(line_b, i->second);            
                std::this_thread::sleep_for(std::chrono::microseconds(DURATION_US));
            } 
        }        
    }

    void generate_reverse_signal() {      
        std::lock_guard<std::mutex> lock(m);
        for (int times = 0 ; times < 1; times ++) {
            for (auto i = seq.rbegin(); i != seq.rend(); ++i ) { 
                gpiod_line_set_value(line_a, i->first);
                gpiod_line_set_value(line_b, i->second);            
                std::this_thread::sleep_for(std::chrono::microseconds(DURATION_US));
            }
        }         
    }

    struct gpiod_chip *chip;
    struct gpiod_line *line_a;
    struct gpiod_line *line_b;
    std::mutex m;
};


int main() {
    BufferToggle bt;
    bt.off();                           // Do not wait for "enter" key

    std::vector<std::thread> workers;

    encoder enc_x(21, 26);
    encoder enc_y(20, 19);
    encoder enc_z(16, 13);

    motor_driver mot_x(17, 27);
    motor_driver mot_y(22, 10);
    motor_driver mot_z(9, 11);

    while (true) {
        static int x_old = 0, y_old = 0, z_old = 0;
        static int count_half_steps_x = 0, count_half_steps_y = 0, count_half_steps_z = 0;
        int x = mot_x.read();
        if (x & 1 != x_old) {            
            count_half_steps_x ++;
            if (! (count_half_steps_x % 2)) {
                (x & 2) ? enc_x.generate_reverse_signal() : enc_x.generate_direct_signal();
            }
        }
        x_old = x & 1;
        
        int y = mot_y.read();
        if (y & 1 != y_old) {
            count_half_steps_y ++;
            if (! (count_half_steps_y % 2)) {
                (y & 2) ? enc_y.generate_direct_signal() : enc_y.generate_reverse_signal();
            }
        }
        y_old = y & 1;

        int z = mot_z.read();
        if (z & 1 != z_old) {
            count_half_steps_z ++;
            if (! (count_half_steps_x % 2)) {
                (z & 2) ? enc_z.generate_direct_signal() : enc_z.generate_reverse_signal();
            }
        }
        z_old = z & 1;
        // std::cout << "X dir: " << (x & (1 << 1)) << "step: " << (x & (1 << 0));
        // std::cout << "Y dir: " << (y & (1 << 1)) << "step: " << (y & (1 << 0));
        // std::cout << "Z dir: " << (z & (1 << 1)) << "step: " << (z & (1 << 0)) << "\n";
       
        //char c = std::getchar();
        // std::cout << "Y:";
        // mot_y.read();
        // std::cout << "Z:";
        // mot_z.read();
    }
    
    // while (char c = std::getchar()) {
    //     switch (c) {
    //     case 'd':
    //         workers.push_back(std::thread(&encoder::generate_reverse_signal, &enc_x));
    //         break;

    //     case 'a':
    //         workers.push_back(std::thread(&encoder::generate_direct_signal, &enc_x));
    //         break;

    //     case 'w':
    //         workers.push_back(std::thread(&encoder::generate_reverse_signal, &enc_y));
    //         break;

    //     case 's':
    //         workers.push_back(std::thread(&encoder::generate_direct_signal, &enc_y));
    //         break;

    //     case '8':
    //         workers.push_back(std::thread(&encoder::generate_reverse_signal, &enc_z));
    //         break;

    //     case '2':
    //         workers.push_back(std::thread(&encoder::generate_direct_signal, &enc_z));
    //         break;

    //     default:
    //         break;
    //     }
    // }
    
    for(auto &w: workers) {
        w.join();
    }

    bt.on();   
}
