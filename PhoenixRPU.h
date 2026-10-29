// Williams Phoenix (Game 485, 1978, System 4) hardware map for RPU on System Alpha
// Source: Phoenix Operators Handbook 16P-485-100, Figures 8 (lamps), 9 (switches), 10 (solenoids), pp. 25-27
//
// NUMBERING CONVENTIONS (verify on first power-up with the RPU diagnostics):
//   Lamps:     index = (column-1)*8 + (row-1)            -> 0..63
//   Switches:  RPU index = (manual switch number - 1)    -> manual number = (column-1)*8 + row
//   Solenoids: RPU index = (test-02 solenoid number - 1)
// The manual's numbers are noted in each comment so they can be checked against the switch test (03),
// coil test (02) and lamp test (01).

/*********************************************************************
    LAMPS  (manual Figure 8; col = YEL-x, row = RED-x)
*********************************************************************/
// Column 2 (YEL-RED)
#define LAMP_1                        8   // "1"
#define LAMP_2                        9   // "2"
#define LAMP_3                        10  // "3"
#define LAMP_4                        11  // "4"
#define LAMP_5                        12  // "5"
// Column 3 (YEL-ORN)
#define LAMP_EXTRA_BALL_WHEN_LIT      16  // EXTRA BALL WHEN LIT
#define LAMP_P                        17
#define LAMP_H                        18
#define LAMP_O                        19
#define LAMP_E                        20
#define LAMP_N                        21
#define LAMP_I                        22
#define LAMP_X                        23
// Column 4 (YEL-BLK)
#define LAMP_X2                       24
#define LAMP_X3                       25
#define LAMP_X4                       26
#define LAMP_X5                       27
#define LAMP_BONUS_1K                 28
#define LAMP_BONUS_2K                 29
#define LAMP_BONUS_3K                 30
#define LAMP_BONUS_4K                 31
// Column 5 (YEL-GRN)
#define LAMP_BONUS_5K                 32
#define LAMP_BONUS_6K                 33
#define LAMP_BONUS_7K                 34
#define LAMP_BONUS_8K                 35
#define LAMP_BONUS_9K                 36
#define LAMP_BONUS_10K                38
#define LAMP_BONUS_20K                39
// Column 6 (YEL-BLU)
#define LAMP_SPINNER_1000_WL          44  // SPINNER 1,000 W/L
#define LAMP_LEFT_SPECIAL             45
#define LAMP_RIGHT_SPECIAL            46
#define LAMP_SHOOT_AGAIN              47  // SAME PLAYER SHOOTS AGAIN (playfield)
// Column 7 (YEL-VIO)
#define LAMP_HEAD_1_PLAYER            49  // 1 CAN PLAY
#define LAMP_HEAD_2_PLAYERS           50
#define LAMP_HEAD_3_PLAYERS           51
#define LAMP_HEAD_4_PLAYERS           52
#define LAMP_HEAD_MATCH               53
#define LAMP_BALL_IN_PLAY             54
#define LAMP_APRON_CREDIT             55  // CREDITS (playfield)
// Column 8 (YEL-GRY)
#define LAMP_HEAD_PLAYER_1_UP         56
#define LAMP_HEAD_PLAYER_2_UP         57
#define LAMP_HEAD_PLAYER_3_UP         58
#define LAMP_HEAD_PLAYER_4_UP         59
#define LAMP_HEAD_TILT                60
#define LAMP_HEAD_GAME_OVER           61
#define LAMP_HEAD_SHOOT_AGAIN         62  // SAME PLAYER SHOOTS (BACK BOX)
#define LAMP_HEAD_HIGH_SCORE          63
// Unused in the manual: all of column 1; col 2 rows 6-8; col 5 row 6; col 6 rows 1-4; col 7 row 1

/*********************************************************************
    SWITCHES  (manual Figure 9; manual number in comment, RPU index = number - 1)
*********************************************************************/
// Column 1 (GRN-BRN)
#define SW_PLUMB_TILT                 0   // 1
#define SW_ROLL_TILT                  1   // 2
#define SW_CREDIT_RESET               2   // 3  credit button
#define SW_COIN_3                     3   // 4  right coin
#define SW_COIN_2                     4   // 5  center coin
#define SW_COIN_1                     5   // 6  left coin
#define SW_SLAM                       6   // 7  slam tilt
#define SW_HIGH_SCORE_RESET           7   // 8  not provided on prototype games
// Column 2 (GRN-RED)
#define SW_ROLLOVER_1                 8   // 9
#define SW_ROLLOVER_2                 9   // 10
#define SW_ROLLOVER_3                 10  // 11
#define SW_ROLLOVER_4                 11  // 12
#define SW_ROLLOVER_5                 12  // 13
#define SW_RIGHT_TARGET               13  // 14
#define SW_SPINNER                    14  // 15
#define SW_JET_RIGHT                  15  // 16 right jet bumper
// Column 3 (GRN-ORN)
#define SW_JET_CENTER                 16  // 17
#define SW_JET_LEFT                   17  // 18
#define SW_LEFT_TARGET                18  // 19
#define SW_LEFT_STANDUP               19  // 20
#define SW_BULLSEYE_TARGET            20  // 21
#define SW_LEFT_BANK_STANDUP          21  // 22
#define SW_PLAYFIELD_TILT             22  // 23
#define SW_RIGHT_BANK_STANDUP         23  // 24
// Column 4 (GRN-YEL): left 4-bank
#define SW_DROP_L_1_TOP               24  // 25
#define SW_DROP_L_2_TOP_CENTER        25  // 26
#define SW_DROP_L_3_BOTTOM_CENTER     26  // 27
#define SW_DROP_L_4_BOTTOM            27  // 28
#define SW_DROP_L_SERIES              28  // 29
#define SW_KICKER_LEFT                29  // 30
#define SW_LEFT_INSIDE_ROLLOVER       30  // 31
#define SW_LEFT_OUTSIDE_ROLLOVER      31  // 32
// Column 5 (GRN-BLK)
#define SW_DROP_CENTER_LEFT           32  // 33
#define SW_DROP_CENTER_RIGHT          34  // 35
#define SW_OUTHOLE                    36  // 37
#define SW_TOP_RIGHT_STANDUP          37  // 38
#define SW_BOTTOM_LEFT_STANDUP        38  // 39
#define SW_BOTTOM_RIGHT_STANDUP       39  // 40
// Column 6 (GRN-BLU): right 4-bank
#define SW_DROP_R_1_TOP               40  // 41
#define SW_DROP_R_2_TOP_CENTER        41  // 42
#define SW_DROP_R_3_BOTTOM_CENTER     42  // 43
#define SW_DROP_R_4_BOTTOM            43  // 44
#define SW_DROP_R_SERIES              44  // 45
#define SW_KICKER_RIGHT               45  // 46
#define SW_RIGHT_INSIDE_ROLLOVER      46  // 47
#define SW_RIGHT_OUTSIDE_ROLLOVER     47  // 48
// Columns 7 and 8 (switches 49-64): not used

/*********************************************************************
    SOLENOIDS  (manual Figure 10; test-02 number in comment, RPU index = number - 1)
*********************************************************************/
// Playfield (2P11)
#define SOL_BALL_RELEASE              0   // 1  (outhole kicker / ball serve)
#define SOL_DROP_CENTER_LEFT_RESET    1   // 2
#define SOL_DROP_L_BOTTOM_RESET       2   // 3  left bank, bottom drop targets reset
#define SOL_DROP_L_TOP_RESET          3   // 4  left bank, top drop targets reset
#define SOL_DROP_R_TOP_RESET          4   // 5  right bank, top drop targets reset
#define SOL_DROP_R_BOTTOM_RESET       5   // 6  right bank, bottom drop targets reset
#define SOL_DROP_CENTER_RIGHT_RESET   6   // 7
// 8 not used
// Cabinet (2P9) - sound triggers go to the Phoenix sound board; coil-driven
#define SOL_SOUND_10                  8   // 9   10 pt sound
#define SOL_SOUND_100                 9   // 10  100 pt sound
#define SOL_SOUND_1000                10  // 11  1,000 pt sound
#define SOL_SOUND_10000               11  // 12  10,000 pt sound
#define SOL_SOUND_ALTERNATOR          12  // 13
#define SOL_KNOCKER                   13  // 14  credit knocker
// 15 not used
#define SOL_COIN_LOCKOUT              15  // 16
// Special solenoids (2P12, each paired with a switch on 2P13)
#define SOL_JET_LEFT                  16  // 17
#define SOL_JET_CENTER                17  // 18
#define SOL_JET_RIGHT                 18  // 19
#define SOL_KICKER_LEFT               19  // 20
#define SOL_KICKER_RIGHT              20  // 21
// 22 not used
// Flipper enables (2P12 pins 1-2) are driven through the flipper-enable line, not a numbered coil

/*********************************************************************
    Names used by the generic ExampleMachine game code, mapped to Phoenix.
    Anything Phoenix does not have points at an unused matrix position
    (switches 49-64 and solenoids 8, 15, 22 are unused in the handbook),
    so that code path can never fire.
*********************************************************************/
#define LAMP_APRON_CREDITS            LAMP_APRON_CREDIT
#define SW_DROP_L_1                   SW_DROP_L_1_TOP
#define SW_DROP_L_2                   SW_DROP_L_2_TOP_CENTER
#define SW_DROP_L_3                   SW_DROP_L_3_BOTTOM_CENTER
#define SW_DROP_L_4                   SW_DROP_L_4_BOTTOM
#define SW_DROP_R_1                   SW_DROP_R_1_TOP
#define SW_DROP_R_2                   SW_DROP_R_2_TOP_CENTER
#define SW_DROP_R_3                   SW_DROP_R_3_BOTTOM_CENTER
#define SW_DROP_R_4                   SW_DROP_R_4_BOTTOM
#define SW_POP_BUMPER                 SW_JET_CENTER      // left and right jets handled alongside it
#define SW_LEFT_SLING                 SW_KICKER_LEFT     // Phoenix kickers play the slingshot role
#define SW_RIGHT_SLING                SW_KICKER_RIGHT
#define SW_LEFT_INLANE                SW_LEFT_INSIDE_ROLLOVER
#define SW_LEFT_OUTLANE               SW_LEFT_OUTSIDE_ROLLOVER
#define SW_RIGHT_INLANE               SW_RIGHT_INSIDE_ROLLOVER
#define SW_RIGHT_OUTLANE              SW_RIGHT_OUTSIDE_ROLLOVER
#define SW_LEFT_SPINNER               SW_SPINNER         // Phoenix has a single spinner
#define SW_RIGHT_SPINNER              60                 // unused (switch 61)
#define SW_SAUCER                     63                 // Phoenix has no saucer (switch 64 unused)
#define SOL_SERVE_BALL                SOL_BALL_RELEASE
#define SOL_SAUCER                    7                  // unused (solenoid 8)
#define SOL_UPPER_BALL_EJECT          7                  // unused (solenoid 8)


// These SolenoidAssociatedSwitches are only
// used for Bally/Stern. On Williams, the "immediate solenoids"
// are activated in hardware so they don't need this
// code
#if (RPU_MPU_ARCHITECTURE<10)
#define NUM_SWITCHES_WITH_TRIGGERS          3 // total number of solenoid/switch pairs
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 3 // This number should match the define above

struct PlayfieldAndCabinetSwitch SolenoidAssociatedSwitches[] = {
  { SW_RIGHT_SLING, SOL_RIGHT_SLING, 4},
  { SW_LEFT_SLING, SOL_LEFT_SLING, 4},
  { SW_POP_BUMPER, SOL_POP_BUMPER, 4}
};
#endif




#ifdef RPU_OS_USE_ACCESSORY_LAMP_BOARD
#define MESSAGE_AREA_TOPPER                       1
#define MESSAGE_AREA_SPEAKER                      2
#define MESSAGE_AREA_LEFT_SPEAKER                 3
#define MESSAGE_AREA_RIGHT_SPEAKER                4
#define MESSAGE_AREA_STADIUM                      5
#define MESSAGE_AREA_UNDERCAB                     6
#define MESSAGE_AREA_BACKGLASS                    7
#define MESSAGE_AREA_GI_0                         8
#define MESSAGE_AREA_GI_1                         9
#define MESSAGE_AREA_GI_2                         10
#define MESSAGE_AREA_GI_3                         11
#define MESSAGE_AREA_GI_4                         12
#define SET_COLOR_0                               0
#define SET_COLOR_1                               1
#define SET_COLOR_2                               2
#define SET_TOPPER_BRIGHTNESS                     3
#define SET_SPEAKER_BRIGHTNESS                    4
#define SET_BACKGLASS_BRIGHTNESS                  5
#define SET_STADIUM_BRIGHTNESS                    6
#define SET_UNDERCAB_BRIGHTNESS                   7
#define SET_GI_BRIGHTNESS_0                       8
#define SET_GI_BRIGHTNESS_1                       9
#define SET_GI_BRIGHTNESS_2                       10
#define SET_GI_BRIGHTNESS_3                       11
#define SET_GI_BRIGHTNESS_4                       12
#define TOPPER_PULSE_COLOR_0                      15
#define TOPPER_PULSE_COLOR_1                      16
#define TOPPER_PULSE_COLOR_2                      17
#define TOPPER_FLASH_COLOR_0                      18
#define TOPPER_FLASH_COLOR_1                      19
#define TOPPER_FLASH_COLOR_2                      20
#define TOPPER_LOOP_COLOR_0                       21
#define TOPPER_LOOP_COLOR_1                       22
#define TOPPER_LOOP_COLOR_2                       23
#define TOPPER_LEFT_FLASH_COLOR_0                 24
#define TOPPER_LEFT_FLASH_COLOR_1                 25
#define TOPPER_LEFT_FLASH_COLOR_2                 26
#define TOPPER_RIGHT_FLASH_COLOR_0                27
#define TOPPER_RIGHT_FLASH_COLOR_1                28
#define TOPPER_RIGHT_FLASH_COLOR_2                29
#define TOPPER_LEFT_TO_RIGHT_COLOR_0              30
#define TOPPER_LEFT_TO_RIGHT_COLOR_1              31
#define TOPPER_LEFT_TO_RIGHT_COLOR_2              32
#define TOPPER_RIGHT_TO_LEFT_COLOR_0              33
#define TOPPER_RIGHT_TO_LEFT_COLOR_1              34
#define TOPPER_RIGHT_TO_LEFT_COLOR_2              35
#define TOPPER_SPARKLE_COLOR_0                    36
#define TOPPER_SPARKLE_COLOR_1                    37
#define TOPPER_SPARKLE_COLOR_2                    38
#define TOPPER_FIRE                               39
#define TOPPER_CANDY_PULSE                        40
#define TOPPER_LIGHTNING_0                        41
#define TOPPER_LIGHTNING_1                        42
#define TOPPER_LIGHTNING_2                        43
#define TOPPER_LIGHTNING_3                        44
#define TOPPER_LIGHTNING_4                        45
#define TOPPER_BIG_LIGHTNING_0                    46
#define TOPPER_BIG_LIGHTNING_1                    47
#define LEFT_TO_RIGHT_SPEAKER_COLOR_0             50
#define LEFT_TO_RIGHT_SPEAKER_COLOR_1             51
#define LEFT_TO_RIGHT_SPEAKER_COLOR_2             52
#define RIGHT_TO_LEFT_SPEAKER_COLOR_0             53
#define RIGHT_TO_LEFT_SPEAKER_COLOR_1             54
#define RIGHT_TO_LEFT_SPEAKER_COLOR_2             55
#define BOTH_SPEAKERS_TWO_COLOR_0_1_PULSE         56
#define LEFT_SPEAKER_PULSE_COLOR_0                60
#define LEFT_SPEAKER_PULSE_COLOR_1                61
#define LEFT_SPEAKER_PULSE_COLOR_2                62
#define LEFT_SPEAKER_FLASH_COLOR_0                63
#define LEFT_SPEAKER_FLASH_COLOR_1                64
#define LEFT_SPEAKER_FLASH_COLOR_2                65
#define LEFT_SPEAKER_LOOP_CW_COLOR_0              66
#define LEFT_SPEAKER_LOOP_CW_COLOR_1              67
#define LEFT_SPEAKER_LOOP_CW_COLOR_2              68
#define LEFT_SPEAKER_LOOP_CCW_COLOR_0             69
#define LEFT_SPEAKER_LOOP_CCW_COLOR_1             70
#define LEFT_SPEAKER_LOOP_CCW_COLOR_2             71
#define RIGHT_SPEAKER_PULSE_COLOR_0               75
#define RIGHT_SPEAKER_PULSE_COLOR_1               76
#define RIGHT_SPEAKER_PULSE_COLOR_2               77
#define RIGHT_SPEAKER_FLASH_COLOR_0               78
#define RIGHT_SPEAKER_FLASH_COLOR_1               79
#define RIGHT_SPEAKER_FLASH_COLOR_2               80
#define RIGHT_SPEAKER_LOOP_CW_COLOR_0             81
#define RIGHT_SPEAKER_LOOP_CW_COLOR_1             82
#define RIGHT_SPEAKER_LOOP_CW_COLOR_2             83
#define RIGHT_SPEAKER_LOOP_CCW_COLOR_0            84
#define RIGHT_SPEAKER_LOOP_CCW_COLOR_1            85
#define RIGHT_SPEAKER_LOOP_CCW_COLOR_2            86
#define STADIUM_SET_COLOR_0                       90
#define STADIUM_SET_COLOR_1                       91
#define STADIUM_SET_COLOR_2                       92
#define STADIUM_PULSE_COLOR_0                     93
#define STADIUM_PULSE_COLOR_1                     94
#define STADIUM_PULSE_COLOR_2                     95
#define UNDERCAB_SET_COLOR_0                      120
#define BACKGLASS_SET_SCENE_0                     170
#define GI_0_SET_COLOR                            210
#define GI_0_PULSE_COLOR                          211
#define GI_0_FLICKER_COLOR                        212
#define GI_1_SET_COLOR                            218
#define GI_1_PULSE_COLOR                          219
#define GI_1_FLICKER_COLOR                        220
#define GI_2_SET_COLOR                            226
#define GI_2_PULSE_COLOR                          227
#define GI_2_FLICKER_COLOR                        228
#define GI_2_FLICKER_START_PIXEL                  229
#define GI_2_FLICKER_END_PIXEL                    230
#define GI_3_SET_COLOR                            232
#define GI_4_SET_COLOR                            240
#define CONTROL_MESSAGE_UNSET                     255
#endif
