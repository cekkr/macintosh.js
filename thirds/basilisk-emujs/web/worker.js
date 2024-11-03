// Import del modulo Basilisk II compilato
importScripts('basilisk.js');

let basiliskModule = null;

// Gestione messaggi dal thread principale
self.onmessage = function(e) {
    const msg = e.data;
    
    switch(msg.type) {
        case 'init':
            // Inizializza Basilisk II
            initBasilisk(msg.options);
            break;
            
        case 'network':
            // Gestisci pacchetti di rete
            if (basiliskModule) {
                const { packet, length } = msg.data;
                const heapSpace = basiliskModule._malloc(length);
                basiliskModule.HEAPU8.set(packet, heapSpace);
                basiliskModule._ems_ether_input(heapSpace, length);
                basiliskModule._free(heapSpace);
            }
            break;
    }
};

// Inizializzazione
async function initBasilisk(options) {
    const config = {
        print: text => self.postMessage({ type: 'print', text }),
        printErr: text => self.postMessage({ type: 'error', text }),
        locateFile: (path, prefix) => prefix + path,
        ...options
    };

    try {
        basiliskModule = await BasiliskII(config);
        
        // Imposta preferenze
        basiliskModule.FS.writeFile('/basilisk_prefs', `
            ethernet true
            etherconfig 0
            rom ${options.romFile}
            disk ${options.diskFile}
            frameskip 1
            modelid 5
            cpu 4
            fpu true
            screen win/800/600
            seriala /dev/null
            serialb /dev/null
        `);

        // Monta il filesystem
        basiliskModule.FS.mkdir('/mnt');
        basiliskModule.FS.mount(basiliskModule.IDBFS, {}, '/mnt');
        
        // Avvia l'emulatore
        basiliskModule._main();
        
        self.postMessage({ type: 'ready' });
    } catch (err) {
        self.postMessage({ type: 'error', text: err.toString() });
    }
}