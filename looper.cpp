#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>
#include <iostream>
#include <vector>
#include <utility>
#include <mutex>

#include "buffer_toggle.h"

#define DURATION_US 1

class Encoder {

private:
    // std::vector<std::pair<int, int>> noisy_seq = {{1, 0}, {1, 1}, {0, 1}, {1, 1}, {0, 1}, {1, 1}, {0, 1}, {0, 0}};
    std::vector<std::pair<int, int>> seq = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
    
public:

    Encoder(int pin_a, int pin_b) {
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

    ~Encoder() {
        // Cleanup GPIO
        gpiod_chip_close(chip);
    }

    void generate_direct_signal() {      
        std::lock_guard<std::mutex> lock(m);      
        auto it = seq[seq_step % seq.size()];        
        gpiod_line_set_value(line_a, it.first);
        gpiod_line_set_value(line_b, it.second);            
        std::this_thread::sleep_for(std::chrono::microseconds(DURATION_US));
        seq_step++;
    }

    void generate_reverse_signal() {      
        std::lock_guard<std::mutex> lock(m);
        auto it = seq[seq_step % seq.size()];        
        gpiod_line_set_value(line_a, it.first);
        gpiod_line_set_value(line_b, it.second);            
        std::this_thread::sleep_for(std::chrono::microseconds(DURATION_US));
        seq_step--;
    }

    struct gpiod_chip *chip;
    struct gpiod_line *line_a;
    struct gpiod_line *line_b;
    int seq_step;
    std::mutex m;
};

class motor_driver {
public:
        motor_driver(int pin_dir, int pin_step, Encoder& enc) : encoder(enc) {
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
    int count_half_steps = 0;
    bool old_step;
    Encoder& encoder;
};


int main() {
    // BufferToggle bt;
    // bt.off();                           // Do not wait for "enter" key

    // std::vector<std::thread> workers;

    Encoder enc_x(21, 26);
    Encoder enc_y(20, 19);
    Encoder enc_z(16, 13);

    motor_driver mot_x(17, 27, enc_x);
    motor_driver mot_y(22, 10, enc_y);    
    motor_driver mot_z(9, 11, enc_z);

    // while (true) {
    //     enc_x.generate_direct_signal();
    // }

    motor_driver* rema_motors[] = {&mot_x, &mot_y, &mot_z};
    
    while (true) {
        for (auto motor: rema_motors) {
            static int x_old = 0, y_old = 0, z_old = 0;
            int  signals = motor->read();
            if (signals & 1 != motor->old_step) {        // step signal changed?
                motor->count_half_steps++;
                if (! (motor->count_half_steps % 2)) {
                    (signals & 2) ? motor->encoder.generate_reverse_signal() : motor->encoder.generate_direct_signal();
                }
            }
            motor->old_step = signals & 1;
        }
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
    
    // for(auto &w: workers) {
    //     w.join();
    // }

    // bt.on();   
}
