// ether_web.cpp (nuovo file)
#include <emscripten.h>
#include <emscripten/bind.h>

// Dichiarazioni per le funzioni JS che chiameremo
EM_JS(void, js_initNetwork, (), {
    // Implementato lato JavaScript
    window.basiliskNetwork.init();
});

EM_JS(int, js_sendPacket, (const uint8_t* data, int length), {
    // Converte i dati in un array JavaScript e li invia
    const packet = HEAPU8.slice(data, data + length);
    return window.basiliskNetwork.sendPacket(packet);
});

EM_JS(int, js_receivePacket, (uint8_t* buffer, int max_length), {
    // Riceve pacchetti dalla coda JavaScript
    return window.basiliskNetwork.receivePacket(buffer, max_length);
});

// Funzione di inizializzazione della rete
bool ether_init(void) {
    js_initNetwork();
    return true;
}

// Funzione per inviare pacchetti
int32 ether_send(const uint8* packet, int32 size) {
    return js_sendPacket(packet, size);
}

// Funzione per ricevere pacchetti
int32 ether_recv(uint8* packet, int32 size) {
    return js_receivePacket(packet, size);
}