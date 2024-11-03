// vm_alloc.h - Memory allocation for Basilisk II (Emscripten version)
#ifndef VM_ALLOC_H
#define VM_ALLOC_H

#include "sysdeps.h"
#include <emscripten.h>

// Definizioni per la protezione della memoria
#define VM_PAGE_READ    1
#define VM_PAGE_WRITE   2
#define VM_PAGE_EXECUTE 4
#define VM_PAGE_DEFAULT (VM_PAGE_READ | VM_PAGE_WRITE)

// Funzioni di allocazione memoria
static inline void *vm_acquire(size_t size, int flags = VM_PAGE_DEFAULT) 
{
    // In Emscripten, usiamo malloc standard
    void *ptr = malloc(size);
    if (ptr) {
        // Inizializza la memoria a zero
        memset(ptr, 0, size);
    }
    return ptr;
}

static inline void vm_release(void *addr, size_t size) 
{
    if (addr) {
        free(addr);
    }
}

static inline bool vm_protect(void *addr, size_t size, int prot) 
{
    // Emscripten non supporta la protezione della memoria a livello di pagina
    // Ritorniamo sempre true perché non possiamo fare molto qui
    return true;
}

// Funzione per ottenere la dimensione di una pagina
static inline size_t vm_get_page_size(void) 
{
    // Dimensione pagina standard per Emscripten/WebAssembly
    return 64 * 1024; // 64KB è la dimensione tipica di una pagina WebAssembly
}

// Arrotonda size al multiplo superiore della dimensione della pagina
static inline size_t vm_round_page_size(size_t size) 
{
    size_t page_size = vm_get_page_size();
    return (size + page_size - 1) & ~(page_size - 1);
}

#ifdef HAVE_MMAP
#undef HAVE_MMAP  // Disabilita mmap per Emscripten
#endif

// Macro di supporto per l'allocazione di memoria
#define VM_ALLOC(size) vm_acquire(size)
#define VM_FREE(addr, size) vm_release(addr, size)

// Funzioni opzionali per debug
#ifdef DEBUG
static inline void vm_dump_stats(void) 
{
    EM_ASM({
        console.log("VM stats not available in Emscripten build");
    });
}
#endif

#endif // VM_ALLOC_H
