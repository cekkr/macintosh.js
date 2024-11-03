cmake_minimum_required(VERSION 3.5)
project(basilisk)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opzioni Emscripten
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Flags Emscripten
set(COMMON_FLAGS
    -O3
    -s USE_SDL=2
    -s WASM=1
    -s ASYNCIFY
    -s FORCE_FILESYSTEM=1
    -s ALLOW_MEMORY_GROWTH=1
    -s TOTAL_MEMORY=268435456
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS','IDBFS']
    -s EXPORTED_FUNCTIONS=['_main','_malloc','_free','_ems_ether_input','_ems_get_mac']
    -s INVOKE_RUN=0
    -s MODULARIZE=1
    -s ENVIRONMENT=web,worker
    -s PROXY_TO_PTHREAD=0
    -s SINGLE_FILE=1
    --no-entry
    -pthread
)

string(REPLACE ";" " " EMSCRIPTEN_FLAGS "${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")

# Sorgenti
file(GLOB SOURCES 
    "src/*.cpp"
    "src/SDL/*.cpp"
    "src/Unix/*.cpp"
)

# Aggiungi il bridge ethernet
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/ether_web.cpp")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Unix
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SDL
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uae_cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CrossPlatform
)

# Definizioni
add_definitions(
    -DREAL_ADDRESSING=0
    -DADDRESS_SPACE_24=1
    -DUNIX_SOCKET_NETWORKING=0
    -DBSD_SOCKET_NETWORKING=0
    -DDISABLE_VOSF=1
    -DUSE_SDL=1
    -DUSE_SDL_VIDEO=1
    -DUSE_SDL_AUDIO=1
    -DEMSCRIPTEN
    -DNO_NATIVE_VM_ALLOC
)

# Target
add_executable(basilisk ${SOURCES})

# Aggiungi flag specifici per C99
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

string(REPLACE ";" " " EMSCRIPTEN_FLAGS "${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")

# Exported functions
set(EXPORTED_FUNCTIONS
    '_ems_ether_input'
    '_ems_get_mac',
    '_ether_init', 
    '_ether_send', 
    '_ether_recv',
    '_malloc'
    '_free'
)
set(EXPORTED_RUNTIME_METHODS
    'ccall'
    'cwrap'
    'HEAPU8'
)