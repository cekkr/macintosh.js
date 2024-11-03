#include "sysdeps.h"
#include "video.h"

// video_blit.h - Video blitting functions for Basilisk II
#ifndef VIDEO_BLIT_H
#define VIDEO_BLIT_H

// Struttura per informazioni sul frame buffer
struct ScreenBlitInfo {
    uint8 *screen_base;      // Puntatore al frame buffer
    int bytes_per_row;       // Bytes per riga
    int bits_per_pixel;      // Bit per pixel
    int bytes_per_pixel;     // Bytes per pixel
    uint32 red_mask;         // Maschera per componente rosso
    uint32 green_mask;       // Maschera per componente verde
    uint32 blue_mask;        // Maschera per componente blu
    uint32 alpha_mask;       // Maschera per componente alpha
    int red_shift;           // Shift per componente rosso
    int green_shift;         // Shift per componente verde
    int blue_shift;          // Shift per componente blu
    int alpha_shift;         // Shift per componente alpha
    int width;              // Larghezza dello schermo
    int height;             // Altezza dello schermo
};

extern ScreenBlitInfo screen_blit_info;

// Funzioni di utilità per le maschere di colore
inline int find_shift(uint32 mask)
{
    int shift = 0;
    if (mask) {
        while ((mask & 1) == 0) {
            shift++;
            mask >>= 1;
        }
    }
    return shift;
}

// Inizializzazione delle maschere di colore
inline void init_blit_info(void)
{
    screen_blit_info.red_shift = find_shift(screen_blit_info.red_mask);
    screen_blit_info.green_shift = find_shift(screen_blit_info.green_mask);
    screen_blit_info.blue_shift = find_shift(screen_blit_info.blue_mask);
    screen_blit_info.alpha_shift = find_shift(screen_blit_info.alpha_mask);
    screen_blit_info.bytes_per_pixel = (screen_blit_info.bits_per_pixel + 7) / 8;
}

// Prototipi per le funzioni di blitting
extern void (*VideoRefreshInit)(void);
//extern void (*VideoRefresh)(int x, int y, int w, int h);
extern void (*VideoRefreshScreen)(void);

// Init blitters
extern void Screen_blitter_init(void);

// Funzioni di blitting per diverse profondità di colore
extern void Screen_blit_1(int x, int y, int w, int h);
extern void Screen_blit_2(int x, int y, int w, int h);
extern void Screen_blit_4(int x, int y, int w, int h);
extern void Screen_blit_8(int x, int y, int w, int h);
extern void Screen_blit_16(int x, int y, int w, int h);
extern void Screen_blit_24(int x, int y, int w, int h);
extern void Screen_blit_32(int x, int y, int w, int h);

// Funzioni di blitting dirette per SDL
extern void Screen_blit_direct_1(int x, int y, int w, int h);
extern void Screen_blit_direct_2(int x, int y, int w, int h);
extern void Screen_blit_direct_4(int x, int y, int w, int h);
extern void Screen_blit_direct_8(int x, int y, int w, int h);
extern void Screen_blit_direct_16(int x, int y, int w, int h);
extern void Screen_blit_direct_24(int x, int y, int w, int h);
extern void Screen_blit_direct_32(int x, int y, int w, int h);

// Funzioni di supporto per il blitting
extern void Screen_blit_update(int x, int y, int w, int h);
extern void Screen_blit_complete(void);

#endif // VIDEO_BLIT_H
