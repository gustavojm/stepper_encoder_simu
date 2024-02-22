#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <wiringPi.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Define the GPIO pins
#define X_PIN_A 21  // Change this to the actual GPIO pin you are using
#define X_PIN_B 26  // Change this to the actual GPIO pin you are using

// Set initial states
void initialize() {
    pinMode(X_PIN_A, OUTPUT);
    pinMode(X_PIN_B, OUTPUT);
    digitalWrite(X_PIN_A, LOW);
    digitalWrite(X_PIN_B, LOW);
}

// Generate quadrature signal
void generateQuadratureSignal(int durationMillis) {
    while (durationMillis--) {
        // Quadrature signal: A rising, B rising
        digitalWrite(X_PIN_A, HIGH);
        digitalWrite(X_PIN_B, LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMillis));

        // Quadrature signal: A falling, B rising
        digitalWrite(X_PIN_A, LOW);
        digitalWrite(X_PIN_B, HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMillis));

        // Quadrature signal: A falling, B falling
        digitalWrite(X_PIN_A, LOW);
        digitalWrite(X_PIN_B, LOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMillis));

        // Quadrature signal: A rising, B falling
        digitalWrite(X_PIN_A, HIGH);
        digitalWrite(X_PIN_B, HIGH);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMillis));
    }

    // ...
}

int main() {

    if (wiringPiSetup() == -1) {
        std::cerr << "Error initializing WiringPi." << std::endl;
        return 1;
    }

    initialize();

    // HTTP listener setup
    http_listener listener("http://0.0.0.0:8080");

    // Handle GET requests to the root endpoint
    listener.support(methods::GET, [](http_request request) {
        request.reply(status_codes::OK, U("Quadrature Signal Generator"));
    });

    // Handle GET requests to start quadrature signal generation
    listener.support(methods::GET, [](http_request request) {
        if (request.relative_uri().to_string() == U("/start_quadrature")) {
            int duration = 1000; // Adjust the duration based on your requirements
            generateQuadratureSignal(duration);
            request.reply(status_codes::OK, U("Quadrature signal generation started"));
        }
    });

    // Start the web server
    try {
        listener.open().wait();
        std::cout << "Listening for requests at: " << listener.uri().to_string() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Keep the application running
    std::this_thread::sleep_for(std::chrono::hours(1));

    return 0;
}
