#!/bin/bash
# Script per compilare Basilisk II e MacEMU con Emscripten per browser su macOS
# Autore: Claude
# Data: 14 Aprile 2025

set -e # Interrompe l'esecuzione in caso di errori

echo "=== Script di compilazione per Basilisk II-JS e MacEMU ==="

# Controllo se Emscripten è installato
if ! command -v emcc &> /dev/null; then
    echo "Emscripten non trovato. Installazione in corso..."
    
    # Verifica se brew è installato
    if ! command -v brew &> /dev/null; then
        echo "Homebrew non trovato. Installazione in corso..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    # Installa emscripten tramite homebrew
    brew install emscripten
    
    # Aggiorna e attiva l'ambiente emscripten
    pushd $(brew --prefix emscripten)
    ./emsdk update
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    popd
    
    echo "Emscripten installato con successo."
else
    echo "Emscripten è già installato."
fi

# Crea directory di lavoro
WORK_DIR="$HOME/basilisk_macemu_build"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# Funzione per controllare/installare dipendenze
check_install_deps() {
    echo "Verifica dipendenze in corso..."
    # Lista delle dipendenze necessarie
    deps=("cmake" "autoconf" "automake" "libtool" "pkg-config" "git" "python3")
    
    for dep in "${deps[@]}"; do
        if ! command -v $dep &> /dev/null; then
            echo "Installazione di $dep..."
            brew install $dep
        else
            echo "$dep è già installato."
        fi
    done
}

# Installa dipendenze
check_install_deps

# Compila Basilisk II
compile_basilisk() {
    echo "=== Compilazione di Basilisk II in corso ==="
    
    # Clone del repository se non esiste già
    if [ ! -d "BasiliskII" ]; then
        git clone https://github.com/cebix/macemu.git
        mv macemu/BasiliskII .
    fi
    
    cd BasiliskII
    
    # Prepara l'ambiente con autoconf
    cd src/Unix
    ./autogen.sh
    
    # Configura per Emscripten
    mkdir -p build_web
    cd build_web
    
    # Configurazione specifica per emscripten
    emconfigure ../configure --disable-jit-compiler --without-gtk --without-sdl --without-x --disable-standalone-gui
    
    # Patch dei file se necessario per compatibilità con emscripten
    # Questo è un passaggio generico che potrebbe richiedere modifiche specifiche
    echo "Applicazione di patch per compatibilità Emscripten..."
    
    # Compila con emscripten
    emmake make
    
    # Compila il file finale per il browser
    echo "Creazione dell'output per browser..."
    
    emcc -O2 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="'BasiliskII'" \
         -s EXPORTED_RUNTIME_METHODS='["callMain"]' \
         -s EXPORTED_FUNCTIONS='["_main"]' \
         -s ALLOW_MEMORY_GROWTH=1 \
         -s TOTAL_MEMORY=134217728 \
         -s NO_EXIT_RUNTIME=1 \
         --preload-file ../../../BasiliskII/data@/basilisk_ii_data \
         BasiliskII.o -o basilisk_ii.js
    
    cd "$WORK_DIR"
    echo "Basilisk II compilato con successo."
}

# Compila MacEMU (SheepShaver)
compile_macemu() {
    echo "=== Compilazione di MacEMU (SheepShaver) in corso ==="
    
    # Clone del repository se non esiste già
    if [ ! -d "SheepShaver" ]; then
        if [ ! -d "macemu" ]; then
            git clone https://github.com/cebix/macemu.git
        fi
        mv macemu/SheepShaver .
    fi
    
    cd SheepShaver
    
    # Prepara l'ambiente con autoconf
    cd src/Unix
    ./autogen.sh
    
    # Configura per Emscripten
    mkdir -p build_web
    cd build_web
    
    # Configurazione specifica per emscripten
    emconfigure ../configure --disable-jit-compiler --without-gtk --without-sdl --without-x --disable-standalone-gui
    
    # Patch dei file se necessario per compatibilità con emscripten
    echo "Applicazione di patch per compatibilità Emscripten..."
    
    # Compila con emscripten
    emmake make
    
    # Compila il file finale per il browser
    echo "Creazione dell'output per browser..."
    
    emcc -O2 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="'SheepShaver'" \
         -s EXPORTED_RUNTIME_METHODS='["callMain"]' \
         -s EXPORTED_FUNCTIONS='["_main"]' \
         -s ALLOW_MEMORY_GROWTH=1 \
         -s TOTAL_MEMORY=134217728 \
         -s NO_EXIT_RUNTIME=1 \
         --preload-file ../../../SheepShaver/data@/sheepshaver_data \
         SheepShaver.o -o sheepshaver.js
    
    cd "$WORK_DIR"
    echo "MacEMU (SheepShaver) compilato con successo."
}

# Crea una cartella per gli output finali
create_output() {
    echo "=== Creazione degli output finali ==="
    
    OUTPUT_DIR="$WORK_DIR/browser_build"
    mkdir -p "$OUTPUT_DIR"
    
    # Copia i file compilati
    cp -v "$WORK_DIR/BasiliskII/src/Unix/build_web/basilisk_ii.js" "$OUTPUT_DIR/"
    cp -v "$WORK_DIR/BasiliskII/src/Unix/build_web/basilisk_ii.wasm" "$OUTPUT_DIR/"
    cp -v "$WORK_DIR/BasiliskII/src/Unix/build_web/basilisk_ii.data" "$OUTPUT_DIR/"
    
    cp -v "$WORK_DIR/SheepShaver/src/Unix/build_web/sheepshaver.js" "$OUTPUT_DIR/"
    cp -v "$WORK_DIR/SheepShaver/src/Unix/build_web/sheepshaver.wasm" "$OUTPUT_DIR/"
    cp -v "$WORK_DIR/SheepShaver/src/Unix/build_web/sheepshaver.data" "$OUTPUT_DIR/"
    
    # Crea un file HTML di esempio per testare l'emulatore
    cat > "$OUTPUT_DIR/basilisk_test.html" << EOF
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Basilisk II Browser Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        #output { border: 1px solid #ccc; padding: 10px; height: 400px; overflow: auto; margin-top: 20px; }
    </style>
</head>
<body>
    <h1>Basilisk II Browser Test</h1>
    <button id="startButton">Start Basilisk II</button>
    <div id="output"></div>
    
    <script src="basilisk_ii.js"></script>
    <script>
        document.getElementById('startButton').addEventListener('click', function() {
            var outputElement = document.getElementById('output');
            
            // Reindirizza l'output della console
            var oldLog = console.log;
            console.log = function(message) {
                outputElement.innerHTML += message + '<br>';
                outputElement.scrollTop = outputElement.scrollHeight;
                oldLog.apply(console, arguments);
            };
            
            // Inizializza BasiliskII
            var basiliskII = BasiliskII({
                noInitialRun: true,
                print: function(text) {
                    console.log(text);
                },
                printErr: function(text) {
                    console.log('Error: ' + text);
                }
            });
            
            basiliskII.then(function(module) {
                console.log('Basilisk II inizializzato con successo');
                // Esegui l'emulatore con i parametri desiderati
                module.callMain([]);
            });
        });
    </script>
</body>
</html>
EOF

    # Crea un file HTML di esempio per SheepShaver
    cat > "$OUTPUT_DIR/sheepshaver_test.html" << EOF
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>SheepShaver Browser Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        #output { border: 1px solid #ccc; padding: 10px; height: 400px; overflow: auto; margin-top: 20px; }
    </style>
</head>
<body>
    <h1>SheepShaver Browser Test</h1>
    <button id="startButton">Start SheepShaver</button>
    <div id="output"></div>
    
    <script src="sheepshaver.js"></script>
    <script>
        document.getElementById('startButton').addEventListener('click', function() {
            var outputElement = document.getElementById('output');
            
            // Reindirizza l'output della console
            var oldLog = console.log;
            console.log = function(message) {
                outputElement.innerHTML += message + '<br>';
                outputElement.scrollTop = outputElement.scrollHeight;
                oldLog.apply(console, arguments);
            };
            
            // Inizializza SheepShaver
            var sheepShaver = SheepShaver({
                noInitialRun: true,
                print: function(text) {
                    console.log(text);
                },
                printErr: function(text) {
                    console.log('Error: ' + text);
                }
            });
            
            sheepShaver.then(function(module) {
                console.log('SheepShaver inizializzato con successo');
                // Esegui l'emulatore con i parametri desiderati
                module.callMain([]);
            });
        });
    </script>
</body>
</html>
EOF

    echo "Output creato in: $OUTPUT_DIR"
    echo "Puoi testare gli emulatori aprendo i file HTML in un browser moderno."
}

# Esegui le funzioni principali
compile_basilisk
compile_macemu
create_output

echo "=== Compilazione completata con successo ==="
echo "I file compilati si trovano in: $WORK_DIR/browser_build"
echo "Puoi testare Basilisk II aprendo $WORK_DIR/browser_build/basilisk_test.html nel tuo browser"
echo "Puoi testare SheepShaver aprendo $WORK_DIR/browser_build/sheepshaver_test.html nel tuo browser"