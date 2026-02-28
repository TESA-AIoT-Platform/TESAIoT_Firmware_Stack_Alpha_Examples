/******************************************************************************
 * File Name : beep_generator.h
 *
 * Description : Header file containing beep generator function prototypes
 *               and definitions for generating sine wave tones.
 ********************************************************************************
 */

#ifndef __BEEP_GENERATOR_H__
#define __BEEP_GENERATOR_H__

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/
/* Sampling rate in Hz (16 kHz) */
#define BEEP_SAMPLE_RATE_HZ (16000u)

/* Beep generator buffer size - one period at 16kHz for low notes */
#define BEEP_BUFFER_SIZE (64u)

/* Default beep parameters */
#define DEFAULT_BEEP_FREQUENCY_HZ (1000u)
#define DEFAULT_BEEP_AMPLITUDE (16384) /* ~50% of 16-bit range */
#define DEFAULT_BEEP_DURATION_MS (200u)

/* Simple amplitude envelope to reduce clicks (ms) */
#define BEEP_ATTACK_MS  (5u)
#define BEEP_RELEASE_MS (10u)

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/

/**
 * @brief Musical key enumeration for transposition
 *
 * Each key represents a semitone shift from C (base key).
 * Multiply base frequencies by the corresponding multiplier.
 */
typedef enum {
    KEY_C = 0,      /**< C major (no transposition) */
    KEY_C_SHARP,    /**< C# major (+1 semitone) */
    KEY_D,          /**< D major (+2 semitones) */
    KEY_D_SHARP,    /**< D# major (+3 semitones) */
    KEY_E,          /**< E major (+4 semitones) */
    KEY_F,          /**< F major (+5 semitones) */
    KEY_F_SHARP,    /**< F# major (+6 semitones) */
    KEY_G,          /**< G major (+7 semitones) */
    KEY_G_SHARP,    /**< G# major (+8 semitones) */
    KEY_A,          /**< A major (+9 semitones) */
    KEY_A_SHARP,    /**< A# major (+10 semitones) */
    KEY_B           /**< B major (+11 semitones) */
} beep_key_t;

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern volatile bool beep_active;
extern volatile bool beep_ended;

/*******************************************************************************
 * Functions Prototypes
 *******************************************************************************/

/**
 * @brief Initialize beep generator with specified parameters
 *
 * Generates a sine wave beep at the specified frequency with the given
 * amplitude and duration. This function pre-calculates one cycle of the
 * sine wave which is then looped during playback.
 *
 * @param frequency_hz   - Beep frequency in Hz (e.g., 1000)
 * @param amplitude      - Amplitude of the sine wave (0-32767 recommended)
 *                         16384 = ~50% amplitude
 * @param duration_ms    - Duration of the beep in milliseconds
 *
 * @return None
 */
void beep_generator_init(uint16_t frequency_hz, uint16_t amplitude,
                         uint16_t duration_ms);

/**
 * @brief Get the next sample from the beep buffer
 *
 * Called by the I2S interrupt handler to get the next sample to send
 * to the I2S FIFO. Returns samples from the generated sine wave.
 *
 * @return 16-bit audio sample (signed)
 */
int16_t beep_generator_get_next_sample(void);

/**
 * @brief Check if beep playback is complete
 *
 * @return true if beep has finished playing, false otherwise
 */
bool beep_generator_is_complete(void);

/**
 * @brief Stop beep generation
 *
 * Immediately stops beep playback and resets the generator state.
 *
 * @return None
 */
void beep_generator_stop(void);

/**
 * @brief Get beep generation status
 *
 * @return true if beep is currently active, false otherwise
 */
bool beep_generator_is_active(void);

/**
 * @brief Example 1: Generate a simple beep
 *
 * Generates a 1000 Hz beep with 200ms duration at 50% amplitude.
 * This is a simple example of using the beep generator.
 *
 * @return None
 */
void beep_example_1(void);

/**
 * @brief Play C-major scale forever via I2S
 */
void beep_play_scale_forever(void);

/**
 * @brief Start continuous C-major scale playback
 */
void beep_scale_start(void);

/**
 * @brief Stop continuous C-major scale playback and deactivate I2S
 */
void beep_scale_stop(void);

/**
 * @brief Check if the continuous scale playback is running
 */
bool beep_scale_is_running(void);

/**
 * @brief Service loop that runs forever and plays the scale when enabled
 */
void beep_scale_service_loop(void);

/**
 * @brief Play song sequence forever (10 lines melody)
 */
void beep_play_song_2(void);

/**
 * @brief Set which song to play (0 or 1)
 */
void beep_set_song(uint8_t song_number);

/**
 * @brief Service loop with song selection support
 */
void beep_song_service_loop(void);

/**
 * @brief Set the musical key for transposition
 *
 * Sets the key for all subsequent notes. Applies a frequency multiplier
 * to transpose all frequencies to the selected key.
 *
 * @param key - Target musical key (KEY_C to KEY_B)
 *
 * @return None
 *
 * @note Changes take effect on the next note played.
 *       Current note will finish in the previous key.
 */
void beep_set_key(beep_key_t key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BEEP_GENERATOR_H__ */
/* [] END OF FILE */
