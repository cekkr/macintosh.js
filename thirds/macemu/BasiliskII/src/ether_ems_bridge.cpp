// ether_ems_bridge.cpp
#include "sysdeps.h"
#include "main.h"
#include "ether.h"
#include "ether_defs.h"
#include <emscripten.h>

// Funzioni esportate per JavaScript
extern "C" {
    // Wrapper per ricevere pacchetti da JavaScript
    EMSCRIPTEN_KEEPALIVE
    void ems_ether_input(uint8_t* packet, int length) {
        // Usiamo le strutture dati esistenti di Basilisk
        ether_add_packet(packet, length);
    }

    // Esporta il MAC address per JavaScript
    EMSCRIPTEN_KEEPALIVE
    void ems_get_mac(uint8_t* out_mac) {
        const uint8_t* mac = ether_get_addr();
        memcpy(out_mac, mac, 6);
    }
}

// Override della funzione di output
void ether_raw_send_packet(uint8_t *packet, int length) {
    EM_ASM({
        if (typeof window.basiliskSendPacket === 'function') {
            const data = new Uint8Array(HEAPU8.buffer, $0, $1);
            // Crea una copia del pacchetto per JavaScript
            const packet = new Uint8Array(data);
            window.basiliskSendPacket(packet);
        }
    }, packet, length);
}