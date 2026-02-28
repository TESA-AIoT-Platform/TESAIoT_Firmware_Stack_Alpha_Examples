/******************************************************************************
 * File Name : beep_generator.c
 *
 * Description : Source file containing beep generator implementation for
 *               generating sine wave tones via I2S.
 ********************************************************************************
 */

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "beep_generator.h"
#include "app_i2s.h"
#include "cy_pdl.h"
#include <math.h>
#include <string.h>

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
/* Beep status flags */
volatile bool beep_active = false;
volatile bool beep_ended = false;
/* Scale playback running flag */
static volatile bool beep_scale_running = false;
/* Current song selection (0 or 1) */
static volatile uint8_t beep_current_song = 0;

/* Key transposition system */
static const float key_multipliers[] = {
    1.00000f,  /* KEY_C       - Base key */
    1.05946f,  /* KEY_C_SHARP - +1 semitone */
    1.12246f,  /* KEY_D       - +2 semitones */
    1.18921f,  /* KEY_D_SHARP - +3 semitones */
    1.25992f,  /* KEY_E       - +4 semitones */
    1.33484f,  /* KEY_F       - +5 semitones */
    1.41421f,  /* KEY_F_SHARP - +6 semitones */
    1.49831f,  /* KEY_G       - +7 semitones */
    1.58740f,  /* KEY_G_SHARP - +8 semitones */
    1.68179f,  /* KEY_A       - +9 semitones */
    1.78180f,  /* KEY_A_SHARP - +10 semitones */
    1.88775f   /* KEY_B       - +11 semitones */
};
static beep_key_t current_key = KEY_C;

/* Beep waveform buffer */
static int16_t beep_buffer[BEEP_BUFFER_SIZE];
static uint32_t beep_buffer_size = 0;

/* Beep playback tracking */
static uint32_t beep_sample_index = 0;
static uint32_t beep_duration_samples = 0;
static uint32_t beep_samples_played = 0;

/*******************************************************************************
 * Function Name: beep_generator_init
 ********************************************************************************
 * Summary: Initialize beep generator with specified frequency, amplitude, and
 *          duration. Generates a sine wave beep.
 *
 * Parameters:
 *  frequency_hz   - Beep frequency in Hz
 *  amplitude      - Amplitude of the sine wave (0-32767 recommended)
 *  duration_ms    - Duration of the beep in milliseconds
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void beep_generator_init(uint16_t frequency_hz, uint16_t amplitude,
                         uint16_t duration_ms) {
  /* Apply key transposition */
  float transposed_freq = (float)frequency_hz * key_multipliers[current_key];
  
  /* Calculate samples per cycle using transposed frequency */
  uint32_t samples_per_cycle = BEEP_SAMPLE_RATE_HZ / (uint32_t)transposed_freq;

  /* Limit buffer size to prevent overflow */
  beep_buffer_size = (samples_per_cycle > BEEP_BUFFER_SIZE) ? BEEP_BUFFER_SIZE
                                                            : samples_per_cycle;

  /* Generate sine wave for one cycle */
  for (uint32_t i = 0; i < beep_buffer_size; i++) {
    /* Generate sine wave: amplitude * sin(2*pi*i/samples_per_cycle) */
    float angle = 2.0f * 3.14159265f * i / samples_per_cycle;
    int16_t sample = (int16_t)(amplitude * sinf(angle));
    beep_buffer[i] = sample;
  }

  /* Calculate total samples to play */
  beep_duration_samples = (BEEP_SAMPLE_RATE_HZ / 1000) * duration_ms;

  /* Reset playback counters */
  beep_sample_index = 0;
  beep_samples_played = 0;

  /* Set status flags */
  beep_active = true;
  beep_ended = false;
}

/*******************************************************************************
 * Function Name: beep_generator_get_next_sample
 ********************************************************************************
 * Summary: Get the next sample from the beep buffer.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  16-bit audio sample (signed)
 *
 *******************************************************************************/
int16_t beep_generator_get_next_sample(void) {
  if (!beep_active) {
    return 0;
  }

  /* Get current sample from buffer */
  int16_t sample = beep_buffer[beep_sample_index];

  /* Apply simple attack/release envelope to reduce clicks */
  uint32_t attack_samples = (BEEP_SAMPLE_RATE_HZ / 1000u) * BEEP_ATTACK_MS;
  uint32_t release_samples = (BEEP_SAMPLE_RATE_HZ / 1000u) * BEEP_RELEASE_MS;
  if (release_samples > beep_duration_samples) {
    release_samples = beep_duration_samples;
  }

  float gain = 1.0f;
  if (beep_samples_played < attack_samples && attack_samples > 0u) {
    gain = (float)beep_samples_played / (float)attack_samples;
  } else if (beep_samples_played + release_samples > beep_duration_samples && release_samples > 0u) {
    /* Near end, fade out */
    uint32_t remaining = (beep_duration_samples > beep_samples_played)
                             ? (beep_duration_samples - beep_samples_played)
                             : 0u;
    gain = (float)remaining / (float)release_samples;
    if (gain < 0.0f) {
      gain = 0.0f;
    }
  }
  sample = (int16_t)((float)sample * gain);

  /* Move to next sample in cycle */
  beep_sample_index = (beep_sample_index + 1) % beep_buffer_size;
  beep_samples_played++;

  /* Check if beep duration has ended */
  if (beep_samples_played >= beep_duration_samples) {
    beep_active = false;
    beep_ended = true;
    beep_sample_index = 0;
    beep_samples_played = 0;
  }

  return sample;
}

/*******************************************************************************
 * Function Name: beep_generator_is_complete
 ********************************************************************************
 * Summary: Check if beep playback is complete.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  true if beep has finished playing, false otherwise
 *
 *******************************************************************************/
bool beep_generator_is_complete(void) { return beep_ended; }

/*******************************************************************************
 * Function Name: beep_generator_stop
 ********************************************************************************
 * Summary: Stop beep generation and reset state.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void beep_generator_stop(void) {
  beep_active = false;
  beep_ended = true;
  beep_sample_index = 0;
  beep_samples_played = 0;
}

/*******************************************************************************
 * Function Name: beep_generator_is_active
 ********************************************************************************
 * Summary: Get beep generation status.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  true if beep is currently active, false otherwise
 *
 *******************************************************************************/
bool beep_generator_is_active(void) { return beep_active; }

/*******************************************************************************
 * Function Name: beep_example_1
 ********************************************************************************
 * Summary: Example 1 - Generate a simple beep tone.
 *
 * Generates a 1000 Hz beep with 200ms duration at 50% amplitude.
 * This demonstrates basic usage of the beep generator.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void beep_example_1(void) {
  /* Initialize beep: 1000 Hz, 50% amplitude, 200ms duration */
  beep_generator_init(1000, 16384, 200);
}

/**
 * Play C-major scale forever: Do Re Mi Fa Sol La Ti Do.
 * Keeps I2S active and feeds samples via the beep ISR.
 */
void beep_play_scale_forever(void) {
  /* Frequencies for C major scale: C4 D4 E4 F4 G4 A4 B4 C5 */
  static const uint16_t scale_freqs[] = {262, 294, 330, 349, 392, 440, 494, 523};
  const uint16_t amp = 16384;      /* ~50% amplitude */
  const uint16_t note_ms = 300;    /* note duration */
  const uint16_t gap_ms = 50;      /* gap between notes */

  /* Ensure I2S is enabled and active */
  app_i2s_enable();
  app_i2s_activate();

  for (;;) {
    uint32_t count = (uint32_t)(sizeof(scale_freqs) / sizeof(scale_freqs[0]));
    for (uint32_t i = 0; i < count; i++) {
      beep_generator_init(scale_freqs[i], amp, note_ms);

      /* Wait until beep finishes */
      while (!beep_generator_is_complete()) {
        Cy_SysLib_Delay(1);
      }

      /* Small silence between notes */
      Cy_SysLib_Delay(gap_ms);
    }
  }
}

/* Start continuous scale playback */
void beep_scale_start(void) {
  beep_scale_running = true;
  app_i2s_enable();
  app_i2s_activate();
}

/* Stop continuous scale playback */
void beep_scale_stop(void) {
  beep_scale_running = false;
  beep_generator_stop();
  app_i2s_deactivate();
}

/* Query scale playback status */
bool beep_scale_is_running(void) { return beep_scale_running; }

/* Forever service loop: plays scale when enabled */
void beep_scale_service_loop(void) {
  /* Musical sequence: Do Re Mi Re Do, Mi Fa Sol Sol, Mi Fa Sol Sol, Sol La Sol Fa Mi Re Do */
  static const uint16_t scale_freqs[] = {
    262, 294, 330, 294, 262,  /* Do Re Mi Re Do */
    330, 349, 392, 392,        /* Mi Fa Sol Sol */
    330, 349, 392, 392,        /* Mi Fa Sol Sol */
    392, 440, 392, 349, 330, 294, 262  /* Sol La Sol Fa Mi Re Do */
  };
  const uint16_t amp = 16384;
  const uint16_t note_ms = 300;
  const uint16_t gap_ms = 50;

  for (;;) {
    if (beep_scale_running) {
      uint32_t count = (uint32_t)(sizeof(scale_freqs) / sizeof(scale_freqs[0]));
      for (uint32_t i = 0; i < count; i++) {
        if (!beep_scale_running) {
          break;
        }
        beep_generator_init(scale_freqs[i], amp, note_ms);
        while (beep_scale_running && !beep_generator_is_complete()) {
          Cy_SysLib_Delay(1);
        }
        if (!beep_scale_running) {
          break;
        }
        Cy_SysLib_Delay(gap_ms);
      }
    } else {
      /* Not running: brief idle */
      Cy_SysLib_Delay(10);
    }
  }
}

/*******************************************************************************
 * Function Name: beep_play_song_2
 ********************************************************************************
 * Summary: Play a longer musical sequence (10-line melody)
 *
 * Sequence:
 * Sol La Sol Mi  Sol La Sol Mi  
 * Re Mi Sol Mi  Re Do  
 * Sol La Sol Mi  Sol La Sol Mi  
 * Re Mi Sol Mi  Re Do  
 * 
 * Mi Sol La Do  La Sol Mi  
 * Re Mi Sol Mi  Re Do  
 * Mi Sol La Do  La Sol Mi  
 * Re Mi Sol Mi  Re Do  
 * 
 * Sol La Do La  Sol Mi  
 * Re Mi Sol Mi  Re Do  Do
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void beep_play_song_2(void) {
  /* Song sequence with all notes */
  static const uint16_t song_freqs[] = {
    /* Sol La Sol Mi  Sol La Sol Mi */
    392, 440, 392, 330, 392, 440, 392, 330,
    /* Re Mi Sol Mi  Re Do */
    294, 330, 392, 330, 294, 262,
    /* Sol La Sol Mi  Sol La Sol Mi */
    392, 440, 392, 330, 392, 440, 392, 330,
    /* Re Mi Sol Mi  Re Do */
    294, 330, 392, 330, 294, 262,
    
    /* Mi Sol La Do  La Sol Mi */
    330, 392, 440, 523, 440, 392, 330,
    /* Re Mi Sol Mi  Re Do */
    294, 330, 392, 330, 294, 262,
    /* Mi Sol La Do  La Sol Mi */
    330, 392, 440, 523, 440, 392, 330,
    /* Re Mi Sol Mi  Re Do */
    294, 330, 392, 330, 294, 262,
    
    /* Sol La Do La  Sol Mi */
    392, 440, 523, 440, 392, 330,
    /* Re Mi Sol Mi  Re Do  Do */
    294, 330, 392, 330, 294, 262, 262
  };
  
  const uint16_t amp = 16384;      /* ~50% amplitude */
  const uint16_t note_ms = 300;    /* note duration */
  const uint16_t gap_ms = 50;      /* gap between notes */

  /* Ensure I2S is enabled and active */
  app_i2s_enable();
  app_i2s_activate();

  for (;;) {
    uint32_t count = (uint32_t)(sizeof(song_freqs) / sizeof(song_freqs[0]));
    for (uint32_t i = 0; i < count; i++) {
      beep_generator_init(song_freqs[i], amp, note_ms);

      /* Wait until beep finishes */
      while (!beep_generator_is_complete()) {
        Cy_SysLib_Delay(1);
      }

      /* Small silence between notes */
      Cy_SysLib_Delay(gap_ms);
    }
    
    /* Brief pause between repetitions */
    Cy_SysLib_Delay(500);
  }
}

/* Set which song to play */
void beep_set_song(uint8_t song_number) {
  beep_current_song = song_number;
}

/* Service loop with song selection - plays selected song when enabled */
void beep_song_service_loop(void) {
  /* Duration definitions: short=300ms, mid=500ms, long=700ms, extra_long=1000ms */
  #define DUR_SHORT 300
  #define DUR_MID   500
  #define DUR_LONG  700
  #define DUR_XLONG 1000
  
  /* Song 0: Extended melody with 4 verses and variable note durations */
  static const uint16_t song0_freqs[] = {
    /* Verse 1: Do Re Mi– Re Do– | Mi Fa Sol– Sol– | Mi Fa Sol– Sol– | Sol– La Sol Fa Mi Re Do–– */
    262, 294, 330, 294, 262,        /* Do  Re  Mi–  Re  Do– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    392, 440, 392, 349, 330, 294, 262,  /* Sol–  La  Sol  Fa  Mi  Re  Do–– */
    /* Verse 2: Do Re Mi– Re Do– | Mi Fa Sol– Sol– | Mi Fa Sol– Sol– | Sol– La Sol Fa Mi Re Do–– */
    262, 294, 330, 294, 262,        /* Do  Re  Mi–  Re  Do– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    392, 440, 392, 349, 330, 294, 262,  /* Sol–  La  Sol  Fa  Mi  Re  Do–– */
    /* Verse 3: Mi Fa Sol La– Sol– | Mi Fa Sol La– Sol– | Sol– La Do– La Sol– | Fa Mi Re Do–– */
    330, 349, 392, 440, 392,        /* Mi  Fa  Sol  La–  Sol– */
    330, 349, 392, 440, 392,        /* Mi  Fa  Sol  La–  Sol– */
    392, 440, 523, 440, 392,        /* Sol–  La  Do–  La  Sol– */
    349, 330, 294, 262,              /* Fa  Mi  Re  Do–– */
    /* Verse 4: Do Re Mi– Re Do– | Mi Fa Sol– Sol– | Mi Fa Sol– Sol– | Sol– La Sol Fa Mi Re Do––– */
    262, 294, 330, 294, 262,        /* Do  Re  Mi–  Re  Do– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    330, 349, 392, 392,              /* Mi  Fa  Sol–  Sol– */
    392, 440, 392, 349, 330, 294, 262   /* Sol–  La  Sol  Fa  Mi  Re  Do––– */
  };
  
  static const uint16_t song0_durs[] = {
    /* Verse 1 */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_SHORT, DUR_MID,  /* Do  Re  Mi–  Re  Do– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_MID, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_LONG,  /* Sol–  La  Sol  Fa  Mi  Re  Do–– */
    /* Verse 2 */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_SHORT, DUR_MID,  /* Do  Re  Mi–  Re  Do– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_MID, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_LONG,  /* Sol–  La  Sol  Fa  Mi  Re  Do–– */
    /* Verse 3 */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,  /* Mi  Fa  Sol  La–  Sol– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,  /* Mi  Fa  Sol  La–  Sol– */
    DUR_MID, DUR_SHORT, DUR_MID, DUR_SHORT, DUR_MID,    /* Sol–  La  Do–  La  Sol– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_LONG,          /* Fa  Mi  Re  Do–– */
    /* Verse 4 */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_SHORT, DUR_MID,  /* Do  Re  Mi–  Re  Do– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,             /* Mi  Fa  Sol–  Sol– */
    DUR_MID, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_XLONG   /* Sol–  La  Sol  Fa  Mi  Re  Do––– */
  };
  
  /* Song 1: Sol La Sol Mi... with variable durations */
  static const uint16_t song1_freqs[] = {
    /* Sol La Sol– Mi– | Sol La Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    392, 440, 392, 330,
    392, 440, 392, 330,
    294, 330, 392, 330,
    294, 262,
    /* Sol La Sol– Mi– | Sol La Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    392, 440, 392, 330,
    392, 440, 392, 330,
    294, 330, 392, 330,
    294, 262,
    /* Mi– Sol La– Do– | La– Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    330, 392, 440, 523,
    440, 392, 330,
    294, 330, 392, 330,
    294, 262,
    /* Mi– Sol La– Do– | La– Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    330, 392, 440, 523,
    440, 392, 330,
    294, 330, 392, 330,
    294, 262,
    /* Sol– La Do– La– | Sol– Mi– | Re Mi Sol– Mi– | Re Do––– */
    392, 440, 523, 440,
    392, 330,
    294, 330, 392, 330,
    294, 262
  };
  
  static const uint16_t song1_durs[] = {
    /* Sol La Sol– Mi– | Sol La Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_LONG,
    /* Sol La Sol– Mi– | Sol La Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_LONG,
    /* Mi– Sol La– Do– | La– Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    DUR_MID, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_MID, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_LONG,
    /* Mi– Sol La– Do– | La– Sol– Mi– | Re Mi Sol– Mi– | Re Do–– */
    DUR_MID, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_MID, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_LONG,
    /* Sol– La Do– La– | Sol– Mi– | Re Mi Sol– Mi– | Re Do––– */
    DUR_MID, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_MID, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    DUR_SHORT, DUR_XLONG
  };
  
  /* Song 2: Happy Birthday melody */
  static const uint16_t song2_freqs[] = {
    /* Sol Sol La Sol Do– Ti– */
    392, 392, 440, 392, 523, 494,
    /* Sol Sol La Sol Re– Do– */
    392, 392, 440, 392, 587, 523,
    /* Sol Sol Sol Mi– Do– Ti– La– */
    392, 392, 784, 659, 523, 494, 440,
    /* Fa– Fa Mi Do– Re– Do–– */
    349, 349, 659, 523, 587, 523
  };
  
  static const uint16_t song2_durs[] = {
    /* Sol Sol La Sol Do– Ti– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    /* Sol Sol La Sol Re– Do– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    /* Sol Sol Sol Mi– Do– Ti– La– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID, DUR_MID, DUR_MID,
    /* Fa– Fa Mi Do– Re– Do–– */
    DUR_MID, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID, DUR_LONG
  };
  
  /* Song 3: Simple melody */
  static const uint16_t song3_freqs[] = {
    /* Do Do Do Re Mi– | Mi Re Do– */
    262, 262, 262, 294, 330,
    330, 294, 262,
    /* Mi Mi Fa Sol– | Sol Fa Mi– */
    330, 330, 349, 392,
    392, 349, 330,
    /* Do Do Do Re Mi– | Mi Re Do– */
    262, 262, 262, 294, 330,
    330, 294, 262,
    /* Mi Sol La Sol Mi– | Re Do–– */
    330, 392, 440, 392, 330,
    294, 262
  };
  
  static const uint16_t song3_durs[] = {
    /* Do Do Do Re Mi– | Mi Re Do– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID,
    /* Mi Mi Fa Sol– | Sol Fa Mi– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID,
    /* Do Do Do Re Mi– | Mi Re Do– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID,
    /* Mi Sol La Sol Mi– | Re Do–– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_LONG
  };
  
  /* Song 4: Jingle Bells */
  static const uint16_t song4_freqs[] = {
    /* Mi Mi Mi– | Mi Mi Mi– | Mi Sol Do Re Mi– */
    330, 330, 330,
    330, 330, 330,
    330, 392, 262, 294, 330,
    /* Fa Fa Fa Fa– | Fa Mi Mi Mi– | Mi Re Re Mi Re– Sol– */
    349, 349, 349, 349,
    349, 330, 330, 330,
    330, 294, 294, 330, 294, 392,
    /* Mi Mi Mi– | Mi Mi Mi– | Mi Sol Do Re Mi– */
    330, 330, 330,
    330, 330, 330,
    330, 392, 262, 294, 330,
    /* Fa Fa Fa Fa– | Fa Mi Mi Mi– | Sol Sol Fa Re Do–– */
    349, 349, 349, 349,
    349, 330, 330, 330,
    392, 392, 349, 294, 262
  };
  
  static const uint16_t song4_durs[] = {
    /* Mi Mi Mi– | Mi Mi Mi– | Mi Sol Do Re Mi– */
    DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    /* Fa Fa Fa Fa– | Fa Mi Mi Mi– | Mi Re Re Mi Re– Sol– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID, DUR_MID,
    /* Mi Mi Mi– | Mi Mi Mi– | Mi Sol Do Re Mi– */
    DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    /* Fa Fa Fa Fa– | Fa Mi Mi Mi– | Sol Sol Fa Re Do–– */
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_MID,
    DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_SHORT, DUR_LONG
  };
  
  const uint16_t amp = 16384;
  const uint16_t gap_ms = 50;

  for (;;) {
    if (beep_scale_running) {
      /* Select song based on current setting */
      const uint16_t* song_freqs;
      const uint16_t* song_durs;
      uint32_t count;
      
      if (beep_current_song == 0) {
        song_freqs = song0_freqs;
        song_durs = song0_durs;
        count = (uint32_t)(sizeof(song0_freqs) / sizeof(song0_freqs[0]));
      } else if (beep_current_song == 1) {
        song_freqs = song1_freqs;
        song_durs = song1_durs;
        count = (uint32_t)(sizeof(song1_freqs) / sizeof(song1_freqs[0]));
      } else if (beep_current_song == 2) {
        song_freqs = song2_freqs;
        song_durs = song2_durs;
        count = (uint32_t)(sizeof(song2_freqs) / sizeof(song2_freqs[0]));
      } else if (beep_current_song == 3) {
        song_freqs = song3_freqs;
        song_durs = song3_durs;
        count = (uint32_t)(sizeof(song3_freqs) / sizeof(song3_freqs[0]));
      } else {
        song_freqs = song4_freqs;
        song_durs = song4_durs;
        count = (uint32_t)(sizeof(song4_freqs) / sizeof(song4_freqs[0]));
      }
      
      for (uint32_t i = 0; i < count; i++) {
        if (!beep_scale_running) {
          break;
        }
        beep_generator_init(song_freqs[i], amp, song_durs[i]);
        while (beep_scale_running && !beep_generator_is_complete()) {
          Cy_SysLib_Delay(1);
        }
        if (!beep_scale_running) {
          break;
        }
        Cy_SysLib_Delay(gap_ms);
      }
      
      /* Brief pause between repetitions */
      if (beep_scale_running) {
        Cy_SysLib_Delay(500);
      }
    } else {
      /* Not running: brief idle */
      Cy_SysLib_Delay(10);
    }
  }
  
  #undef DUR_SHORT
  #undef DUR_MID
  #undef DUR_LONG
  #undef DUR_XLONG
}

/*******************************************************************************
 * Function Name: beep_set_key
 ********************************************************************************
 * Summary: Set the musical key for transposition.
 *
 * Parameters:
 *  key - Target musical key (KEY_C to KEY_B)
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void beep_set_key(beep_key_t key) {
  if (key <= KEY_B) {
    current_key = key;
  }
}



/*
  // Example usage:

  #include "source/app_i2s/beep_generator.h"
  #include "source/app_i2s/beep_i2s.h"

  //>> Auto-start playback with Song 4 (Jingle Bells)
  beep_set_song(4);
  beep_scale_start();
  Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, CYBSP_LED_STATE_ON);

  //>> Run song service loop
  beep_song_service_loop();
*/

/* [] END OF FILE */
